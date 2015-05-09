/*
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

#include "bencodemodel.h"
#include "bencode.h"

// TreeItem::TreeItem(const QString &name, AbstractTreeItem *parent)
//     : AbstractTreeItem(parent)
//     , _name(name)
// {
// }

// TreeItem::~TreeItem()
// {
// }

// QString TreeItem::name() const
// {
//     return _name;
// }

// TreeItem *TreeItem::clone() const
// {
//     TreeItem *newItem = new TreeItem;
//     newItem->_name = _name;

//     foreach (AbstractTreeItem *child, children()) {
//         newItem->appendChild(child->clone());
//     }
//     return newItem;
// }

// QString TreeItem::toString() const
// {
//     return "name " + _name;
// }

BencodeModel::BencodeModel(QObject *parent)
    : AbstractTreeModel(new Bencode, parent)
{
}

BencodeModel::~BencodeModel()
{
}

// void BencodeModel::add(const QString &name, const QModelIndex &index)
// {
//     beginInsertRows(index, rowCount(index), rowCount(index));
//     TreeItem *item;
//     if (index.isValid())
//         item = static_cast<TreeItem*>(index.internalPointer());
//     else
//         item = static_cast<TreeItem*>(root());

//     new TreeItem(name, item);
//     endInsertRows();
// }

// void BencodeModel::remove(const QModelIndex &index)
// {
//     TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
//     if (item == root() || !item)
//         return;

//     beginRemoveRows(index.parent(), item->row(), item->row());
//     delete item;
//     endRemoveRows();
// }

// void BencodeModel::up(const QModelIndex &index)
// {
//     TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
//     if (!item || root() == item)
//         return;

//     if (index.row() == 0)
//         return;

//     beginMoveRows(index.parent(), index.row(), index.row(), index.parent(), index.row() - 1);
//     item->setRow(index.row() - 1);
//     endMoveRows();
// }

// void BencodeModel::down(const QModelIndex &index)
// {
//     TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
//     if (!item || root() == item)
//         return;

//     if (index.row() + 1 == rowCount(index.parent()))
//         return;

//     beginMoveRows(index.parent(), index.row(), index.row(), index.parent(), index.row() + 2);
//     item->setRow(index.row() + 1);
//     endMoveRows();
// }

int BencodeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return static_cast<int>(Columns::Count);
}

QVariant BencodeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    // if (role == Qt::DisplayRole) {
    //     TreeItem *node = static_cast<TreeItem*>(index.internalPointer());
    //     if (node) {
    //         return node->name();
    //     }
    // }

    return QVariant();
}
