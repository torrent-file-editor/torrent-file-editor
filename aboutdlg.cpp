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
#include "application.h"

#define VERSION_LABEL                               \
    "<style>"                                       \
    "h2 {"                                          \
    "margin-bottom: 0;"                             \
    "}"                                             \
    "p {"                                           \
    "margin-top: 0;"                                \
    "}"                                             \
    "</style>"                                      \
    "<h2>%1</h2><p>%2 (%3)</p>"

AboutDlg::AboutDlg(QWidget *parent)
#ifdef Q_OS_WIN
    : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint)
#else
    : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
#endif
    , ui(new Ui::AboutDlg)
{
    ui->setupUi(this);
    ui->label->setText(QString(VERSION_LABEL)
                       .arg(qApp->applicationName())
                       .arg(qApp->applicationVersion())
                       .arg(Application::buildDateTime().date().toString(Qt::DefaultLocaleLongDate)));
    setWindowTitle(QString(tr("About %1")).arg(qApp->applicationName()));

#ifdef NO_DONATION
    ui->wdgDonation->hide();
#endif
}

AboutDlg::~AboutDlg()
{
    delete ui;
}
