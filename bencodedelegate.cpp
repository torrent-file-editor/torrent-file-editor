/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 *
 * Copyright (C) 2015  Ivan Romanov <drizt72@zoho.eu>
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
#include "combobox.h"

#include <QEvent>

BencodeDelegate::BencodeDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QWidget *BencodeDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (static_cast<BencodeModel::Column>(index.column()) != BencodeModel::Column::Type)
        return QStyledItemDelegate::createEditor(parent, option, index);

    ComboBox *comboBox = new ComboBox(parent);
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
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    if (static_cast<BencodeModel::Column>(index.column()) != BencodeModel::Column::Hex)
        size += QSize(0, 8);

    if (static_cast<BencodeModel::Column>(index.column()) == BencodeModel::Column::Type)
        size.setWidth(typeSizeHint().width());
    return size;
}

QSize BencodeDelegate::typeSizeHint()
{
    static QSize size;
    if (!size.isValid()) {
        ComboBox *comboBox = new ComboBox;
        comboBox->addItem(Bencode::typeToStr(Bencode::Type::Integer), static_cast<int>(Bencode::Type::Integer));
        comboBox->addItem(Bencode::typeToStr(Bencode::Type::String), static_cast<int>(Bencode::Type::String));
        comboBox->addItem(Bencode::typeToStr(Bencode::Type::List), static_cast<int>(Bencode::Type::List));
        comboBox->addItem(Bencode::typeToStr(Bencode::Type::Dictionary), static_cast<int>(Bencode::Type::Dictionary));
        comboBox->ensurePolished();
        comboBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        comboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        comboBox->adjustSize();
        size = comboBox->size();
        delete comboBox;
    }
    return size;
}
