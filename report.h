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

#ifndef BACKTRACE_REPORT_H
#define BACKTRACE_REPORT_H

#include "backtrace.h"

#include <string>
#include <vector>

class Report
{
public:
    Report();
    Report(int type);
    Report(const Report &);
    ~Report();

    void addBacktrace(Backtrace *);
    void loadFromFile(const std::string &filename);
    void saveToFile(const std::string &filename);

    const Backtrace::List & backtraces() const;
    size_t size() const;
    void clear();
    void setColumnNames(const std::vector<std::string> &);
    std::vector<std::string> columnNames() const;
    int type() const;

private:
    class Private;
    Private *const d;
};

#endif
