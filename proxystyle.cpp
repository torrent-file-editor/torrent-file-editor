// SPDX-FileCopyrightText: 2015, 2017, 2026 Ivan Romanov <drizt72@zoho.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "proxystyle.h"

void ProxyStyle::drawPrimitive(PrimitiveElement element,
                               const QStyleOption *option,
                               QPainter *painter,
                               const QWidget *widget) const
{
    if (PE_FrameFocusRect == element) {
        // do not draw focus rectangle
    } else {
        QProxyStyle::drawPrimitive(element, option, painter, widget);
    }
}
