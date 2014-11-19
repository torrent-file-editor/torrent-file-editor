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

#include "bencode.h"

#include <QDebug>
#include <QStringList>

Bencode::Bencode()
    : type(Invalid)
    , integer(0)
    , string(QByteArray())
    , list(QList<Bencode>())
    , dictionary(QMap<QByteArray, Bencode>())
{
}

Bencode::Bencode(const Bencode &other)
    : type(Invalid)
    , integer(0)
    , string(QByteArray())
    , list(QList<Bencode>())
    , dictionary(QMap<QByteArray, Bencode>())
{
    type = other.type;
    switch (type) {
    case Integer:     integer = other.integer;        break;
    case String:      string = other.string;          break;
    case List:        list = other.list;              break;
    case Dictionary:  dictionary = other.dictionary;  break;
    default: break;
    }
}

Bencode::Bencode(int integer_)
    : type(Integer)
    , integer(integer_)
    , string(QByteArray())
    , list(QList<Bencode>())
    , dictionary(QMap<QByteArray, Bencode>())
{
}

Bencode::Bencode(const QByteArray &string_)
    : type(String)
    , integer(0)
    , string(string_)
    , list(QList<Bencode>())
    , dictionary(QMap<QByteArray, Bencode>())
{
}

Bencode::Bencode(const BencodeList &list_)
    : type(List)
    , integer(0)
    , string(QByteArray())
    , list(list_)
    , dictionary(QMap<QByteArray, Bencode>())
{
}

Bencode::Bencode(const BencodeMap &dictionary_)
    : type(Dictionary)
    , integer(0)
    , string(QByteArray())
    , list(QList<Bencode>())
    , dictionary(dictionary_)
{
}

bool Bencode::isValid() const
{
    return type != Invalid;
}

bool Bencode::isInteger() const
{
    return type == Integer;
}

bool Bencode::isString() const
{
    return type == String;
}

bool Bencode::isList() const
{
    return type == List;
}

bool Bencode::isDictionary() const
{
    return type == Dictionary;
}

QByteArray Bencode::toRaw() const
{
    return toRaw(*this);
}

QVariant Bencode::toJson() const
{
    return toJson(*this);
}

Bencode Bencode::fromRaw(const QByteArray &raw)
{
    int pos = 0;
    Bencode res = parseItem(raw, pos);
    return res;
}

Bencode Bencode::fromJson(const QVariant &json)
{
    Bencode res;

    switch (json.type()) {
    case QVariant::String:
        res = toRawString(json.toString());
        break;

    case QVariant::Map: {
        QVariantMap variantMap = json.toMap();
        BencodeMap map;
        QStringList keys = variantMap.keys();
        foreach (const QString &key, keys) {
            map.insert(toRawString(key), fromJson(variantMap[key]));
        }
        res = map;
        break; }

    case QVariant::List: {
        QVariantList variantList = json.toList();
        BencodeList list;
        for (int i = 0; i < variantList.size(); ++i) {
            list.append(fromJson(variantList[i]));
        }
        res = list;
        break; }

    case QVariant::UInt:
    case QVariant::Int:
    case QVariant::ULongLong:
    case QVariant::LongLong:
        res = json.toLongLong();
        break;

    default:
        qDebug() << "wrong variant" << json.type();
        break;
    }

    return res;
}

Bencode &Bencode::operator=(qlonglong integer)
{
    string = QByteArray();
    list.clear();
    dictionary.clear();
    type = Integer;
    this->integer = integer;
    return *this;
}

Bencode &Bencode::operator=(const QByteArray &string)
{
    integer = 0;
    list.clear();
    dictionary.clear();
    type = String;
    this->string = string;
    return *this;
}

Bencode &Bencode::operator=(const QList<Bencode> &list)
{
    integer = 0;
    string = QByteArray();
    dictionary.clear();
    type = List;
    this->list = list;
    return *this;
}

Bencode &Bencode::operator=(const QMap<QByteArray, Bencode> &dictionary)
{
    integer = 0;
    string = QByteArray();
    list.clear();
    type = Dictionary;
    this->dictionary = dictionary;
    return *this;
}

Bencode &Bencode::operator[](const QByteArray &key)
{
    switch (type) {
    case Invalid:
        type = Dictionary;

    default:
        return dictionary[key];
        break;
    }
}

Bencode &Bencode::operator[](int index)
{
    switch (type) {
    case Invalid:
        type = List;

    default:
        return list[index];
        break;
    }
}

Bencode Bencode::parseItem(const QByteArray &raw, int &pos)
{
    // Integer
    if (raw[pos] == 'i') {
        return parseInteger(raw, pos);
    }
    // String
    else if (raw[pos] >= '0' &&  raw[pos] <= '9'){
        return parseString(raw, pos);
    }
    // List
    else if (raw[pos] == 'l') {
        return parseList(raw, pos);
    }
    // Dictionary
    else if (raw[pos] == 'd') {
        return parseDictionary(raw, pos);
    }
    else {
        qDebug() << "item parsing error. " << pos;
        return Bencode();
    }
}

Bencode Bencode::parseInteger(const QByteArray &raw, int &pos)
{
    int basePos = pos;
    pos++;
    int end = raw.indexOf('e', pos);
    if (end == -1) {
        qDebug() << "number parsing error. pos" << basePos;
        return Bencode();
    }

    // check number
    for (int i = pos; i < end; ++i) {
        if ((raw[i] >= '0' && raw[i] <= '9')
            || (i == pos && raw[i] == '-')) {

            continue;
        }

        qDebug() << "number parsing error. pos" << basePos;
        return Bencode();
    }

    Bencode res;
    res.type = Integer;
    res.integer = QString(raw.mid(pos, end - pos)).toLongLong();
    pos = end + 1;
    qDebug() << "number parsed" << res.integer << "pos" << basePos << "=>" << pos;
    return res;
}

Bencode Bencode::parseString(const QByteArray &raw, int &pos)
{
    int stPos = pos;

    int delimiter = raw.indexOf(':', pos);
    int size = QString(raw.mid(pos, delimiter - pos)).toInt();

    delimiter++;
    Bencode res;
    res.type = String;
    res.string = raw.mid(delimiter, size);
    pos = delimiter + size;

    qDebug() << "byte array parsed" << fromRawString(res.string).mid(0, 100) << "pos" << stPos << "=>" << pos;

    return res;
}

Bencode Bencode::parseList(const QByteArray &raw, int &pos)
{
    int basePos = pos;
    pos++;
    Bencode res;
    res.type = List;

    int i = 0;
    while(raw[pos] != 'e') {
        qDebug() << "list parsing" << i << "item";

        Bencode item = parseItem(raw, pos);
        // some error happens
        if (!item.isValid())
            return Bencode();

        res.list << item;
    }
    pos++;
    qDebug() << "list parsed" << res.list.size() << "pos" << basePos << "=>" << pos;
    return res;
}

Bencode Bencode::parseDictionary(const QByteArray &raw, int &pos)
{
    int basePos = pos;
    pos++;

    Bencode res;
    res.type = Dictionary;

    QStringList keys;

    while(raw[pos] != 'e') {
        QByteArray key = parseString(raw, pos).string;
        keys << fromRawString(key);
        qDebug() << "map parsing" << keys.last() << "item";
        Bencode value = parseItem(raw, pos);

        // some error happens
        if (!value.isValid())
            return Bencode();

        res.dictionary.insert(key, value);
    }
    pos++;
    qDebug() << "map parsed" << res.dictionary.size() << keys << "pos" << basePos << "=>" << pos;
    return res;
}

QString Bencode::fromRawString(const QByteArray &raw)
{
    QString res;
    for (int i = 0; i < raw.size(); ++i) {
        QChar c = raw[i];
        if (c.isPrint() && c != '%') {
            res += raw[i];
        }
        else {
            res += '%';
            res += raw.mid(i, 1).toHex();
        }
    }

    return res;
}

QByteArray Bencode::toRawString(const QString &string)
{
    QByteArray ba = string.toLatin1();
    QByteArray res;
    for (int i = 0; i < ba.size(); ++i) {
        QChar c = string[i];
        if (c != '%') {
            res += ba[i];
        }
        else {
            res += QByteArray::fromHex(ba.mid(i + 1, 2));
            i += 2;
        }
    }

    return res;
}

QByteArray Bencode::toRaw(const Bencode &bencode)
{
    QByteArray res;
    switch (bencode.type) {
    case Integer:
        res += 'i';
        res += QString::number(bencode.integer).toLatin1();
        res += 'e';
        qDebug() << "encode number" << bencode.integer;
        break;

    case String: {
        QByteArray ba = bencode.string;
        res += QString::number(ba.size());
        res += ':';
        res += ba;
        qDebug() << "encode byte array size" << ba.size() << fromRawString(ba).mid(0, 100);
        break; }

    case List: {
        res += 'l';
        BencodeList list = bencode.list;
        for (int i = 0; i < list.size(); ++i) {
            qDebug() << "encoding" << i << "item";
            res += toRaw(list[i]);
        }
        qDebug() << "encode list size" << list.size();
        res += 'e';
        break; }

    case Dictionary: {
        res += 'd';
        BencodeMap map = bencode.dictionary;
        QList<QByteArray> keys = map.keys();
        QStringList fromRawKeys;
        foreach (const QByteArray &key, keys) {
            fromRawKeys << fromRawString(key);
            qDebug() << "encode" << fromRawString(key) << "item";
            res += QString::number(key.size());
            res += ':';
            res += key;

            res += toRaw(map[key]);
        }
        res += 'e';
        qDebug() << "encode map" << fromRawKeys;
        break; }

    default:
        qDebug() << "wrong type" << bencode.type;
        break;

    }
    return res;
}

QVariant Bencode::toJson(const Bencode &bencode)
{
    QVariant res = QVariant();

    switch (bencode.type) {
    case String:
        res = fromRawString(bencode.string);
        break;

    case Dictionary: {
        QVariantMap map;
        QList<QByteArray> keys = bencode.dictionary.keys();
        foreach (const QByteArray &key, keys) {
            map.insert(fromRawString(key), toJson(bencode.dictionary[key]));
        }
        res = map;
        break; }

    case List: {
        QVariantList list;
        for (int i = 0; i < bencode.list.size(); ++i) {
            list << toJson(bencode.list[i]);
        }
        res = list;
        break; }

    case Integer:
        res = bencode.integer;
        break;

    default:
        qDebug() << "wrong bencode type" << bencode.type;
        break;
    }

    return res;
}

bool operator==(const Bencode &left, const Bencode &right)
{
    if (left.type != right.type)
        return false;

    bool res = false;
    switch (left.type) {
    case Bencode::Integer:
        res = left.integer == right.integer;
        break;

    case Bencode::String:
        res = left.string == right.string;
        break;

    case Bencode::List:
        res = left.list == right.list;
        break;

    case Bencode::Dictionary:
        res = left.dictionary == right.dictionary;
        break;

    default:
        res = true;
        break;
    }

    return res;
}

bool operator!=(const Bencode &left, const Bencode &right)
{
    return !(left == right);
}
