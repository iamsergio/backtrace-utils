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


#ifndef BACKTRACE_VIEW_H
#define BACKTRACE_VIEW_H

#include "qtbacktrace.h"

#include <QListView>
#include <QDockWidget>

class BacktraceDecorationCache;

class BacktraceView : public QListView
{
    Q_OBJECT
public:
    explicit BacktraceView(BacktraceDecorationCache *, QWidget *parent = nullptr);
    ~BacktraceView();
    void setBacktrace(Backtrace *);

    void setEditorCommand(const QString &command);

private Q_SLOTS:
    void openSourceFile(const QModelIndex &);
private:
    class Private;
    Private *const d;
};

class BacktraceViewDock : public QDockWidget
{
    Q_OBJECT
public:
    explicit BacktraceViewDock(BacktraceDecorationCache *, QWidget *parent = nullptr);
    BacktraceView* view() const;
private:
    BacktraceView *const m_view;
};

#endif
