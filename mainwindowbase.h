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

#ifndef VIEWER_MAIN_WINDOW_BASE_H
#define VIEWER_MAIN_WINDOW_BASE_H

#include "ui_mainwindow.h"
#include "backtracecache.h"

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class BacktraceModel;
class BacktraceProxyModel;
class BacktraceView;

class MainWindowBase : public QMainWindow
{
public:
    explicit MainWindowBase(QWidget *parent = nullptr);
    ~MainWindowBase();
    bool openReport(const QString &filename);

protected:
    void resizeEvent(QResizeEvent *) override;

protected:
    Ui::MainWindow *const ui;
    BacktraceModel *const m_model;
    BacktraceProxyModel *const m_proxyModel;
    BacktraceView * m_backtraceView;

private Q_SLOTS:
    void chooseAndOpenReport();
    void filterByText();
    void showBacktrace();

private:
    QModelIndex selectedIndex() const;
};

#endif
