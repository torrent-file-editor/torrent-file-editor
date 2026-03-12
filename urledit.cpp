// SPDX-FileCopyrightText: 2014, 2016-2020, 2023, 2026 Ivan Romanov <drizt72@zoho.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "urledit.h"

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QPushButton>
#include <QUrl>

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

    retranslateUi(); // NOLINT(clang-analyzer-optin.cplusplus.VirtualCall)
}

void UrlEdit::openUrl()
{
    QUrl url(text());
    if (url.isValid()) {
        QDesktopServices::openUrl(url);
    }
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
