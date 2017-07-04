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

#include <QDebug>

AbstractTreeItem::AbstractTreeItem(AbstractTreeItem *parent)
    : _parent(0)
    , _children(QList<AbstractTreeItem*>())
{
    setParent(parent);
}

AbstractTreeItem::~AbstractTreeItem()
{
    qDeleteAll(_children);
    if (_parent)
        _parent->_children.removeOne(this);
}

void AbstractTreeItem::setRow(int row)
{
    Q_ASSERT(_parent);
    if (!_parent)
        return;

    _parent->_children.move(this->row(), row);
}

int AbstractTreeItem::row() const
{
    return _parent ? _parent->_children.indexOf(const_cast<AbstractTreeItem*>(this)) : 0;
}

void AbstractTreeItem::setParent(AbstractTreeItem *newParent)
{
    if (_parent)
        _parent->_children.removeOne(this);

    if (newParent)
        newParent->_children.append(this);

    _parent = newParent;
}

AbstractTreeItem *AbstractTreeItem::parent() const
{
    return _parent;
}

void AbstractTreeItem::insertChild(int row, AbstractTreeItem *child)
{
    Q_ASSERT(child);
    Q_ASSERT(!child->_parent);

    if (child->_parent)
        child->_parent->removeChild(child);

    child->_parent = this;
    _children.insert(row, child);
}

void AbstractTreeItem::appendChild(AbstractTreeItem *child)
{
    Q_ASSERT(child);
    Q_ASSERT(!child->_parent);

    if (child->_parent)
        child->_parent->removeChild(child);

    child->_parent = this;
    _children.append(child);
}

void AbstractTreeItem::removeChild(AbstractTreeItem *child)
{
    Q_ASSERT(child);
    Q_ASSERT(_children.contains(child));

    _children.removeOne(child);
}

AbstractTreeItem *AbstractTreeItem::child(int row) const
{
    Q_ASSERT(row < childCount());
    if (row < childCount())
        return _children.at(row);
    else
        return 0;
}

int AbstractTreeItem::childCount() const
{
    return _children.size();
}

QList<AbstractTreeItem*> AbstractTreeItem::children() const
{
    return _children;
}

void AbstractTreeItem::dump(int indent) const
{
    QString fill(indent, QLatin1Char(' '));
    qDebug() << qPrintable(fill + QLatin1String("{"));
    qDebug() << qPrintable(fill + QLatin1String(" ") + toString());
    qDebug() << qPrintable(fill + QLatin1String(" ") + QLatin1String("this")) << this;
    qDebug() << qPrintable(fill + QLatin1String(" ") + QLatin1String("parent")) << _parent;
    qDebug() << qPrintable(fill + QLatin1String(" ") + QLatin1String("children")) << _children;

    foreach (AbstractTreeItem *item, _children) {
        item->dump(indent + 1);
    }
    qDebug() << qPrintable(fill + QLatin1String("}"));
}
