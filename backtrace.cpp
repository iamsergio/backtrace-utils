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

#include "backtrace.h"
#include "procmaps.h"

#define UNW_LOCAL_ONLY
#include <libunwind.h>

enum {
    MaxFrames = 64
};

static void init_unw()
{
    if (unw_set_caching_policy(unw_local_addr_space, UNW_CACHE_PER_THREAD)) {
        fprintf(stderr, "WARNING: Failed to enable per-thread libunwind caching.\n");
    }

#if HAVE_UNW_SET_CACHE_SIZE
    if (unw_set_cache_size(unw_local_addr_space, 1024)) {
        fprintf(stderr, "WARNING: Failed to set libunwind cache size.\n");
    }
#endif
}

void Backtrace::generate(int skip)
{
    static ProcMaps *s_procMaps = nullptr;

    if (!s_procMaps) {
        init_unw(); // init on first call
        s_procMaps = new ProcMaps();
    }

    void * rawIPArray[MaxFrames];

    m_frames.clear(); // leaks ?

    auto numFrames = unw_backtrace(rawIPArray, MaxFrames);
    for (int i = skip; i < numFrames; ++i) {
        auto address = uintptr_t(rawIPArray[i]);
        ElfObject obj = s_procMaps->objectForAddress(address);
        if (!obj.isValid())
            continue;
        Frame frame = { -1, obj.filename, obj.offsetForAddress(address) };
        m_frames.push_back(frame);
    }
}
