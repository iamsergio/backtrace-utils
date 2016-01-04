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

#include "backtracemodel.h"

#include <QDebug>

BacktraceModel::BacktraceModel(QObject *parent)
    : QAbstractTableModel(parent)
    , m_backtraceCache(new BacktraceDecorationCache())
{
    connect(this, &BacktraceModel::rowsInserted, this, &BacktraceModel::countChanged);
    connect(this, &BacktraceModel::rowsRemoved, this, &BacktraceModel::countChanged);
    connect(this, &BacktraceModel::modelReset, this, &BacktraceModel::countChanged);
    connect(this, &BacktraceModel::layoutChanged, this, &BacktraceModel::countChanged);
}

BacktraceModel::~BacktraceModel()
{
    delete m_backtraceCache;
}

int BacktraceModel::columnCount(const QModelIndex &) const
{
    return qMax(1, int(m_report.columnNames().size()));
}

int BacktraceModel::rowCount(const QModelIndex &/*parent*/) const
{
    return m_backtraces.size();
}

QVariant BacktraceModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= int(m_backtraces.size()))
        return QVariant();

    Backtrace *bt = m_backtraces.at(index.row());
    if (index.column() >= int(bt->m_metadata.size()))
        return QVariant();

    if (role == Qt::DisplayRole) {
        auto str = QString::fromStdString(bt->m_metadata.at(index.column()));
        if (str == "$dup_count$") {
            return m_backtraceCache->countForBacktrace(bt);
        }

        bool ok = false;
        int number = str.toInt(&ok);
        return ok ? QVariant(number) : QVariant(str);
    } else if (role == BacktraceRole) {
        return QVariant::fromValue<Backtrace*>(bt);
    }

    return QVariant();
}

QVariant BacktraceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || section >= int(m_report.columnNames().size()))
        return {};

    if (role == Qt::DisplayRole) {
        return QString::fromStdString(m_report.columnNames().at(section));
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

int BacktraceModel::count() const
{
    return rowCount({});
}

bool BacktraceModel::loadFile(const QString &filename)
{
    beginResetModel();

    qDebug() << "Loading " << filename;

    m_report.loadFromFile(filename.toStdString());
    qDebug() << "Loaded report with"  << m_report.size() << "entries";
    m_backtraceCache->setInterestingFilterType(m_report.type());

    m_backtraces.clear();
    for (auto bt : m_report.backtraces()) {
        m_backtraceCache->addBacktrace(bt);
    }

    qDebug() << "Generating decorations";
    m_backtraceCache->generateDecorations();
    m_backtraces = m_backtraceCache->interestingBacktraces();

    endResetModel();

    qDebug() << "Loaded" << m_backtraces.size() << "rows.";

    return true;
}

BacktraceDecorationCache *BacktraceModel::backtraceDecorationCache() const
{
    return m_backtraceCache;
}

const Report &BacktraceModel::report() const
{
    return m_report;
}
