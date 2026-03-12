// SPDX-FileCopyrightText: 2023-2024, 2026 Ivan Romanov <drizt72@zoho.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

// Order is matter! First QString leads to strange building error on Fedora 31 MinGW
// clang-format off
#include <QVariant>
#include <QString>
// clang-format on

class JsonConverter
{
public:
    static QByteArray qStringToWtf8(const QString &string);
    static QString wtf8toQString(const QByteArray &ba);

    static QVariant parse(const QString &str, int *byte = nullptr, QString *error = nullptr);
    static QString stringify(const QVariant &variant);
};
