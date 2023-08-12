#include "jsonconverter.h"

#include "json.hpp"

#include <QMetaType>

static QVariant jsonToVariant(const nlohmann::ordered_json &jsonValue)
{
    QVariant res;
    if (jsonValue.is_object()) {
        QVariantMap map;
        for (auto it = jsonValue.begin(); it != jsonValue.end(); ++it) {
            QString key = QString::fromStdString(it.key());
            QVariant val = jsonToVariant(it.value());
            map[key] = val;
        }
        res = map;
    }
    else if (jsonValue.is_array()) {
        QVariantList list;
        for (const auto &item: jsonValue) {
            list.append(jsonToVariant(item));
        }
        res = list;
    }
    else if (jsonValue.is_number_unsigned()) {
        res = jsonValue.get<qulonglong>();
    }
    else if (jsonValue.is_number_integer()) {
        res = jsonValue.get<qlonglong>();
    }
    else if (jsonValue.is_number_float()) {
        res = jsonValue.get<double>();
    }
    else if (jsonValue.is_boolean()) {
        res = jsonValue.get<bool>();
    }
    else if (jsonValue.is_string()) {
        res = QString::fromStdString(jsonValue.get<std::string>());
    }
    else {
        res = QVariant();
    }

    return res;
}

static nlohmann::ordered_json variantToJson(const QVariant &variantValue)
{
    nlohmann::ordered_json res;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    switch (variantValue.typeId()) {
#else
    switch (variantValue.type()) {
#endif
    case QMetaType::QVariantMap: {
        QVariantMap map = variantValue.toMap();
        res = nlohmann::ordered_json::object();
        for (auto it = map.begin(); it != map.end(); ++it) {
            std::string key = it.key().toStdString();
            nlohmann::ordered_json val = variantToJson(it.value());
            res[key] = val;
        }
        break;
    }

    case QMetaType::QVariantList: {
        QVariantList list = variantValue.toList();
        res = nlohmann::ordered_json::array();
        for (const QVariant &item: list) {
            res.push_back(variantToJson(item));
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
        res = variantValue.toLongLong();
        break;

    case QMetaType::ULongLong:
        res = variantValue.toULongLong();
        break;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    case QMetaType::Float:
#endif
    case QMetaType::Double:
        res = variantValue.toDouble();
        break;

    case QMetaType::Bool:
        res = variantValue.toBool();
        break;

    case QMetaType::QString:
        res = variantValue.toString().toStdString();
        break;

    default:
        res = nullptr;
        break;
    }

    return res;
}

QVariant JsonConverter::parse(const QString &str, int *byte, QString *error)
{
    bool allowExceptions = true;
    bool ignoreComments = true;
    nlohmann::ordered_json json;

    try {
        json = nlohmann::ordered_json::parse(str.toStdString(), nullptr, allowExceptions, ignoreComments);
    }
    catch (nlohmann::ordered_json::parse_error &e) {
        if (error) {
            QString str = QString::fromStdString(e.what()).section(QStringLiteral(":"), 1);
            *error = str;
        }

        if (byte) {
            *byte = e.byte;
        }
    }
    catch (nlohmann::ordered_json::exception &e) {
        if (error) {
            *error = QString::fromStdString(e.what());
        }
    }

    return jsonToVariant(json);
}

QString JsonConverter::stringify(const QVariant &variant)
{
    nlohmann::ordered_json json = variantToJson(variant);
    return QString::fromStdString(json.dump(2));
}
