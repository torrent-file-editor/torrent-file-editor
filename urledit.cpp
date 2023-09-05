/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 *
 * Copyright (C) 2014  Ivan Romanov <drizt72@zoho.eu>
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
#include <QApplication>
#include <QClipboard>

UrlEdit::UrlEdit(QWidget *parent)
    : LineEditWidget(parent)
    , _pbOpenUrl(new QPushButton(this))
    , _pbCopyUrl(new QPushButton(this))
{
    _pbCopyUrl->setObjectName(QStringLiteral("pbCopyUrl"));
    _pbCopyUrl->setIcon(QIcon::fromTheme(QStringLiteral("edit-copy")));
    _pbCopyUrl->setContentsMargins(0, 0, 0, 0);
    _pbCopyUrl->setFocusPolicy(Qt::NoFocus);
    _pbCopyUrl->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    _pbCopyUrl->setAutoFillBackground(true);
    _pbCopyUrl->setCursor(Qt::PointingHandCursor);
    _pbCopyUrl->resize(0, 0);
    _pbCopyUrl->setFlat(true);
    _pbCopyUrl->setMinimumWidth(24);
    _pbCopyUrl->setMaximumWidth(24);
    addWidget(_pbCopyUrl);

    connect(_pbCopyUrl, SIGNAL(clicked()), SLOT(copyAll()));

    _pbOpenUrl->setObjectName(QStringLiteral("pbOpenUrl"));
    _pbOpenUrl->setIcon(QIcon::fromTheme(QStringLiteral("applications-internet")));
    _pbOpenUrl->setContentsMargins(0, 0, 0, 0);
    _pbOpenUrl->setFocusPolicy(Qt::NoFocus);
    _pbOpenUrl->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    _pbOpenUrl->setAutoFillBackground(true);
    _pbOpenUrl->setCursor(Qt::PointingHandCursor);
    _pbOpenUrl->resize(0, 0);
    _pbOpenUrl->setFlat(true);
    _pbOpenUrl->setMinimumWidth(24);
    _pbOpenUrl->setMaximumWidth(24);
    addWidget(_pbOpenUrl);

    connect(_pbOpenUrl, SIGNAL(clicked()), SLOT(openUrl()));

    retranslateUi();
}

void UrlEdit::openUrl()
{
    QUrl url(text());
    if (url.isValid())
        QDesktopServices::openUrl(url);
}

void UrlEdit::copyAll()
{
    qApp->clipboard()->setText(text());
}

void UrlEdit::retranslateUi()
{
    _pbCopyUrl->setToolTip(tr("Copy"));
    _pbOpenUrl->setToolTip(tr("Open in internet browser"));
}
