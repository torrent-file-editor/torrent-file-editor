/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 *
 * Copyright (C) 2014  Ivan Romanov <drizt@land.ru>
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

#include "urledit.h"

#include <QUrl>
#include <QPushButton>
#include <QDesktopServices>

UrlEdit::UrlEdit(QWidget *parent)
    : LineEditWidget(parent)
    , _pbOpenUrl(new QPushButton(this))
{
    _pbOpenUrl->setObjectName(QLatin1String("pbOpenUrl"));
    _pbOpenUrl->setIcon(QIcon::fromTheme(QLatin1String("applications-internet"), QIcon(QLatin1String(":/icons/applications-internet.png"))));
    _pbOpenUrl->setContentsMargins(0, 0, 0, 0);
    _pbOpenUrl->setFocusPolicy(Qt::NoFocus);
    _pbOpenUrl->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    _pbOpenUrl->setIconSize(QSize(16, 16));
    _pbOpenUrl->setAutoFillBackground(true);
    _pbOpenUrl->setCursor(Qt::PointingHandCursor);
    _pbOpenUrl->resize(0, 0);
    _pbOpenUrl->setFlat(true);
    _pbOpenUrl->setToolTip(tr("Open in internet browser"));
    _pbOpenUrl->setMinimumWidth(24);
    _pbOpenUrl->setMaximumWidth(24);
    addWidget(_pbOpenUrl);

    connect(_pbOpenUrl, SIGNAL(clicked()), SLOT(openUrl()));
}

void UrlEdit::openUrl()
{
    QUrl url(text());
    if (url.isValid())
        QDesktopServices::openUrl(url);
}
