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

#ifndef BACKTRACE_VIEWER_PROC_MAPS_H
#define BACKTRACE_VIEWER_PROC_MAPS_H

#include <vector>
#include <unistd.h>
#include <string>
#include <stdint.h>

// ProcMaps represents the /proc/<pid>/maps file

struct ElfObject
{
    typedef std::vector<ElfObject> List;
    uintptr_t startAddress; // start of .text
    uintptr_t endAddress;   // end of .text
    std::string filename;
    bool isValid() const { return !filename.empty(); }

    // addr2line receives offsets from .text, not virtual addresses
    uintptr_t offsetForAddress(uintptr_t addr) const { return addr - startAddress; }
};

class ProcMaps
{
public:
    explicit ProcMaps();

    ElfObject objectForAddress(uintptr_t) const;

private:
    void load();
    std::string procFileName() const;
    const pid_t m_pid;
    ElfObject::List m_elfObjects;
};

#endif
