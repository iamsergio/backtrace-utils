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

#ifndef BACKTRACE_VIEWER_BACKTRACE_DECORATION_CACHE_H
#define BACKTRACE_VIEWER_BACKTRACE_DECORATION_CACHE_H

#include "qtbacktrace.h"

#include <QRegularExpression>
#include <QMap>

struct InterestingBacktraceFilters
{
    struct Filter {
        int numFramesToCheck;
        QRegularExpression inclusiveFilter;
        QRegularExpression exclusiveFilter;
    };

    QMap<int, Filter> m_filterByType;
};

/**
 * Decorates raw backtraces with line, column and function info in a human readable form.
 * Caches the decoration to speed up future decorations.
 */
class BacktraceDecorationCache
{
public:
    BacktraceDecorationCache();
    ~BacktraceDecorationCache();

    /**
     * Generates line, function and filename info for all backtraces previously added with addBacktrace()
     */
    void generateDecorations();

    /**
     * Add a backtrace for which decorations will be generated for.
     * Add all your backtraces with this function before calling generateDecorations()
     */
    void addBacktrace(Backtrace *);

    /**
     * List of backtraces excluding dups.
     */
    const Backtrace::List &uniqueBacktraces() const;

    /**
     * List of backtraces excluding dups and excluding uninteresting ones.
     */
    Backtrace::List interestingBacktraces() const;

    void setInterestingFilter(const InterestingBacktraceFilters &filters);
    void setInterestingFilterType(int type);

    Backtrace::Frame::Decoration decorationForFrame(const Backtrace::Frame &frame) const;

    /**
     * For statistical purposes, the dup count.
     */
    int countForBacktrace(Backtrace *bt) const;

private:
    Q_DISABLE_COPY(BacktraceDecorationCache)
    class Private;
    Private *const d;
};

#endif
