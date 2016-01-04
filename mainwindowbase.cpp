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

#include "mainwindow.h"
#include "backtracemodel.h"
#include "backtraceproxymodel.h"
#include "backtraceview.h"

#include <QApplication>
#include <QFileDialog>
#include <QString>
#include <QListWidgetItem>
#include <QItemSelectionModel>
#include <QMenu>
#include <QCursor>
#include <QClipboard>
#include <QProcess>
#include <QDebug>
#include <QRegularExpression>

#include <string>
#include <iostream>

using namespace std;

MainWindowBase::MainWindowBase(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow())
    , m_model(new BacktraceModel(this))
    , m_proxyModel(new BacktraceProxyModel(m_model->backtraceDecorationCache(), this))
{
    ui->setupUi(this);
    connect(ui->actionQuit, &QAction::triggered, qApp, &QApplication::quit);
    connect(ui->actionOpen_Log, &QAction::triggered, this, &MainWindowBase::chooseAndOpenReport);
    connect(ui->filterLineEdit, &QLineEdit::textChanged, this, &MainWindowBase::filterByText);

    m_proxyModel->setSourceModel(m_model);
    ui->tableView->setModel(m_proxyModel);

    setWindowTitle("container-stats-viewer");
    ui->tableView->setSortingEnabled(true);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(ui->tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindowBase::showBacktrace);

    BacktraceViewDock *btDock = new BacktraceViewDock(m_model->backtraceDecorationCache(), this);
    addDockWidget(Qt::BottomDockWidgetArea, btDock);
    m_backtraceView = btDock->view();

    ui->tableView->resizeColumnsToContents();
}

MainWindowBase::~MainWindowBase()
{
    delete ui;
}

void MainWindowBase::resizeEvent(QResizeEvent *ev)
{
    QMainWindow::resizeEvent(ev);
    ui->tableView->resizeColumnsToContents();
}

void MainWindowBase::chooseAndOpenReport()
{
    const QString filename = QFileDialog::getOpenFileName(this, tr("Open log"));
    openReport(filename);
}

bool MainWindowBase::openReport(const QString &filename)
{
    bool success = m_model->loadFile(filename);
    ui->tableView->resizeColumnsToContents();
    return success;
}

void MainWindowBase::filterByText()
{
    m_proxyModel->setText(ui->filterLineEdit->text());
}

void MainWindowBase::showBacktrace()
{
    const auto indexes = ui->tableView->selectionModel()->selectedIndexes();
    if (indexes.isEmpty())
        return;

    auto bt = indexes.first().data(BacktraceModel::BacktraceRole).value<Backtrace*>();
    m_backtraceView->setBacktrace(bt);
}

QModelIndex MainWindowBase::selectedIndex() const
{
    const auto indexes = ui->tableView->selectionModel()->selectedIndexes();
    return indexes.isEmpty() ? QModelIndex() : indexes.first();
}
