// SPDX-FileCopyrightText: 2014, 2016-2018, 2020, 2026 Ivan Romanov <drizt72@zoho.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "folderedit.h"

#include <QApplication>
#include <QFileDialog>
#include <QPushButton>
#include <QStyle>
#include <QUrl>

FolderEdit::FolderEdit(QWidget *parent)
    : LineEditWidget(parent)
    , _pbOpenFolder(new QPushButton(this))
    , _path()
{
    _pbOpenFolder->setObjectName(QStringLiteral("pbOpenUrl"));
    _pbOpenFolder->setIcon(qApp->style()->standardIcon(QStyle::SP_DirOpenIcon));
    _pbOpenFolder->setContentsMargins(0, 0, 0, 0);
    _pbOpenFolder->setFocusPolicy(Qt::NoFocus);
    _pbOpenFolder->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    _pbOpenFolder->setAutoFillBackground(true);
    _pbOpenFolder->setCursor(Qt::PointingHandCursor);
    _pbOpenFolder->resize(0, 0);
    _pbOpenFolder->setFlat(true);
    _pbOpenFolder->setMinimumWidth(24);
    _pbOpenFolder->setMaximumWidth(24);
    addWidget(_pbOpenFolder);

    connect(_pbOpenFolder, SIGNAL(clicked()), SLOT(openFolder()));

    retranslateUi(); // NOLINT(clang-analyzer-optin.cplusplus.VirtualCall)
}

void FolderEdit::setFolder(const QString &path)
{
    _path = path;
}

void FolderEdit::openFolder()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Add Folder"), _path);
    if (path.isEmpty()) {
        return;
    }

    _path = path;
    setText(path);
    emit textEdited(path);
}

void FolderEdit::retranslateUi()
{
    _pbOpenFolder->setToolTip(tr("Choose folder"));
}
