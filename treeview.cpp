/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 *
 * Copyright (C) 2016  Ivan Romanov <drizt72@zoho.eu>
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

#include "treeview.h"
#include "bencodemodel.h"
#include "combobox.h"

#include <QMouseEvent>
#include <QComboBox>

TreeView::TreeView(QWidget *parent)
    : QTreeView(parent)
{
    setMouseTracking(true);
}

void TreeView::mouseMoveEvent(QMouseEvent *event)
{
    QTreeView::mouseMoveEvent(event);

    // Close before opened edit widgets
    QList<ComboBox*> list = findChildren<ComboBox*>();
    for (auto item: list) {
        item->close();
    }

    QModelIndex index = indexAt(event->pos());
    if (static_cast<BencodeModel::Column>(index.column()) == BencodeModel::Column::Type && (index.flags() & Qt::ItemIsEditable)) {
        edit(index);
    }
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void TreeView::initViewItemOption(QStyleOptionViewItem *option) const
{
    QTreeView::initViewItemOption(option);
    QStyleOptionViewItem &options = *option;
#else
QStyleOptionViewItem TreeView::viewOptions() const
{
    QStyleOptionViewItem options = QTreeView::viewOptions();
#endif
    // Hack. Draw active focused tree when type combobox is showed
    // not just current line as in original QTreeView
    QList<QComboBox*> list = findChildren<QComboBox*>();
    if (isActiveWindow()) {
        for (auto widget: list) {
            if (widget->hasFocus()) {
                options.state |= QStyle::State_Active;
                break;
            }
        }
    }
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    return options;
#endif
}
