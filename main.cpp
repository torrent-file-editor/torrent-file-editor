/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 *
 * Copyright (C) 2014-2015  Ivan Romanov <drizt72@zoho.eu>
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

#include "application.h"
#include "bencode.h"
#include "jsonconverter.h"
#include "mainwindow.h"

#include <QVariant>
#include <QFile>

// Allow run Qt5 static version https://github.com/tonytheodore/mxe/commit/497669fa44356db0cd8335e2554b7bac12eb88c2
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0) && defined Q_OS_WIN && defined BUILD_STATIC
#include <QtPlugin>
Q_IMPORT_PLUGIN(QWindowsVistaStylePlugin)
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
Q_IMPORT_PLUGIN(QGifPlugin)
Q_IMPORT_PLUGIN(QICOPlugin)
Q_IMPORT_PLUGIN(QJpegPlugin)
#endif

#ifdef Q_OS_WIN
# include <windows.h>
HANDLE hConsole = NULL;
#endif

#ifdef Q_OS_MAC
# include "cocoainitializer.h"
# include "sparkleautoupdater.h"
#endif

#ifdef ENABLE_NVWA
# include "nvwa/debug_new.h"
#endif

#ifdef Q_OS_WIN
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
void winDebugHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
# else
void winDebugHandler(QtMsgType type, const char *msg)
# endif
{
    {
        QString time = QTime::currentTime().toString();
        QString debugMsg = QStringLiteral("[%1] ").arg(time);
        switch (type) {
        case QtDebugMsg:
            break;
        case QtWarningMsg:
            debugMsg += QLatin1String("W:");
            break;
        case QtCriticalMsg:
            debugMsg += QLatin1String("C:");
            break;
# if QT_VERSION >= 0x050500
        case QtInfoMsg:
            debugMsg += QLatin1String("I:");
            break;
# endif

        case QtFatalMsg:
            debugMsg += QLatin1String("F:");
            break;
            abort();
        }
# if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        debugMsg += msg;
        debugMsg += QStringLiteral(" (%1:%2, %3)").arg(QString::fromUtf8(context.file), QString::number(context.line), QString::fromUtf8(context.function));
# else
        debugMsg += QLatin1String(msg);
# endif


# ifdef UNICODE
        OutputDebugString(debugMsg.toStdWString().c_str());
# else
        OutputDebugString(debugMsg.toStdString().c_str());
# endif

        if (type == QtFatalMsg)
            abort();
    }
}
#endif

void openWinConsole()
{
#ifdef Q_OS_WIN
    BOOL b = AllocConsole();
    if (!b)
        return;

    hConsole = GetStdHandle(STD_INPUT_HANDLE);

    freopen("CONIN$", "r", stdin);
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
#endif
}

void closeWinConsole()
{
#ifdef Q_OS_WIN
    if (!hConsole)
        return;

    printf("\nPress any key to close window...\n");
    getchar();
    FreeConsole();
#endif
}

bool toJson(const QString &source, const QString &dest)
{
    QFile sourceFile(source);
    if (!sourceFile.open(QIODevice::ReadOnly)) {
        qDebug("Error: can't open source file");
        return false;
    }

    QByteArray raw(sourceFile.readAll());
    sourceFile.close();

    Bencode *bencode = Bencode::fromRaw(raw);

    QVariant json = bencode->toJson();
    delete bencode;
    if (!json.isValid()) {
        qDebug("Error: can't parse bencode format");
        return false;
    }

    QString str = JsonConverter::stringify(json);

    QFile destFile(dest);
    if (!destFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug("Error: can't open destination file");
        return false;
    }

    destFile.write(str.toUtf8());
    destFile.close();
    return true;
}

bool fromJson(const QString &source, const QString &dest)
{
    QFile sourceFile(source);
    if (!sourceFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        openWinConsole();
        qDebug("Error: can't open source file");
        return false;
    }

    QString str = QString::fromUtf8(sourceFile.readAll());
    sourceFile.close();

    QVariant variant = JsonConverter::parse(str);

    if (!variant.isValid()) {
        qDebug("Error: can't parse json format");
        return false;
    }

    Bencode *bencode = Bencode::fromJson(variant);

    QFile destFile(dest);
    if (!destFile.open(QIODevice::WriteOnly)) {
        qDebug("Error: can't open destination file");
        return false;
    }
    destFile.write(bencode->toRaw());
    destFile.close();
    delete bencode;
    return true;
}

int main(int argc, char *argv[])
{
    if (argc == 2 && !strcmp(argv[1], "--help")) {
        openWinConsole();
        printf("Usage: torrent-file-editor --to-json | --from-json  source dest\n");
        closeWinConsole();
        return 0;
    }

#ifdef ENABLE_NVWA
    NVWA::new_progname = argv[0];
    NVWA::new_autocheck_flag = false;
#endif

    if (argc == 4) { // -V112 PVS-Studio
        QString command = QString::fromUtf8(argv[1]);
#ifndef Q_OS_WIN
        QString source = QString::fromUtf8(argv[2]);
        QString dest = QString::fromUtf8(argv[3]);
#else
        QString source = QString::fromLocal8Bit(argv[2]);
        QString dest = QString::fromLocal8Bit(argv[3]);
#endif

        if (command == QLatin1String("--to-json") || command == QLatin1String("--from-json")) {
            int retCode = 0;
            openWinConsole();

            if (!QFile::exists(source)) {
                qDebug("Error: source file is not exist!");
                retCode = -1;
            }
            else if (command == QLatin1String("--to-json"))
                retCode = toJson(source, dest) ? 0 : -1;
            else
                retCode = fromJson(source, dest) ? 0 : -1;

            closeWinConsole();
            return retCode;
        }
    }

#ifdef Q_OS_WIN
# if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    qInstallMessageHandler(winDebugHandler);
# else
    qInstallMsgHandler(winDebugHandler);
# endif
#endif

    int returnCode = 0;
    // For nvwa purposes. Need to delete local objects before leaks checking.
    {
    Application a(argc, argv);

    MainWindow w;
    a.setMainWindow(&w);
    w.show();

#ifdef Q_OS_MAC
    CocoaInitializer initializer;
    SparkleAutoUpdater *updater = new SparkleAutoUpdater;
    updater->checkForUpdates();
#endif

    if (argc == 2) {
        QString filename = QString::fromLocal8Bit(argv[1]);
        if (QFile::exists(filename))
            w.open(filename);
    }

    returnCode = a.exec();
    }

#ifdef ENABLE_NVWA
    NVWA::check_leaks_summary();
#endif

    return returnCode;
}
