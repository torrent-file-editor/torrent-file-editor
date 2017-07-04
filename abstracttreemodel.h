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

#pragma once

#include <QAbstractItemModel>

class AbstractTreeItem;

class AbstractTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit AbstractTreeModel(AbstractTreeItem *root, QObject *parent = 0);
    ~AbstractTreeModel() override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex & index) const override;
    int rowCount(const QModelIndex & parent = QModelIndex()) const override;

protected:
    AbstractTreeItem *root() const;

private:
    AbstractTreeItem *_root;
};
