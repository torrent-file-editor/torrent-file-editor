/*
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

#include "aboutdlg.h"
#include "ui_aboutdlg.h"

#include <config.h>

#include <QApplication>
#include <QDate>

AboutDlg::AboutDlg(QWidget *parent)
#ifdef Q_OS_WIN
    : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint)
#else
    : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
#endif
    , ui(new Ui::AboutDlg)
{
    ui->setupUi(this);
    QString version = APP_VERSION + QString(" (");
    QDate date = QDate::fromString(APP_COMPILATION_DATE, Qt::ISODate);
    version += date.toString(Qt::DefaultLocaleLongDate) + ")";
    ui->lblVersion->setText(version);
    setWindowTitle(QString(tr("About %1")).arg(qApp->applicationName()));
}

AboutDlg::~AboutDlg()
{
    delete ui;
}
