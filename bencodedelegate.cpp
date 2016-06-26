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

#include "bencodedelegate.h"
#include "bencodemodel.h"
#include "bencode.h"

#include <QComboBox>

BencodeDelegate::BencodeDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QWidget *BencodeDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (static_cast<BencodeModel::Column>(index.column()) != BencodeModel::Column::Type)
        return QStyledItemDelegate::createEditor(parent, option, index);

    QComboBox *comboBox = new QComboBox(parent);
    comboBox->addItem(Bencode::typeToStr(Bencode::Type::Integer), static_cast<int>(Bencode::Type::Integer));
    comboBox->addItem(Bencode::typeToStr(Bencode::Type::String), static_cast<int>(Bencode::Type::String));
    comboBox->addItem(Bencode::typeToStr(Bencode::Type::List), static_cast<int>(Bencode::Type::List));
    comboBox->addItem(Bencode::typeToStr(Bencode::Type::Dictionary), static_cast<int>(Bencode::Type::Dictionary));

    comboBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    comboBox->setMinimumHeight(option.rect.height());
    comboBox->setMaximumHeight(option.rect.height());
    comboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    return comboBox;
}

void BencodeDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (static_cast<BencodeModel::Column>(index.column()) != BencodeModel::Column::Type)
        return QStyledItemDelegate::setEditorData(editor, index);

    QComboBox *comboBox = qobject_cast<QComboBox*>(editor);
    int type = static_cast<int>(static_cast<Bencode*>(index.internalPointer())->type());
    comboBox->setCurrentIndex(comboBox->findData(type));
}

void BencodeDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (static_cast<BencodeModel::Column>(index.column()) != BencodeModel::Column::Type)
        return QStyledItemDelegate::setModelData(editor, model, index);

    QComboBox *comboBox = qobject_cast<QComboBox*>(editor);
    Bencode::Type type = static_cast<Bencode::Type>(comboBox->itemData(comboBox->currentIndex()).toInt());
    qobject_cast<BencodeModel*>(model)->changeType(index, type);
}

QSize BencodeDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //    if (static_cast<BencodeModel::Column>(index.column()) != BencodeModel::Column::Type)
        return QStyledItemDelegate::sizeHint(option, index);
}
