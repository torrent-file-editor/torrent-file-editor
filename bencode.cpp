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

#include "bencode.h"

#include <QDebug>
#include <QStringList>

QStringList hexKeys
{
    QStringLiteral("pieces"),
    QStringLiteral("originator"),
    QStringLiteral("certificate"),
    QStringLiteral("signature")
};

Bencode::Bencode(Type type, const QByteArray &key)
    : AbstractTreeNode(nullptr)
    , _type(type)
    , _integer(0)
    , _string(QByteArray())
    , _key(key)
    , _hex(false)
{
}

Bencode::Bencode(qlonglong integer, const QByteArray &key)
    : AbstractTreeNode(nullptr)
    , _type(Integer)
    , _integer(integer)
    , _string(QByteArray())
    , _key(key)
    , _hex(false)
{
}

Bencode::Bencode(const QByteArray &string, const QByteArray &key)
    : AbstractTreeNode(nullptr)
    , _type(String)
    , _integer(0)
    , _string(string)
    , _key(key)
    , _hex(false)
{
}

void Bencode::setType(Type type)
{
    if (type == _type)
        return;

    qDeleteAll(children());
    _integer = 0;
    _string = QByteArray();
    _type = type;
}

Bencode *Bencode::checkAndCreate(Type type, int index)
{
    Q_ASSERT(index > childCount());

    Bencode *item = nullptr;
    while (index > childCount()) {
        appendChild(new Bencode(Type::String));
    }

    if (index == childCount()) {
        item = new Bencode;
        appendChild(item);
    }

    if (child(index)->_type != type) {
        delete child(index);
        item = new Bencode;
        insertChild(index, item);
    }
    return item;
}

Bencode *Bencode::checkAndCreate(Type type, const QByteArray &key)
{
    Bencode *item = child(key);
    if (!item || item->_type != type) {
        delete item;
        item = new Bencode(type, key);
        appendMapItem(item);
    }

    return item;
}

void Bencode::appendMapItem(Bencode *item)
{
    Q_ASSERT(!item->_key.isEmpty());
    Q_ASSERT(isDictionary());
    Q_ASSERT(!item->parent());

    if (item->parent())
        item->parent()->removeChild(item);

    for (int i = 0; i < childCount(); i++) {
        if (item->_key < child(i)->_key) {
            insertChild(i, item);
            break;
        }
    }

    if (!item->parent()) {
        appendChild(item);
    }
}

Bencode *Bencode::child(const QByteArray &key) const
{
    Bencode *res = nullptr;
    for (auto *item: children()) {
        if (static_cast<Bencode*>(item)->_key == key) {
            res = static_cast<Bencode*>(item);
            break;
        }
    }
    return res;
}


QByteArray Bencode::toRaw() const
{
    return toRaw(this);
}

QVariant Bencode::toJson() const
{
    return toJson(this);
}

Bencode *Bencode::fromRaw(const QByteArray &raw)
{
    int pos = 0;
    Bencode *res = parseItem(raw, pos);
    return res;
}

Bencode *Bencode::fromJson(const QVariant &json)
{
    Bencode *res = nullptr;

    switch (json.userType()) {
    case QVariant::String:
        res = new Bencode(toRawString(json.toString()));
        break;

    case QVariant::Map: {
        QVariantMap variantMap = json.toMap();
        res = new Bencode(Type::Dictionary);
        QStringList keys = variantMap.keys();

        foreach (const QString &key, keys) {
            Bencode *newItem = fromJson(variantMap.value(key));
            newItem->_key = toRawString(key);
            if (hexKeys.contains(QString(key)))
                newItem->_hex = true;

            int pos = 0;
            while (pos < res->childCount() && newItem->_key > res->child(pos)->key()) {
                pos++;
            }

            res->insertChild(pos, newItem);
        }
        break; }

    case QVariant::List: {
        QVariantList variantList = json.toList();
        res = new Bencode(Type::List);
        for (int i = 0; i < variantList.size(); ++i) {
            res->appendChild(fromJson(variantList.at(i)));
        }
        break; }

    case QVariant::UInt:
    case QVariant::Int:
    case QVariant::ULongLong:
    case QVariant::LongLong:
    case QVariant::Double:
        res = new Bencode(json.toLongLong());
        break;

    default:
#ifdef DEBUG
        qDebug() << "wrong variant" << json.type();
#endif
        break;
    }

    return res;
}

QString Bencode::typeToStr(Type type)
{
    switch (type) {
    case List:        return QObject::tr("list");        break;
    case Dictionary:  return QObject::tr("dictionary");  break;
    case Integer:     return QObject::tr("integer");     break;
    case String:      return QObject::tr("string");      break;
    case Invalid:     return QObject::tr("invalid");     break;
    }

    return QString();
}

bool Bencode::compare(const Bencode *other) const
{
    if (!other)
        return false;

    if (_type != other->_type)
        return false;

    if (parent() && static_cast<Bencode*>(parent())->_type == Type::Dictionary && other->parent() && _key != other->_key)
        return false;

    switch (_type) {
    case Type::String:
        if (_string != other->_string)
            return false;
        break;

    case Type::Integer:
        if (_integer != other->_integer)
            return false;
        break;

    case Type::Dictionary:
    case Type::List:
        if (childCount() != other->childCount())
            return false;

        for (int i = 0; i < childCount(); i++) {
            bool res = child(i)->compare(other->child(i));
            if (!res) {
                return false;
            }
        }
        break;

    default:
        break;
    }

    return true;
}

Bencode *Bencode::clone() const
{
    Bencode *newItem = new Bencode;
    newItem->_type = _type;
    newItem->_integer = _integer;
    newItem->_string = _string;
    newItem->_key = _key;
    newItem->_hex = _hex;

    for (Bencode *child: children()) {
        newItem->appendChild(child->clone());
    }
    return newItem;
}

QString Bencode::toString() const
{
    QString res;
    if (!_key.isEmpty()) {
        res = QStringLiteral("key ") + QString::fromUtf8(_key) + QStringLiteral(" | "); // -V119 PVS-Studio
    }

    switch (_type) {
    case Type::Invalid: res += QLatin1String("invalid"); break;
    case Type::Integer: res += QLatin1String("integer ") + QString::number(_integer); break;
    case Type::String: res += QLatin1String("string ") + fromRawString(_string); break;
    case Type::Dictionary: res += QLatin1String("dictionary"); break;
    case Type::List: res += QLatin1String("list"); break;
    }
    res = res.left(300);
    return res;
}

Bencode *Bencode::parseItem(const QByteArray &raw, int &pos)
{
    // it is ok to parse empty bencode
    if (pos == 0 && raw.isEmpty())
        return nullptr;

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
#ifdef DEBUG
        qDebug() << "item parsing error. " << pos;
#endif
        return new Bencode();
    }
}

Bencode *Bencode::parseInteger(const QByteArray &raw, int &pos)
{
#ifdef DEBUG
    int basePos = pos;
#endif
    pos++;
    int end = raw.indexOf('e', pos);
    if (end == -1) {
#ifdef DEBUG
        qDebug() << "number parsing error. pos" << basePos;
#endif
        return new Bencode;
    }

    // check number
    for (int i = pos; i < end; ++i) {
        if ((raw[i] >= '0' && raw[i] <= '9')
            || (i == pos && raw[i] == '-')) {

            continue;
        }

#ifdef DEBUG
        qDebug() << "number parsing error. pos" << basePos;
#endif
        return new Bencode;
    }

    Bencode *res = new Bencode(QString::fromUtf8(raw.mid(pos, end - pos)).toLongLong());
    pos = end + 1;
#ifdef DEBUG
    qDebug() << "number parsed" << res->_integer << "pos" << basePos << "=>" << pos;
#endif
    return res;
}

Bencode *Bencode::parseString(const QByteArray &raw, int &pos)
{
#ifdef DEBUG
    int basePos = pos;
#endif

    int delimiter = raw.indexOf(':', pos);
    int size = QString::fromUtf8(raw.mid(pos, delimiter - pos)).toInt();

    delimiter++;
    Bencode *res = new Bencode(raw.mid(delimiter, size));
    pos = delimiter + size;

#ifdef DEBUG
    qDebug() << "byte array parsed" << fromRawString(res->_string).mid(0, 100) << "pos" << basePos << "=>" << pos;
#endif

    return res;
}

Bencode *Bencode::parseList(const QByteArray &raw, int &pos)
{
#ifdef DEBUG
    int basePos = pos;
    int i = 0;
#endif
    pos++;
    Bencode *res = new Bencode(Type::List);

    while(raw[pos] != 'e') {
#ifdef DEBUG
        qDebug() << "list parsing" << i++ << "item";
#endif

        Bencode *item = parseItem(raw, pos);
        Q_ASSERT(item);

        // some error happens
        if (!item || !item->isValid()) { // -V560 PVS-Studio
            delete res;
            delete item;
            return new Bencode;
        }
        res->appendChild(item);
    }
    pos++;
#ifdef DEBUG
    qDebug() << "list parsed" << res->children().size() << "pos" << basePos << "=>" << pos;
#endif
    return res;
}

Bencode *Bencode::parseDictionary(const QByteArray &raw, int &pos)
{
#ifdef DEBUG
    int basePos = pos;
#endif
    pos++;

    Bencode *res = new Bencode(Type::Dictionary);

#ifdef DEBUG
    QStringList keys;
#endif

    while(raw[pos] != 'e') {
        Bencode *keyItem = parseString(raw, pos);
        QByteArray key = keyItem->_string;
        delete keyItem;

#ifdef DEBUG
        keys << fromRawString(key);
        qDebug() << "map parsing" << keys.last() << "item";
#endif
        Bencode *value = parseItem(raw, pos);
        Q_ASSERT(value);

        // some error happens
        if (!value || !value->isValid()) { // -V560 PVS-Studio
            delete res;
            delete value;
            return new Bencode();
        }

        value->_key = key;
        if (hexKeys.contains(QString::fromUtf8(key)))
            value->_hex = true;

        res->appendMapItem(value);
    }
    pos++;
#ifdef DEBUG
    qDebug() << "map parsed" << res->children().size() << keys << "pos" << basePos << "=>" << pos;
#endif
    return res;
}

QString Bencode::fromRawString(const QByteArray &raw)
{
    QString res;
    for (int i = 0; i < raw.size(); ++i) {
        QChar c = QLatin1Char(raw[i]);
        // All normal ASCII symbols except '%'
        if (c >= QLatin1Char(' ') && c <= QLatin1Char('~') && c != QLatin1Char('%')) {
            res += QLatin1Char(raw[i]);
        }
        else {
            res += QLatin1Char('%');
            res += QString::fromUtf8(raw.mid(i, 1).toHex());
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
        if (c != QLatin1Char('%')) {
            res += ba[i];
        }
        else {
            res += QByteArray::fromHex(ba.mid(i + 1, 2));
            i += 2;
        }
    }

    return res;
}

QByteArray Bencode::toRaw(const Bencode *bencode)
{
    QByteArray res;
    switch (bencode->_type) {
    case Integer:
        res += 'i';
        res += QString::number(bencode->_integer).toLatin1();
        res += 'e';
#ifdef DEBUG
        qDebug() << "encode number" << bencode->_integer;
#endif
        break;

    case String: {
        QByteArray ba = bencode->_string;
        res += QString::number(ba.size()).toLatin1();
        res += ':';
        res += ba;
#ifdef DEBUG
        qDebug() << "encode byte array size" << ba.size() << fromRawString(ba).mid(0, 100);
#endif
        break; }

    case List: {
        res += 'l';
        QList<Bencode*> list = bencode->children();
        for (int i = 0; i < list.size(); ++i) {
#ifdef DEBUG
            qDebug() << "encoding" << i << "item";
#endif
            res += toRaw(list.at(i));
        }
#ifdef DEBUG
        qDebug() << "encode list size" << list.size();
#endif
        res += 'e';
        break; }

    case Dictionary: {
        res += 'd';
        QList<Bencode*> map = bencode->children();
        QStringList fromRawKeys;
        for (int i = 0; i < map.size(); ++i) {
            QByteArray key = map.at(i)->_key;
            fromRawKeys << fromRawString(key);
#ifdef DEBUG
            qDebug() << "encode" << fromRawString(key) << "item";
#endif
            res += QString::number(key.size()).toLatin1();
            res += ':';
            res += key;

            res += toRaw(map.at(i));
        }
        res += 'e';
#ifdef DEBUG
        qDebug() << "encode map" << fromRawKeys;
#endif
        break; }

    default:
#ifdef DEBUG
        qDebug() << "wrong type" << bencode->_type;
#endif
        break;

    }
    return res;
}

QVariant Bencode::toJson(const Bencode *bencode)
{
    QVariant res = QVariant();

    switch (bencode->_type) {
    case String:
        res = fromRawString(bencode->_string);
        break;

    case Dictionary: {
        QVariantMap map;
        for (const auto *item: bencode->children()) {
            map.insert(fromRawString(static_cast<const Bencode*>(item)->_key), toJson(static_cast<const Bencode*>(item)));
        }
        res = map;
        break; }

    case List: {
        QVariantList list;
        for (const auto *item: bencode->children()) {
            list << toJson(static_cast<const Bencode*>(item));
        }
        res = list;
        break; }

    case Integer:
        res = bencode->_integer;
        break;

    default:
#ifdef DEBUG
        qDebug() << "wrong bencode type" << bencode->_type;
#endif
        break;
    }

    return res;
}
