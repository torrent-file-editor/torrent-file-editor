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

#pragma once

#include <QVariant>
#include <QMap>
#include <QList>

class Bencode;

typedef QList<Bencode> BencodeList;
typedef QMap<QByteArray, Bencode> BencodeMap;

class Bencode
{
public:
    Bencode();
    Bencode(const Bencode &other);

    Bencode(int integer);
    Bencode(const QByteArray &string);
    Bencode(const BencodeList &list);
    Bencode(const BencodeMap &dictionary);

    enum Type {
        Invalid,
        // Not limited, can be signed. I use qlonglong
        Integer,
        // It is not correct. By fact can contains byte arrays.
        // I use QByteArray. When converting to json all non
        // printable symbols and '%' will be replaced with %HH (hex code)
        String,
        List,
        Dictionary };

    Type type;

    qlonglong integer;
    QByteArray string;
    QList<Bencode> list;
    QMap<QByteArray, Bencode> dictionary;

    bool isValid() const;
    bool isInteger() const;
    bool isString() const;
    bool isList() const;
    bool isDictionary() const;

    QByteArray toRaw() const;
    QVariant toJson() const;

    static Bencode fromRaw(const QByteArray &raw);
    static Bencode fromJson(const QVariant &json);

    Bencode &operator=(qlonglong integer);
    Bencode &operator=(const QByteArray &string);
    Bencode &operator=(const QList<Bencode> &list);
    Bencode &operator=(const QMap<QByteArray, Bencode> &dictionary);

    Bencode &operator[](const QByteArray &key);
    Bencode &operator[](int index);

private:
    static Bencode parseItem(const QByteArray &raw, int &pos);

    static Bencode parseInteger(const QByteArray &raw, int &pos);
    static Bencode parseString(const QByteArray &raw, int &pos);
    static Bencode parseList(const QByteArray &raw, int &pos);
    static Bencode parseDictionary(const QByteArray &raw, int &pos);

    static QString fromRawString(const QByteArray &raw);
    static QByteArray toRawString(const QString &string);

    static QByteArray toRaw(const Bencode &bencode);
    static QVariant toJson(const Bencode &bencode);
};

bool operator==(const Bencode &left, const Bencode &right);
bool operator!=(const Bencode &left, const Bencode &right);
