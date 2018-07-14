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

#ifdef HAVE_QT5
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

    _copyAct = new QAction(QLineEdit::tr("&Copy") + ACCEL_KEY(QKeySequence::Copy), this);
    _copyAct->setIcon(QIcon::fromTheme(QLatin1String("edit-copy")));
    _copyAct->setShortcut(QKeySequence::Copy);
    connect(_copyAct, SIGNAL(triggered()), SLOT(copy()));
    _menu->addAction(_copyAct);
    addAction(_copyAct);

    _copySizeAct = new QAction(QLatin1String("Copy with Size\tCtrl+Shift+C"), this);
    _copySizeAct->setShortcut(QKeySequence(QLatin1String("Ctrl+Shift+C")));
    connect(_copySizeAct, SIGNAL(triggered()), SLOT(copyWithSize()));
    _menu->addAction(_copySizeAct);
    addAction(_copySizeAct);
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

    _menu->exec(event->globalPos());
}
