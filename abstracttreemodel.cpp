/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 *
 * Copyright (C) 2015  Ivan Romanov <drizt@land.ru>
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

#include "abstracttreeitem.h"
#include "abstracttreemodel.h"

#include <QDebug>

AbstractTreeModel::AbstractTreeModel(AbstractTreeItem *root, QObject *parent)
    : QAbstractItemModel(parent)
    , _root(root)
{
}

AbstractTreeModel::~AbstractTreeModel()
{
    delete _root;
}

QModelIndex AbstractTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    AbstractTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = _root;
    else
        parentItem = static_cast<AbstractTreeItem*>(parent.internalPointer());

    AbstractTreeItem *childItem = 0;
    if (row >= 0 && row < parentItem->childCount())
        childItem = parentItem->children().at(row);

    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();

}

QModelIndex	AbstractTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    AbstractTreeItem *childItem = static_cast<AbstractTreeItem*>(index.internalPointer());
    AbstractTreeItem *parentItem = childItem->parent();

    if (parentItem == _root)
        return QModelIndex();

    int row = 0;
    if (parentItem->parent())
        row = parentItem->row();

    return createIndex(row, 0, parentItem);
}

int	AbstractTreeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;

    AbstractTreeItem *parentItem;
    if (!parent.isValid())
        parentItem = _root;
    else
        parentItem = static_cast<AbstractTreeItem*>(parent.internalPointer());

    return parentItem->children().size();
}

AbstractTreeItem *AbstractTreeModel::root() const
{
    return _root;
}
