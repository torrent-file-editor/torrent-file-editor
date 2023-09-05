/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 *
 * Copyright (C) 2014-2015  Ivan Romanov <drizt72@zoho.eu>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "tableview.h"

#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QItemSelectionModel>
#include <QMenu>
#include <QLineEdit>
#include <QDir>

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
# include <QStyleHints>
# define ACCEL_KEY(k) ((!QCoreApplication::testAttribute(Qt::AA_DontShowIconsInMenus)                      \
                         && QGuiApplication::styleHints()->showShortcutsInContextMenus())                  \
                       && !QKeySequence(k).toString(QKeySequence::NativeText).isEmpty() ?                  \
                       QLatin1Char('\t') + QKeySequence(k).toString(QKeySequence::NativeText) : QString())
#else
# define ACCEL_KEY(k) (!QCoreApplication::testAttribute(Qt::AA_DontShowIconsInMenus)                       \
                       && !QKeySequence(k).toString(QKeySequence::NativeText).isEmpty() ?                  \
                       QLatin1Char('\t') + QKeySequence(k).toString(QKeySequence::NativeText) : QString())
#endif


TableView::TableView(QWidget *parent)
    : QTableView(parent)
    , _menu(new QMenu(this))
{
    setContextMenuPolicy(Qt::DefaultContextMenu);

    _copyAct = new QAction(this);
    _copyAct->setIcon(QIcon::fromTheme(QLatin1String("edit-copy")));
    _copyAct->setShortcut(QKeySequence::Copy);
    connect(_copyAct, SIGNAL(triggered()), SLOT(copy()));
    _menu->addAction(_copyAct);
    addAction(_copyAct);

    _copySizeAct = new QAction(tr("Copy with Size"), this);
    _copySizeAct->setShortcut(QKeySequence(Qt::ShiftModifier | Qt::ControlModifier | Qt::Key_C));
    connect(_copySizeAct, SIGNAL(triggered()), SLOT(copyWithSize()));
    _menu->addAction(_copySizeAct);
    addAction(_copySizeAct);

    _copyWoExtAct = new QAction(tr("Copy Filename"), this);
    connect(_copyWoExtAct, SIGNAL(triggered()), SLOT(copyWoExt()));
    _menu->addAction(_copyWoExtAct);
    addAction(_copyWoExtAct);

    updateTranslations();
}

void TableView::copy()
{
    QItemSelectionModel *selection = selectionModel();
    QModelIndexList rows = selection->selectedRows();
    QStringList files;
    for (const auto &row: rows) {
        files << row.data().toString();
    }

    if (!files.isEmpty()) {
        QApplication::clipboard()->setText(files.join(QStringLiteral("\n")));
    }
}

void TableView::copyWithSize()
{
    QItemSelectionModel *selection = selectionModel();
    QModelIndexList nameRows = selection->selectedRows(0);
    QModelIndexList sizeRows = selection->selectedRows(1);
    QStringList files;

    for (int i = 0; i < nameRows.length(); ++i) {
        files << nameRows.at(i).data().toString() + QLatin1String("\t") + sizeRows.at(i).data().toString();
    }

    if (!files.isEmpty()) {
        QApplication::clipboard()->setText(files.join(QStringLiteral("\n")));
    }
}

void TableView::copyWoExt()
{
    QItemSelectionModel *selection = selectionModel();
    QModelIndexList nameRows = selection->selectedRows(0);
    QStringList files;

    for (int i = 0; i < nameRows.length(); ++i) {
        QString filename = nameRows.at(i).data().toString();

        if (filename.contains(QDir::separator())) {
            filename = filename.section(QDir::separator(), -1);
        }

        if (filename.contains(QLatin1String("."))) {
            filename = filename.section(QLatin1String("."), 0, -2);
        }
        files << filename;
    }

    if (!files.isEmpty()) {
        QApplication::clipboard()->setText(files.join(QStringLiteral("\n")));
    }
}

QModelIndex TableView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) // -V813 PVS-Studio
{

    switch (cursorAction) {
    case QAbstractItemView::MoveHome:
        return model()->index(0, 0);
        break;

    case QAbstractItemView::MoveEnd:
        return model()->index(model()->rowCount() - 1, 0);
        break;

    default:
        return QTableView::moveCursor(cursorAction, modifiers);
        break;
    }
}

void TableView::keyPressEvent(QKeyEvent *event)
{
    if (event == QKeySequence::Delete) {
        emit deleteRow();
    }
    else {
        QTableView::keyPressEvent(event);
    }
}

void TableView::contextMenuEvent(QContextMenuEvent *event)
{
    bool hasSelection = selectionModel()->hasSelection();
    _copyAct->setEnabled(hasSelection);
    _copySizeAct->setEnabled(hasSelection);
    _copyWoExtAct->setEnabled(hasSelection);

    _menu->exec(event->globalPos());
}

void TableView::changeEvent(QEvent *event)
{
    switch(event->type()) {
    case QEvent::LanguageChange:
        updateTranslations();
        break;

    default:
        break;
    }

    QTableView::changeEvent(event);
}

void TableView::updateTranslations()
{
    _copyAct->setText(tr("Copy") + ACCEL_KEY(QKeySequence::Copy));
    _copySizeAct->setText(tr("Copy with Size"));
    _copyWoExtAct->setText(tr("Copy Filename"));
}
