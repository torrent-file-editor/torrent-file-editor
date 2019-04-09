/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 *
 * Copyright (C) 2016  Ivan Romanov <drizt72@zoho.eu>
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

#include "combobox.h"

#include <QAbstractItemView>
#include <QTimer>
#include <QShowEvent>
#include <QCursor>

ComboBox::ComboBox(QWidget *parent)
    : QComboBox(parent)
{
    connect(this, SIGNAL(activated(int)), SLOT(close()));
}

void ComboBox::hidePopup()
{
    if (view()->isVisible())
        QTimer::singleShot(0, this, SLOT(close()));
    QComboBox::hidePopup();
}

void ComboBox::leaveEvent(QEvent *event)
{
    QComboBox::leaveEvent(event);
    if (!view()->isVisible())
        close();
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
