// SPDX-FileCopyrightText: 2016-2017, 2019, 2026 Ivan Romanov <drizt72@zoho.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "combobox.h"

#include <QAbstractItemView>
#include <QCursor>
#include <QShowEvent>
#include <QTimer>

ComboBox::ComboBox(QWidget *parent)
    : QComboBox(parent)
{
    connect(this, SIGNAL(activated(int)), SLOT(close()));
}

void ComboBox::hidePopup()
{
    if (view()->isVisible()) {
        QTimer::singleShot(0, this, SLOT(close()));
    }
    QComboBox::hidePopup();
}

void ComboBox::leaveEvent(QEvent *event)
{
    QComboBox::leaveEvent(event);
    if (!view()->isVisible()) {
        close();
    }
}

void ComboBox::wheelEvent(QWheelEvent *event)
{
    Q_UNUSED(event);

    // do nothing to prevent unexpected table changes
    // also see https://stackoverflow.com/questions/3241830/qt-how-to-disable-mouse-scrolling-of-qcombobox
}

void ComboBox::close()
{
    QComboBox::close();
    parentWidget()->setFocus();
}
