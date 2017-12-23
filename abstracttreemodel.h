/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 *
 * Copyright (C) 2015, 2017  Ivan Romanov <drizt72@zoho.eu>
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

#pragma once

#include <QAbstractItemModel>

#include "abstracttreenode.h"

template<typename T>
class AbstractTreeModel : public QAbstractItemModel
{
public:
    explicit AbstractTreeModel(T *root, QObject *parent = nullptr)
        : QAbstractItemModel(parent)
        , _root(root)
    {
    }

    ~AbstractTreeModel() override
    {
        delete _root;
    }

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override
    {
        if (!hasIndex(row, column, parent)) {
            return QModelIndex();
        }

        T *parentItem;

        parentItem = indexToNode(parent);

        T *childItem = nullptr;
        if (row >= 0 && row < parentItem->childCount()) {
            childItem = parentItem->children().at(row);
        }

        if (childItem) {
            return createIndex(row, column, childItem);
        }
        else {
            return QModelIndex();
        }

    }

    QModelIndex parent(const QModelIndex &index) const override
    {
        if (!index.isValid()) {
            return QModelIndex();
        }

        T *childItem = indexToNode(index);
        T *parentItem = childItem->parent();

        if (parentItem == _root) {
            return QModelIndex();
        }

        int row = 0;
        if (parentItem->parent()) {
            row = parentItem->row();
        }

        return createIndex(row, 0, parentItem);
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        if (parent.column() > 0) {
            return 0;
        }

        T *parentItem;
        if (!parent.isValid()) {
            parentItem = _root;
        }
        else {
            parentItem = indexToNode(parent);
        }

        return parentItem->childCount();
    }

protected:
    inline T *root() const
    {
        return _root;
    }

    inline T *indexToNode(const QModelIndex &index) const
    {
        return index.isValid() ? static_cast<T*>(index.internalPointer()) : _root;
    }

    inline QModelIndex nodeToIndex(T *node) const
    {
        Q_ASSERT(node);
        return (node == _root || !node) ? QModelIndex() : createIndex(node->row(), 0, node); // -V560 PVS-Studio
    }

private:
    T *_root;
};
