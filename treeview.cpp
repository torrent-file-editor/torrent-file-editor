// SPDX-FileCopyrightText: 2016-2017, 2023, 2026 Ivan Romanov <drizt72@zoho.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "treeview.h"
#include "bencodemodel.h"
#include "combobox.h"

#include <QComboBox>
#include <QMouseEvent>

TreeView::TreeView(QWidget *parent)
    : QTreeView(parent)
{
    setMouseTracking(true);
}

void TreeView::mouseMoveEvent(QMouseEvent *event)
{
    QTreeView::mouseMoveEvent(event);

    // Close before opened edit widgets
    QList<ComboBox *> list = findChildren<ComboBox *>();
    for (auto item : list) {
        item->close();
    }

    QModelIndex index = indexAt(event->pos());
    if (static_cast<BencodeModel::Column>(index.column()) == BencodeModel::Column::Type
        && (index.flags() & Qt::ItemIsEditable)) {
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
    QList<QComboBox *> list = findChildren<QComboBox *>();
    if (isActiveWindow()) {
        for (auto widget : list) {
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
