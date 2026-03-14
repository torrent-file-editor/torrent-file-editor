// SPDX-FileCopyrightText: 2023-2024, 2026 Ivan Romanov <drizt72@zoho.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "jsonconverter.h"

#include "json-builder.h"
#include "json-parser.h"

#include <QChar>
#include <QMetaType>

QByteArray JsonConverter::qStringToWtf8(const QString &string)
{
    QByteArray res;
    res.reserve(string.size() * 3); // Worst case

    const ushort *data = string.utf16();
    int length = string.length();

    for (int i = 0; i < length; ++i) {
        uint32_t p = data[i];

        // convert surrogate 16-bit code unit pair to supplementary code point
        if (p >= 0xD800 && p <= 0xDBFF && (i + 1) < length) {
            uint16_t next = data[i + 1];
            if (next >= 0xDC00 && next <= 0xDFFF) {
                p = 0x10000 + ((p - 0xD800) << 10) + (next - 0xDC00);
                i++;
            }
        }

        if (p <= 0x007F) {
            res.append(static_cast<char>(p));
        } else if (p <= 0x07FF) {
            res.append(static_cast<char>(0xC0 | (p >> 6)));
            res.append(static_cast<char>(0x80 | (p & 0x3F)));
        } else if (p <= 0xFFFF) {
            res.append(static_cast<char>(0xE0 | (p >> 12)));
            res.append(static_cast<char>(0x80 | ((p >> 6) & 0x3F)));
            res.append(static_cast<char>(0x80 | (p & 0x3F)));
        } else {
            res.append(static_cast<char>(0xF0 | (p >> 18)));
            res.append(static_cast<char>(0x80 | ((p >> 12) & 0x3F)));
            res.append(static_cast<char>(0x80 | ((p >> 6) & 0x3F)));
            res.append(static_cast<char>(0x80 | (p & 0x3F)));
        }
    }

    return res;
}

QString JsonConverter::wtf8toQString(const QByteArray &ba)
{
    QString res;
    res.reserve(ba.size());

    const int len = ba.size();
    QChar replacement(0xFFFD);

    for (int i = 0; i < len; ++i) {
        uint8_t b1 = static_cast<uint8_t>(ba[i]);

        // 1-byte ASCII
        if (b1 <= 0x7F) {
            res.append(QChar(b1, 0));
            continue;
        }

        // 2-byte: 110xxxxx 10xxxxxx
        if (b1 >= 0xC2 && b1 <= 0xDF) {
            if (i + 1 >= len) {
                res.append(replacement);
                continue;
            }

            uint8_t b2 = ba[i + 1];
            if ((b2 & 0xC0) != 0x80) {
                res.append(replacement);
                continue;
            }

            uint32_t cp = ((b1 & 0x1F) << 6) | (b2 & 0x3F);
            res.append(QChar(cp));
            i++;
            continue;
        }

        // 3-byte: 1110xxxx 10xxxxxx 10xxxxxx
        if (b1 >= 0xE0 && b1 <= 0xEF) {
            if (i + 2 >= len) {
                res.append(replacement);
                continue;
            }

            uint8_t b2 = ba[i + 1];
            uint8_t b3 = ba[i + 2];

            if ((b2 & 0xC0) != 0x80 || (b3 & 0xC0) != 0x80) {
                res.append(replacement);
                continue;
            }

            // Overlong protection
            if (b1 == 0xE0 && b2 < 0xA0) {
                res.append(replacement);
                continue;
            }

            uint32_t cp = ((b1 & 0x0F) << 12) | ((b2 & 0x3F) << 6) | (b3 & 0x3F);

            // Allow surrogates for WTF-8
            res.append(QChar(uint16_t(cp)));
            i += 2;
            continue;
        }

        // 4-byte: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        if (b1 >= 0xF0 && b1 <= 0xF4) {
            if (i + 3 >= len) {
                res.append(replacement);
                continue;
            }

            uint8_t b2 = ba[i + 1];
            uint8_t b3 = ba[i + 2];
            uint8_t b4 = ba[i + 3];

            if ((b2 & 0xC0) != 0x80 || (b3 & 0xC0) != 0x80 || (b4 & 0xC0) != 0x80) {
                res.append(replacement);
                continue;
            }

            // Overlong & range checks
            if ((b1 == 0xF0 && b2 < 0x90) || (b1 == 0xF4 && b2 >= 0x90)) {
                res.append(replacement);
                continue;
            }

            uint32_t cp = ((b1 & 0x07) << 18) | ((b2 & 0x3F) << 12) | ((b3 & 0x3F) << 6) | (b4 & 0x3F);
            if (cp > 0x10FFFF) {
                res.append(replacement);
                continue;
            }

            cp -= 0x10000;
            uint16_t high = 0xD800 + (cp >> 10);
            uint16_t low = 0xDC00 + (cp & 0x3FF);

            res.append(QChar(high));
            res.append(QChar(low));
            i += 3;
            continue;
        }

        // Anything else is invalid
        res.append(replacement);
    }

    return res;
}

static QVariant jsonToVariant(json_value *jsonValue)
{
    QVariant res;
    switch (jsonValue->type) {
    case json_object: {
        QVariantMap map;
        for (int i = 0; i < static_cast<int>(jsonValue->u.object.length); ++i) {
            json_char *name = jsonValue->u.object.values[i].name;
            int nameLen = static_cast<int>(jsonValue->u.object.values[i].name_length);
            QString key = JsonConverter::wtf8toQString(QByteArray::fromRawData(name, nameLen));
            QVariant val = jsonToVariant(jsonValue->u.object.values[i].value);
            map[key] = val;
        }
        res = map;
        break;
    }

    case json_array: {
        QVariantList list;
        for (int i = 0; i < static_cast<int>(jsonValue->u.array.length); ++i) {
            list.append(jsonToVariant(jsonValue->u.array.values[i]));
        }
        res = list;
        break;
    }

    case json_integer:
        res = QVariant::fromValue<qulonglong>(jsonValue->u.integer);
        break;

    case json_double:
        res = QVariant::fromValue<double>(jsonValue->u.dbl);
        break;

    case json_boolean:
        res = jsonValue->u.boolean;
        break;

    case json_string:
        res = JsonConverter::wtf8toQString(
            QByteArray::fromRawData(jsonValue->u.string.ptr, static_cast<int>(jsonValue->u.string.length)));
        break;

    default:
        res = QVariant();
        break;
    }

    return res;
}

static json_value *variantToJson(const QVariant &variantValue)
{
    json_value *res;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    switch (static_cast<QMetaType::Type>(variantValue.typeId())) {
#else
    switch (static_cast<QMetaType::Type>(variantValue.type())) {
#endif
    case QMetaType::QVariantMap: {
        QVariantMap map = variantValue.toMap();
        res = json_object_new(map.size());
        for (auto it = map.begin(); it != map.end(); ++it) {
            json_value *child = variantToJson(it.value());
            QByteArray key = JsonConverter::qStringToWtf8(it.key());
            json_object_push_length(res, key.length(), key.constData(), child);
        }
        break;
    }

    case QMetaType::QVariantList: {
        QVariantList list = variantValue.toList();
        res = json_array_new(list.length());
        for (const QVariant &item : list) {
            json_array_push(res, variantToJson(item));
        }
        break;
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    case QMetaType::Short:
    case QMetaType::UShort:
    case QMetaType::Long:
    case QMetaType::ULong:
#endif
    case QMetaType::Int:
    case QMetaType::UInt:
    case QMetaType::LongLong:
        res = json_integer_new(static_cast<json_int_t>(variantValue.toLongLong()));
        break;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    case QMetaType::Float:
#endif
    case QMetaType::Double:
        res = json_double_new(variantValue.toDouble());
        break;

    case QMetaType::Bool:
        res = json_boolean_new(variantValue.toBool());
        break;

    case QMetaType::QString: {
        QByteArray wtf8 = JsonConverter::qStringToWtf8(variantValue.toString());
        res = json_string_new_length(wtf8.length(), wtf8.constData());
        break;
    }

    default:
        res = nullptr;
        break;
    }

    return res;
}

static int lineToPos(const QString &text, int line)
{
    if (line < 0) {
        return -1;
    }

    int pos = 0;

    for (int i = 0; i < line; ++i) {
        pos = text.indexOf(QLatin1Char('\n'), pos);
        if (pos == -1) {
            return -1; // line does not exist
        }
        pos++; // move after '\n'
    }

    return pos;
}

QVariant JsonConverter::parse(const QString &str, int *byte, QString *error)
{
    json_settings settings;
    memset(&settings, 0, sizeof(settings));
    settings.settings |= json_enable_comments;
    QByteArray ba = str.toUtf8();

    QVariant res;
    char errorBuf[json_error_max] = "";
    json_value *json = json_parse_ex(&settings, ba.constData(), ba.length(), errorBuf);
    if (json) {
        res = jsonToVariant(json);
        json_value_free(json);
    }

    QString errorStr = QString::fromUtf8(errorBuf);
    int errorLine = errorStr.section(QLatin1Char(':'), 0, 0).toInt() - 1;

    if (error) {
        *error = errorStr;
    }

    if (byte) {
        *byte = lineToPos(str, errorLine);
    }

    return res;
}

QString JsonConverter::stringify(const QVariant &variant)
{
    json_value *value = variantToJson(variant);

    json_serialize_opts opts;
    opts.mode = json_serialize_mode_multiline;
    opts.opts = 0;
    opts.indent_size = 2;

    char *buf = new char[json_measure_ex(value, opts)];

    json_serialize_ex(buf, value, opts);

    QString res = QString::fromUtf8(buf);

    delete[] buf;
    json_builder_free(value);

    return res;
}
