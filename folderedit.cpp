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

#include "folderedit.h"

#include <QUrl>
#include <QPushButton>
#include <QApplication>
#include <QStyle>
#include <QFileDialog>

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

    retranslateUi();
}

void FolderEdit::setFolder(const QString &path)
{
    _path = path;
}

void FolderEdit::openFolder()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Add Folder"), _path);
    if (path.isEmpty())
        return;

    _path = path;
    setText(path);
    emit textEdited(path);
}

void FolderEdit::retranslateUi()
{
    _pbOpenFolder->setToolTip(tr("Choose folder"));
}
