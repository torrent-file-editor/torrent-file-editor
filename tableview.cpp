/*
 * Copyright (C) 2014-2015  Ivan Romanov <drizt@land.ru>
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

#include <QKeyEvent>

TableView::TableView(QWidget *parent)
    : QTableView(parent)
{
}

QModelIndex TableView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
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
    if (event->key() == Qt::Key_Delete) {
        emit deleteRow();
    }
    else {
        QTableView::keyPressEvent(event);
    }
}
