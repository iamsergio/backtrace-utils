/*
   This file is part of backtrace-utils.

  Copyright (C) 2015-2016 Sergio Martins <smartins@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "backtraceproxymodel.h"
#include "backtracecache.h"

using namespace std;

BacktraceProxyModel::BacktraceProxyModel(BacktraceDecorationCache *decorationCache, QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_decorationCache(decorationCache)
{
    connect(this, &BacktraceProxyModel::rowsInserted, this, &BacktraceProxyModel::countChanged);
    connect(this, &BacktraceProxyModel::rowsRemoved, this, &BacktraceProxyModel::countChanged);
    connect(this, &BacktraceProxyModel::modelReset, this, &BacktraceProxyModel::countChanged);
    connect(this, &BacktraceProxyModel::layoutChanged, this, &BacktraceProxyModel::countChanged);
}

bool BacktraceProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (source_parent.isValid())
        return false;

    if (m_text.empty())
        return true;

    const QModelIndex sourceIndex = sourceModel()->index(source_row, 0);
    auto bt = sourceIndex.data(BacktraceModel::BacktraceRole).value<Backtrace*>();
    Q_ASSERT(bt);

    for (const auto &frame : bt->m_frames) {
        if (frame.elfFilename.find(m_text) != string::npos)
            return true;

        auto decoration = m_decorationCache->decorationForFrame(frame);
        if (decoration.isValid()) {
            if (decoration.functionName.find(m_text) != string::npos)
                return true;
            if (decoration.sourceFilename.find(m_text) != string::npos)
                return true;
        }
    }

    return false;
}

void BacktraceProxyModel::setText(const QString &filter)
{
    string s = filter.toLower().toStdString();
    if (s != m_text) {
        m_text = s;
        invalidateFilter();
    }
}
