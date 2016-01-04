/*
  This file is part of backtrace-utils.

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

#ifndef BACKTRACE_VIEWER_BACKTRACE_H
#define BACKTRACE_VIEWER_BACKTRACE_H

#include <vector>
#include <string>
#include <cstdint>

struct Backtrace
{
    typedef std::vector<Backtrace*> List;

    struct Frame {
        struct Decoration {
            std::string functionName;
            std::string sourceFilename;
            bool isValid() const { return !functionName.empty() || !sourceFilename.empty(); }
        };

        typedef std::vector<Frame> List;
        int elfFilenameId; // Some unique ID to represent the elfFilename, so we save space when storing lots of backtraces in a text file.
        std::string elfFilename;
        uintptr_t offset; // offset from .text
    };

    inline std::string toString() const;
    void generate(int skip = 1);
    Frame::List m_frames;
    std::vector<std::string> m_metadata;
};

std::string Backtrace::toString() const
{
    std::string s;
    for (auto it = m_frames.cbegin(), end = m_frames.cend(); it != end; ++it) {
        s += (*it).elfFilename + ";" + std::to_string((*it).offset) + ";";
    }

    return s;
}

#endif
