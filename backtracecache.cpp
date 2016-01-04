/*
  This file is part of backtrace-utils.

  Copyright (c) 2015-2016 Sergio Martins <smartins@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "backtracecache.h"

#include <QProcess>
#include <QSet>
#include <QDebug>
#include <QRegularExpression>

class BacktraceDecorationCache::Private
{
public:
    void decorate(Backtrace::Frame &);
    bool generateDecorations(const QString &elfObjectName, const QList<uintptr_t> &addresses);
    bool generateDecorations(const QString &elfObjectName, const Backtrace::Frame::List &);
    bool isInterestingBacktrace(Backtrace *bt) const;
    bool isFalsePositiveFrame(const Backtrace::Frame &frame) const;
    bool isInterestingFrame(const Backtrace::Frame &frame) const;

    QHash<Backtrace::Frame, Backtrace::Frame::Decoration> m_decorationForFrame;
    QHash<QString, Backtrace::Frame::List> m_framesByElfName; // all our known frames, indexed by elf name (library or executable where they appear)
    Backtrace::List m_backtraces;
    QHash<Backtrace, int> m_uniqueBacktraceCount;
    bool m_decorationsGenerated = false;
    InterestingBacktraceFilters m_interestingBacktraceFilters;
    int m_interestingFilterType;
};

bool BacktraceDecorationCache::Private::generateDecorations(const QString &elfObjectName, const Backtrace::Frame::List &frames)
{
    QList<uintptr_t> addresses;
    addresses.reserve(frames.size());
    for (const auto &frame : frames) {
        addresses.push_back(frame.offset);
    }

    if (addresses.isEmpty()) // Everything is already decorated for this elf object
        return true;

    return generateDecorations(elfObjectName, addresses);
}

bool BacktraceDecorationCache::Private::generateDecorations(const QString &elfObjectName, const QList<uintptr_t> &addresses)
{
    QStringList hexAddresses;
    hexAddresses.reserve(addresses.size());
    for (auto address : addresses) {
        hexAddresses.push_back(QString::number(address, 16));
    }

    const QString command = QStringLiteral("addr2line");
    QStringList arguments = { "-C", "-e", elfObjectName, "-f" };
    arguments.append(hexAddresses);

    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start(command, arguments);
    if (!process.waitForStarted()) {
        qWarning() << "Could not run addr2line";
        return false;
    }

    QByteArray data;
    while (process.waitForReadyRead())
        data.append(process.readAll());

    QList<QByteArray> lines = data.split('\n');
    if (lines.last().isEmpty())
        lines.removeLast();

    const int numLines = lines.size();

    if (numLines != addresses.size() * 2) {
        qWarning() << "Invalid backtrace. numLines=" << numLines << "; expected=" << addresses.size() * 2
                   << "lines=" << data << "; elfObjectName=" << elfObjectName;
        return false;
    }

    for (int i = 0; i < numLines - 1; i += 2) {
        Backtrace::Frame::Decoration decoration;

        decoration.functionName = lines[i].toStdString();
        decoration.sourceFilename = lines[i+1].toStdString();
        Backtrace::Frame frame = { -1, elfObjectName.toStdString(), addresses[i / 2] };
        m_decorationForFrame.insert(frame, decoration);
    }

    return true;
}

bool BacktraceDecorationCache::Private::isInterestingFrame(const Backtrace::Frame &frame) const
{
    auto decoration = m_decorationForFrame.value(frame);
    static const QStringList interesting = { "::first", "::last", "::begin", "::end", "::data", "::operator[]", "::front", "::back" };

    auto pred = [decoration](const QString &s){ return QString::fromStdString(decoration.functionName).contains(s); };
    if (std::any_of(interesting.cbegin(), interesting.cend(), pred)) {
        return true;
    }

    return false;
}

bool BacktraceDecorationCache::Private::isInterestingBacktrace(Backtrace *bt) const
{
    if (m_interestingFilterType == -1)
        return true;

    auto filter = m_interestingBacktraceFilters.m_filterByType.value(m_interestingFilterType);
    if (filter.inclusiveFilter.pattern().isEmpty() && filter.exclusiveFilter.pattern().isEmpty())
        return true;

    QStringList interestingFrames;
    const int numFrames = qMin(filter.numFramesToCheck, int(bt->m_frames.size()));
    interestingFrames.reserve(numFrames);
    for (int i = 0; i < numFrames; ++i) {
        auto decoration = m_decorationForFrame.value(bt->m_frames[i]);
        interestingFrames.push_back(QString::fromStdString(decoration.functionName)); // FIXME: remove fromStdString
    }

    const QString backtraceStr = interestingFrames.join(';');
    if (!filter.inclusiveFilter.pattern().isEmpty()) {
        auto match = filter.inclusiveFilter.match(backtraceStr);
        if (!match.hasMatch())
            return false;
    }

    if (!filter.exclusiveFilter.pattern().isEmpty()) {
        auto match = filter.exclusiveFilter.match(backtraceStr);
        if (match.hasMatch())
            return false;
    }

    return true;
}

BacktraceDecorationCache::BacktraceDecorationCache()
    : d(new Private())
{
}

BacktraceDecorationCache::~BacktraceDecorationCache()
{
    delete d;
}

void BacktraceDecorationCache::addBacktrace(Backtrace *bt)
{
    auto it = d->m_uniqueBacktraceCount.find(*bt);
    if (it != d->m_uniqueBacktraceCount.end()) {
        (*it)++; // was found, increment
        return;
    }

    d->m_backtraces.push_back(bt);
    d->m_uniqueBacktraceCount.insert(*bt, 1);
    for (const Backtrace::Frame &frame : bt->m_frames) {
        if (frame.elfFilename.empty()) {
            qWarning() << "Empty elf name for frame with offset" << frame.offset;
            continue;
        }

        auto &list = d->m_framesByElfName[QString::fromStdString(frame.elfFilename)];
        if (find(list.cbegin(), list.cend(), frame) == list.cend()) // O(n)
            list.push_back(frame);
    }
}

const Backtrace::List & BacktraceDecorationCache::uniqueBacktraces() const
{
    return d->m_backtraces;
}

Backtrace::List BacktraceDecorationCache::interestingBacktraces() const
{
    Backtrace::List interestingBTs;
    for (auto bt : d->m_backtraces) {
        if (d->isInterestingBacktrace(bt))
            interestingBTs.push_back(bt);
    }

    return interestingBTs;
}

void BacktraceDecorationCache::setInterestingFilter(const InterestingBacktraceFilters &filters)
{
    d->m_interestingBacktraceFilters = filters;
}

void BacktraceDecorationCache::setInterestingFilterType(int type)
{
    d->m_interestingFilterType = type;
}

Backtrace::Frame::Decoration BacktraceDecorationCache::decorationForFrame(const Backtrace::Frame &frame) const
{
    return d->m_decorationForFrame.value(frame);
}

int BacktraceDecorationCache::countForBacktrace(Backtrace *bt) const
{
    return d->m_uniqueBacktraceCount.value(*bt);
}

void BacktraceDecorationCache::generateDecorations()
{
    for (auto it = d->m_framesByElfName.cbegin(), end = d->m_framesByElfName.cend(); it != end; ++it) {
        d->generateDecorations(it.key(), it.value());
    }
}
