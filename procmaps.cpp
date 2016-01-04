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

#include "procmaps.h"
#include <fstream>
#include <unistd.h>
#include <regex>
#include <iostream>

using namespace std;

ProcMaps::ProcMaps() : m_pid(getpid())
{
    load();
}

ElfObject ProcMaps::objectForAddress(uintptr_t addr) const
{
    for (const ElfObject &obj : m_elfObjects) {
        if (obj.startAddress <= addr && obj.endAddress >= addr) {
            return obj;
        }
    }

    return {};
}

string ProcMaps::procFileName() const
{
    return string("/proc/") + std::to_string(m_pid) + "/maps";
}

void ProcMaps::load()
{
    m_elfObjects.empty();
    ifstream in(procFileName());
    if (!in.is_open())
        return;

    regex rx(R"((.*?)-(.*?) .* .* .* (.*))");
    smatch match;
    string line;
    while (getline(in, line)) {
        if (!regex_match(line, match, rx) || match.size() != 4)
            continue;

        ElfObject obj;
        try {
            obj.startAddress = std::stoll(match[1], 0, 16);
            obj.endAddress = std::stoll(match[2], 0, 16);
            obj.filename = match[3];
        } catch (std::out_of_range e) {
            // cerr << "Ignoring " << match[3] << ": " << match[1] << "; " << match[2] << "\n";
        }

        if (!obj.filename.empty())
            m_elfObjects.push_back(obj);
    }
}
