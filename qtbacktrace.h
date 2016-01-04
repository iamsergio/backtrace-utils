#ifndef QT_BACKTRACE_H
#define QT_BACKTRACE_H

#include "backtrace.h"

#include <QHash>
#include <QMetaType>

Q_DECLARE_METATYPE(Backtrace*)

inline bool operator==(const Backtrace::Frame &lhs, const Backtrace::Frame &rhs)
{
    return lhs.offset == rhs.offset && lhs.elfFilename == rhs.elfFilename;
}

inline bool operator==(const Backtrace &lhs, const Backtrace &rhs)
{
    return lhs.m_frames == rhs.m_frames;
}

inline uint qHash(const Backtrace::Frame &frame, uint seed = 0) Q_DECL_NOTHROW
{
    QtPrivate::QHashCombine hash;
    seed = hash(seed, frame.offset);
    seed = hash(seed, QString::fromStdString(frame.elfFilename));

    return seed;
}

inline uint qHash(const Backtrace &bt, uint seed = 0) Q_DECL_NOTHROW
{
    QtPrivate::QHashCombine hash;

    for (const auto &frame : bt.m_frames) {
        seed = hash(seed, qHash(frame, seed));
    }

    return seed;
}

#endif
