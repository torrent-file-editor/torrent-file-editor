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

#include <QList>
#include <QString>

class AbstractTreeItem
{
public:
    explicit AbstractTreeItem(AbstractTreeItem *parent = 0);
    virtual ~AbstractTreeItem();

    void setRow(int row);
    int row() const;

    void setParent(AbstractTreeItem *newParent);
    AbstractTreeItem *parent() const;

    void insertChild(int row, AbstractTreeItem *child);
    void appendChild(AbstractTreeItem *child);
    void removeChild(AbstractTreeItem *child);

    AbstractTreeItem *child(int row) const;
    int childCount() const;
    QList<AbstractTreeItem*> children() const;

    virtual AbstractTreeItem *clone() const = 0;
    void dump(int indent = 0) const;
    virtual QString toString() const = 0;

private:
    AbstractTreeItem *_parent;
    QList<AbstractTreeItem*> _children;
};
