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

#ifndef BACKTRACE_BOOST_SERIALIZERS_H
#define BACKTRACE_BOOST_SERIALIZERS_H

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>

namespace boost {
namespace serialization {

template<class Archive>
void serialize(Archive &ar, Backtrace::Frame &frame, const unsigned int /*version*/)
{
    // We don't serialize the elfFilename, occupies too much space, use elfFilenameId instead
    ar & frame.elfFilenameId;
    ar & frame.offset;
}

template<class Archive>
void serialize(Archive &ar, Backtrace &bt, const unsigned int /*version*/)
{
    ar &bt.m_metadata;
    ar & bt.m_frames;
}

}
}

#endif
