/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 *
 * Copyright (C) 2017  Ivan Romanov <drizt72@zoho.eu>
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

#include "checkupdate.h"

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
# include <QJsonDocument>
#else
# include <qjson/serializer.h>
# include <qjson/parser.h>
#endif

#include <QVariantMap>

#include <tchar.h>
#include <windows.h>
#include <wininet.h>

#define CHUNK_SIZE 1024 * 1024 /* 1MiB */
#define CAST_URL _T("https://torrent-file-editor.github.io/appcast/appcast.json")

CheckUpdate::CheckUpdate(QObject *parent)
    : QObject(parent)
{
}

void CheckUpdate::start()
{
    HINTERNET hInternetSession;
    HINTERNET hURL;
    char cBuffer[CHUNK_SIZE];
    DWORD dwBytesRead;

    // Make internet connection.
    hInternetSession = InternetOpen(_T("Microsoft Internet Explorer"), // agent
                                    INTERNET_OPEN_TYPE_PRECONFIG,      // access
                                    NULL, NULL, 0);                    // defaults

    // Make connection to desired page.
    hURL = InternetOpenUrl(hInternetSession,                       // session handle
                           CAST_URL,                               // URL to access
                           NULL, -1L, 0, 0);                       // defaults

    // Read page into memory buffer.
    InternetReadFile(hURL,                    // handle to URL
                     (LPSTR)cBuffer,          // pointer to buffer
                     (DWORD)sizeof(cBuffer),  // size of buffer
                     &dwBytesRead);           // pointer to var to hold return value


    InternetCloseHandle(hURL);

    if (dwBytesRead == 0) {
        InternetCloseHandle(hInternetSession);
        emit finished(QString(), QString());
        return;
    }

    if (dwBytesRead < sizeof(cBuffer)) {
        cBuffer[dwBytesRead] = '\0';
    }
    else {
        cBuffer[sizeof(cBuffer) - 1] = '\0';
    }

    QByteArray ba(cBuffer, dwBytesRead);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    QJsonParseError error;
    QVariantMap variant = QJsonDocument::fromJson(ba, &error).toVariant().toMap();
    if (error.error) {
        emit finished(QString(), QString());
        return;
    }
#else
    QJson::Parser parser;
    bool ok;
    QVariantMap variant = parser.parse(ba, &ok).toMap();
    if (!ok) {
        emit finished(QString(), QString());
        return;
    }
#endif

    QString version = variant.value(QLatin1String("version")).toString();
#ifdef Q_OS_WIN64
    QString url = variant.value(QLatin1String("url-win-x64")).toString();
#elif defined Q_OS_WIN32
    QString url = variant.value(QLatin1String("url-win-x32")).toString();
#endif

    emit finished(version, url);
}
