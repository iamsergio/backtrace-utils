/*
  This file is part of backtrace-viewer.

  Copyright (c) 2015 Sergio Martins <smartins@kde.org>

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

#include "report.h"
#include "boost_serializers.h"

#include <vector>
#include <fstream>
#include <iostream>

using namespace std;

class Report::Private
{
public:

    Private(int type) : type(type)
    {
    }

    void createElfFilenameIds();
    void restoreElfFilenames();
    Backtrace::List backtraces;
    vector<string> elfFilenames;
    vector<string> columnNames;
    int type;
};

void Report::Private::createElfFilenameIds()
{
    elfFilenames.clear();
    for (Backtrace *bt : backtraces) {
        for (auto &frame : bt->m_frames) {
            // Cache the names, so we can serialize indexes instead of filenames to save space
            auto it = find(elfFilenames.cbegin(), elfFilenames.cend(), frame.elfFilename);
            if (it == elfFilenames.cend()) {
                elfFilenames.push_back(frame.elfFilename);
                frame.elfFilenameId = elfFilenames.size() - 1;
            } else {
                frame.elfFilenameId = it - elfFilenames.cbegin();
            }
        }
    }
}

void Report::Private::restoreElfFilenames()
{
    const int numKnownElfs = elfFilenames.size();
    for (Backtrace *bt : backtraces) {
        for (auto &frame : bt->m_frames) {

            if (frame.elfFilenameId >= 0 && frame.elfFilenameId < numKnownElfs) {
                frame.elfFilename = elfFilenames[frame.elfFilenameId];
            } else {
                std::cerr << "Wrong elfFileNameID" << frame.elfFilenameId << "\n";
            }
        }
    }
}

Report::Report()
: d(new Private(-1))
{

}

Report::Report(int type)
    : d(new Private(type))
{
}

Report::Report(const Report &other)
    : d(new Private(*other.d))
{
}

Report::~Report()
{
    delete d;
}

void Report::addBacktrace(Backtrace *bt)
{
    d->backtraces.push_back(bt);
}

void Report::loadFromFile(const string &filename)
{
    std::ifstream ifs(filename);
    clear();
    try {
        boost::archive::text_iarchive ia(ifs);
        ia >> d->elfFilenames;
        ia >> d->backtraces; // TODO leaks
        ia >> d->columnNames;
        ia >> d->type;
        d->restoreElfFilenames();
    } catch (boost::archive::archive_exception ae) {
        std::cerr << "archive exception: " << ae.what() << "\n";
    }
}

void Report::saveToFile(const string &filename)
{
    if (!size())
        return;

    d->createElfFilenameIds();
    std::ofstream ofs(filename);
    boost::archive::text_oarchive oa(ofs);
    oa << d->elfFilenames;
    oa << d->backtraces;
    oa << d->columnNames;
    oa << d->type;
}

const Backtrace::List &Report::backtraces() const
{
    return d->backtraces;
}

size_t Report::size() const
{
    return d->backtraces.size();
}

void Report::clear()
{
    d->backtraces.clear(); // TODO: leaks
}

void Report::setColumnNames(const vector<string> &names)
{
    d->columnNames = names;
}

std::vector<string> Report::columnNames() const
{
    return d->columnNames;
}

int Report::type() const
{
    return d->type;
}
