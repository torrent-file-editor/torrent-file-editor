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

#include <config.h>

#include "application.h"
#include "mainwindow.h"
#include "proxystyle.h"

#include <QFileOpenEvent>

Application::Application(int &argc, char **argv)
    : QApplication(argc, argv)
{
    setApplicationName(QLatin1String("Torrent File Editor"));
    setApplicationVersion(QLatin1String(APP_VERSION));

    setStyle(new ProxyStyle());
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
    return QDateTime(QDate::fromString(QLatin1String(APP_COMPILATION_DATE), Qt::DateFormat::ISODate),
                     QTime::fromString(QLatin1String(APP_COMPILATION_TIME), Qt::DateFormat::ISODate));
}
