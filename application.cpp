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

#include <config.h>

#include "application.h"
#include "mainwindow.h"
#include "proxystyle.h"

#include <QFileOpenEvent>

Application::Application(int &argc, char **argv)
    : QApplication(argc, argv)
    , _mainWindow(nullptr)
{
    setApplicationName(QStringLiteral("Torrent File Editor"));
    setApplicationVersion(QStringLiteral(APP_VERSION));

    setStyle(new ProxyStyle());

    if (QIcon::themeName().isEmpty()) {
        QIcon::setThemeName(QStringLiteral("gnome"));
    }
}

bool Application::event(QEvent *event)
{
    // To use Mac OS X bundle for opening .torrent files
    if (event->type() == QEvent::FileOpen) {
        QFileOpenEvent *fileOpenEvent = static_cast<QFileOpenEvent*>(event);
        _mainWindow->open(fileOpenEvent->file());
    }

    return QApplication::event(event);
}

void Application::setMainWindow(MainWindow *mainWindow)
{
    _mainWindow = mainWindow;
}

QDateTime Application::buildDateTime()
{
    return QDateTime(QDate::fromString(QStringLiteral(APP_COMPILATION_DATE), Qt::DateFormat::ISODate),
                     QTime::fromString(QStringLiteral(APP_COMPILATION_TIME), Qt::DateFormat::ISODate));
}
