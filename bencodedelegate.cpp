// SPDX-FileCopyrightText: 2015-2017, 2026 Ivan Romanov <drizt72@zoho.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bencodedelegate.h"
#include "bencode.h"
#include "bencodemodel.h"
#include "combobox.h"

#include <QEvent>

BencodeDelegate::BencodeDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QWidget *BencodeDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (static_cast<BencodeModel::Column>(index.column()) != BencodeModel::Column::Type) {
        return QStyledItemDelegate::createEditor(parent, option, index);
    }

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
    if (static_cast<BencodeModel::Column>(index.column()) != BencodeModel::Column::Type) {
        return QStyledItemDelegate::setEditorData(editor, index);
    }

    QComboBox *comboBox = qobject_cast<QComboBox *>(editor);
    int type = static_cast<int>(static_cast<Bencode *>(index.internalPointer())->type());
    comboBox->setCurrentIndex(comboBox->findData(type));
}

void BencodeDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (static_cast<BencodeModel::Column>(index.column()) != BencodeModel::Column::Type) {
        return QStyledItemDelegate::setModelData(editor, model, index);
    }

    QComboBox *comboBox = qobject_cast<QComboBox *>(editor);
    Bencode::Type type = static_cast<Bencode::Type>(comboBox->itemData(comboBox->currentIndex()).toInt());
    qobject_cast<BencodeModel *>(model)->changeType(index, type);
}

QSize BencodeDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    if (static_cast<BencodeModel::Column>(index.column()) != BencodeModel::Column::Hex) {
        size += QSize(0, 8);
    }

    if (static_cast<BencodeModel::Column>(index.column()) == BencodeModel::Column::Type) {
        size.setWidth(typeSizeHint().width());
    }
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
