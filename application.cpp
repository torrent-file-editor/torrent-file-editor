// SPDX-FileCopyrightText: 2014, 2016-2018, 2026 Ivan Romanov <drizt72@zoho.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <config.h>

#include "application.h"
#include "mainwindow.h"
#include "proxystyle.h"

#include <QFileOpenEvent>

Application::Application(int &argc, char **argv)
    : QApplication(argc, argv)
    , _mainWindow(nullptr)
{
    setApplicationName(QStringLiteral(APP_NAME));
    setApplicationVersion(QStringLiteral(APP_VERSION));

    setStyle(new ProxyStyle());

    if (QIcon::themeName().isEmpty()) { // NOLINT(clang-analyzer-cplusplus.NewDeleteLeaks)
        QIcon::setThemeName(QStringLiteral("gnome"));
    }
}

bool Application::event(QEvent *event)
{
    // To use Mac OS X bundle for opening .torrent files
    if (event->type() == QEvent::FileOpen) {
        QFileOpenEvent *fileOpenEvent = static_cast<QFileOpenEvent *>(event);
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
