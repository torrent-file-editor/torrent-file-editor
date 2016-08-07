/*
 * Copyright (C) 2014-2015  Ivan Romanov <drizt@land.ru>
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
#include "application.h"
#include "bencode.h"

#include <QVariant>
#include <QTranslator>
#include <QLocale>
#include <QFile>
#include <QLibraryInfo>

#ifdef HAVE_QT5
# include <QJsonDocument>
#else
# include <qjson/serializer.h>
# include <qjson/parser.h>
#endif

#ifdef Q_OS_WIN
# include <windows.h>
HANDLE hConsole = NULL;
#endif

#ifdef Q_OS_MAC
# include "cocoainitializer.h"
# include "sparkleautoupdater.h"
#endif

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
#ifdef HAVE_QT5
        MainWindow::instance()->addLog(msg);
#else
        MainWindow::instance()->addLog(QLatin1String(msg));
#endif
}

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

#ifdef HAVE_QT5
    QByteArray ba = QJsonDocument::fromVariant(json).toJson();
#else
    QJson::Serializer serializer;
    serializer.setIndentMode(QJson::IndentFull);
    QByteArray ba = serializer.serialize(json);
#endif

    QFile destFile(dest);
    if (!destFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug("Error: can't open destination file");
        return false;
    }

    destFile.write(ba);
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

    QByteArray ba(sourceFile.readAll());
    sourceFile.close();

#ifdef HAVE_QT5
    QJsonParseError error;
    QVariant variant = QJsonDocument::fromJson(ba, &error).toVariant();
#else
    QJson::Parser parser;
    bool ok;
    QVariant variant = parser.parse(ba, &ok);
#endif

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

    if (argc == 4) {
        QString command = QLatin1String(argv[1]);
        QString source = QLatin1String(argv[2]);
        QString dest = QLatin1String(argv[3]);

        if (command == QLatin1String("--to-json") || command == QLatin1String("--from-json")) {
            int retCode = 0;
            openWinConsole();

            if (!QFile::exists(source)) {
                qDebug("Error: source file is not exist!");
                retCode = -1;
            }
            else if (command == QLatin1String("--to-json"))
                retCode = toJson(source, dest) ? -1 : 0;
            else
                retCode = fromJson(source, dest) ? -1 : 0;

            closeWinConsole();
            return retCode;
        }
    }

#ifdef DEBUG
# ifdef HAVE_QT5
    qInstallMessageHandler(debugHandler);
# else
    qInstallMsgHandler(debugHandler);
# endif
#endif

    Application a(argc, argv);

    QTranslator translator;
    if (translator.load(QLocale(), QLatin1String("torrentfileeditor"), QLatin1String("_"), QLatin1String(":/translations")))
        a.installTranslator(&translator);

#ifdef Q_OS_WIN
    QString qtTranslationsPath = QLatin1String(":/translations");
#else
    QString qtTranslationsPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#endif

#ifdef HAVE_QT5
    QString qtTranslationsName = QLatin1String("qtbase");
#else
    QString qtTranslationsName = QLatin1String("qt");
#endif

    QTranslator qtTranslator;
    if (qtTranslator.load(QLocale(), qtTranslationsName, QLatin1String("_"), qtTranslationsPath))
        a.installTranslator(&qtTranslator);

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

    return a.exec();
}
