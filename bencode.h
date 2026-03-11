// SPDX-FileCopyrightText: 2014-2015, 2017, 2026 Ivan Romanov <drizt72@zoho.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "abstracttreenode.h"

#include <QList>
#include <QMap>
#include <QVariant>

class Bencode : public AbstractTreeNode<Bencode>
{
public:
    enum Type {
        Invalid,
        // Not limited, can be signed. I use qlonglong
        Integer,
        // It is not correct. By fact can contains byte arrays.
        // I use QByteArray. When converting to json all non
        // printable symbols and '%' will be replaced with %HH (hex code)
        String,
        List,
        Dictionary
    };

    Bencode(Type type = Type::Invalid, const QByteArray &key = QByteArray());
    Bencode(qlonglong integer, const QByteArray &key = QByteArray());
    Bencode(const QByteArray &string, const QByteArray &key = QByteArray());

    void setType(Type type);
    inline Type type() const
    {
        return _type;
    }

    inline void setInteger(qlonglong integer)
    {
        _integer = integer;
    }
    inline qlonglong integer() const
    {
        return _integer;
    }

    inline void setString(const QByteArray &string)
    {
        Q_ASSERT(!string.isNull());

        _string = string;
    }
    inline QByteArray string() const
    {
        return _string;
    }

    inline void setKey(const QByteArray &key)
    {
        _key = key;
    }
    inline QByteArray key() const
    {
        return _key;
    }

    inline void setHex(bool hex)
    {
        _hex = hex;
    }
    inline bool hex() const
    {
        return _hex;
    }

    Bencode *checkAndCreate(Type type, int index);
    Bencode *checkAndCreate(Type type, const QByteArray &key);

    void appendMapItem(Bencode *item);

    // Avoid compilation error due the next not overriden child function
    inline Bencode *child(int index) const
    {
        return AbstractTreeNode<Bencode>::child(index);
    }

    Bencode *child(const QByteArray &key) const;

    inline bool isValid() const
    {
        return _type != Invalid;
    }
    inline bool isInteger() const
    {
        return _type == Integer;
    }
    inline bool isString() const
    {
        return _type == String;
    }
    inline bool isList() const
    {
        return _type == List;
    }
    inline bool isDictionary() const
    {
        return _type == Dictionary;
    }

    QByteArray toRaw() const;
    QVariant toJson() const;

    static Bencode *fromRaw(const QByteArray &raw);
    static Bencode *fromJson(const QVariant &json);
    static QString typeToStr(Type type);

    bool compare(const Bencode *other) const;

    // reimplemented
    Bencode *clone() const override;
    QString toString() const override;

private:
    static Bencode *parseItem(const QByteArray &raw, int &pos);

    static Bencode *parseInteger(const QByteArray &raw, int &pos);
    static Bencode *parseString(const QByteArray &raw, int &pos);
    static Bencode *parseList(const QByteArray &raw, int &pos);
    static Bencode *parseDictionary(const QByteArray &raw, int &pos);

    static QString fromRawString(const QByteArray &raw);
    static QByteArray toRawString(const QString &string);

    static QByteArray toRaw(const Bencode *bencode);
    static QVariant toJson(const Bencode *bencode);

    Type _type;

    qlonglong _integer;
    QByteArray _string;
    QByteArray _key;
    bool _hex;
};
