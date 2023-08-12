#pragma once

#include <QString>
#include <QVariant>

class JsonConverter
{
public:
    static QVariant parse(const QString &str, int *byte = nullptr, QString *error = nullptr);
    static QString stringify(const QVariant &variant);
};

