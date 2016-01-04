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


#include "backtraceview.h"
#include "backtracecache.h"

#include <QStandardItemModel>
#include <QDebug>
#include <QProcess>

class StandardItemModel : public QStandardItemModel
{
public:
    explicit StandardItemModel(QObject *parent) : QStandardItemModel(parent) {}

    Qt::ItemFlags flags(const QModelIndex &) const override
    {
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }
};

class BacktraceView::Private
{
public:

    Private(BacktraceDecorationCache *decorationCache, BacktraceView *q)
        : q(q)
        , m_model(new StandardItemModel(q))
        , m_backtrace(nullptr)
        , m_decorationCache(decorationCache)
    {
    }

    BacktraceView *const q;
    QStandardItemModel *const m_model;
    Backtrace *m_backtrace;
    BacktraceDecorationCache *const m_decorationCache;
    QString m_editorCommand;
};

BacktraceView::BacktraceView(BacktraceDecorationCache *decorationCache, QWidget *parent)
    : QListView(parent)
    , d(new Private(decorationCache, this))
{
    setModel(d->m_model);
    connect(this, &QAbstractItemView::doubleClicked, this, &BacktraceView::openSourceFile);
}

BacktraceView::~BacktraceView()
{
    delete d;
}

void BacktraceView::setBacktrace(Backtrace *bt)
{
    if (bt == d->m_backtrace)
        return;

    d->m_backtrace = bt;

    d->m_model->clear();
    QStandardItem *rootItem = d->m_model->invisibleRootItem();

    for (auto it = bt->m_frames.cbegin(), end = bt->m_frames.cend(); it != end; ++it) {
        auto decoration = d->m_decorationCache->decorationForFrame(*it);
        std::string text = decoration.functionName + " at " + decoration.sourceFilename;
        auto item = new QStandardItem(QString::fromStdString(text));
        rootItem->appendRow(item);
    }
}

void BacktraceView::setEditorCommand(const QString &command)
{
    d->m_editorCommand = command;
}

void BacktraceView::openSourceFile(const QModelIndex &index)
{
    if (d->m_editorCommand.isEmpty())
        return;

    const QString text = index.data().toString();
    QStringList splitted = text.split(" at ");

    if (splitted.size() != 2) {
        qWarning() << "Could not split" << text;
        return;
    }

    const QString filename = splitted[1];
    if (filename.startsWith("??"))
        return;

    QProcess::startDetached(d->m_editorCommand, { filename });
}

BacktraceViewDock::BacktraceViewDock(BacktraceDecorationCache *decorationCache, QWidget *parent)
    : QDockWidget(parent)
    , m_view(new BacktraceView(decorationCache, parent))
{
    setWindowTitle(tr("Backtrace"));
    setWidget(m_view);
}

BacktraceView *BacktraceViewDock::view() const
{
    return m_view;
}
