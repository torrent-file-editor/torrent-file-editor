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

#include "mainwindow.h"

#include <QApplication>
#include <QTranslator>
#include <QLocale>

#ifdef HAVE_QT5
void debugHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
#else
void debugHandler(QtMsgType type, const char *msg)
#endif
{
#ifdef HAVE_QT5
    Q_UNUSED(context);
#endif
    Q_UNUSED(type);

    if (MainWindow::instance())
        MainWindow::instance()->addLog(QString(msg));
}

int main(int argc, char *argv[])
{
#ifdef HAVE_QT5
    qInstallMessageHandler(debugHandler);
#else
    qInstallMsgHandler(debugHandler);
#endif
    QApplication a(argc, argv);

    QString lang = QLocale().name().section('_', 0, 0);

    QTranslator translator;
    translator.load("torrentfileeditor_" + lang, ":/translations");
    a.installTranslator(&translator);

    MainWindow w;
    w.show();

    return a.exec();
}
