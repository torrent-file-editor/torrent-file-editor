/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 *
 * Copyright (C) 2015  Ivan Romanov <drizt72@zoho.eu>
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

#include "bencodemodel.h"
#include "bencode.h"

#include <QTextCodec>
#include <QDateTime>
#include <QCryptographicHash>
#include <QStringList>
#include <QDebug>
#include <QUrl>

BencodeModel::BencodeModel(QObject *parent)
    : AbstractTreeModel(new Bencode(Bencode::Type::Dictionary), parent)
    , _bencode(new Bencode(Bencode::Type::Dictionary, "root"))
    , _originBencode(new Bencode(Bencode::Type::Dictionary, "root"))
    , _textCodec(QTextCodec::codecForName("UTF-8"))
{
    root()->appendChild(_bencode);
}

BencodeModel::~BencodeModel()
{
    delete _bencode;
    delete _originBencode;
}

void BencodeModel::setJson(const QVariant &json)
{
    Bencode *newBencode = Bencode::fromJson(json);
    if (newBencode && newBencode->compare(_bencode)) {
        delete newBencode;
        return;
    }

    removeRows(0, rowCount());
    _bencode = newBencode;
    if (!_bencode)
        _bencode = new Bencode(Bencode::Type::Dictionary);
    _bencode->setKey("root");

    beginInsertRows(QModelIndex(), 0, 0);
    root()->appendChild(_bencode);
    endInsertRows();
}

QVariant BencodeModel::toJson() const
{
    return _bencode ? _bencode->toJson() : QVariant();
}

void BencodeModel::setRaw(const QByteArray &raw)
{
    Bencode *newBencode = Bencode::fromRaw(raw);
    if (newBencode && newBencode->compare(_bencode)) {
        delete newBencode;
        return;
    }

    removeRows(0, rowCount());
    _bencode = newBencode;
    if (!_bencode)
        _bencode = new Bencode(Bencode::Type::Dictionary);
    _bencode->setKey("root");

    beginInsertRows(QModelIndex(), 0, 0);
    root()->appendChild(_bencode);
    endInsertRows();
}

QByteArray BencodeModel::toRaw() const
{
    return _bencode->toRaw();
}

bool BencodeModel::isValid() const
{
    return _bencode && _bencode->isValid();
}

void BencodeModel::resetModified()
{
    delete _originBencode;
    _originBencode = _bencode ? _bencode->clone() : nullptr;
}

bool BencodeModel::isModified() const
{
    if (!_bencode && !_originBencode)
        return false;

    if (!_bencode ^ !_originBencode)
        return true;

    return _bencode ? !_bencode->compare(_originBencode) : false;
}

void BencodeModel::setTextCodec(QTextCodec *textCodec)
{
    _textCodec = textCodec;
    emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}

QTextCodec *BencodeModel::textCodec() const
{
    return _textCodec;
}

void BencodeModel::setName(const QString &name)
{
    if (name.isEmpty() && _bencode && _bencode->child("info") && _bencode->child("info")->child("name")) { // -V807 PVS-Studio
        removeRow(_bencode->child("info")->child("name")->row(), nodeToIndex(_bencode->child("info")));
        if (!_bencode->child("info")->childCount()) {
            removeRow(_bencode->child("info")->row(), nodeToIndex(_bencode));
        }
    }
    else if (!name.isEmpty()) {
        emit layoutAboutToBeChanged();
        _bencode->checkAndCreate(Bencode::Type::Dictionary, "info")->checkAndCreate(Bencode::Type::String, "name")->setString(fromUnicode(name));
        emit layoutChanged();
    }
}

QString BencodeModel::name() const
{
    if (_bencode && _bencode->child("info") && _bencode->child("info")->child("name"))
        return toUnicode(_bencode->child("info")->child("name")->string());
    else
        return QString();
}

void BencodeModel::setPrivateTorrent(bool privateTorrent)
{
    if (!privateTorrent && _bencode && _bencode->child("info") && _bencode->child("info")->child("private")) { // -V807 PVS-Studio
        removeRow(_bencode->child("info")->child("private")->row(), nodeToIndex(_bencode->child("info")));
        if (!_bencode->child("info")->childCount()) {
            removeRow(_bencode->child("info")->row(), nodeToIndex(_bencode));
        }
    }
    else if (privateTorrent) {
        emit layoutAboutToBeChanged();
        _bencode->checkAndCreate(Bencode::Type::Dictionary, "info")->checkAndCreate(Bencode::Type::Integer, "private")->setInteger(1);
        emit layoutChanged();
    }
}

bool BencodeModel::privateTorrent() const
{
    if (_bencode && _bencode->child("info") && _bencode->child("info")->child("private"))
        return static_cast<bool>(_bencode->child("info")->child("private")->integer());
    else
        return false;
}

void BencodeModel::setUrl(const QString &url)
{
    if (url.isEmpty() && _bencode && _bencode->child("publisher-url")) {
        removeRow(_bencode->child("publisher-url")->row(), nodeToIndex(_bencode));
    }
    else if (!url.isEmpty()) {
        emit layoutAboutToBeChanged();
        _bencode->checkAndCreate(Bencode::Type::String, "publisher-url")->setString(fromUnicode(url));
        emit layoutChanged();
    }
}

QString BencodeModel::url() const
{
    if (_bencode && _bencode->child("publisher-url"))
        return toUnicode(_bencode->child("publisher-url")->string());
    else
        return QString();
}

void BencodeModel::setPublisher(const QString &publisher)
{
    if (publisher.isEmpty() && _bencode && _bencode->child("publisher")) {
        removeRow(_bencode->child("publisher")->row(), nodeToIndex(_bencode));
    }
    else if (!publisher.isEmpty()) {
        emit layoutAboutToBeChanged();
        _bencode->checkAndCreate(Bencode::Type::String, "publisher")->setString(fromUnicode(publisher));
        emit layoutChanged();
    }
}

QString BencodeModel::publisher() const
{
    if (_bencode && _bencode->child("publisher"))
        return toUnicode(_bencode->child("publisher")->string());
    else
        return QString();
}

void BencodeModel::setCreatedBy(const QString &createdBy)
{
    if (createdBy.isEmpty() && _bencode && _bencode->child("created by")) {
        removeRow(_bencode->child("created by")->row(), nodeToIndex(_bencode));
    }
    else if (!createdBy.isEmpty()) {
        emit layoutAboutToBeChanged();
        _bencode->checkAndCreate(Bencode::Type::String, "created by")->setString(fromUnicode(createdBy));
        emit layoutChanged();
    }
}

QString BencodeModel::createdBy() const
{
    if (_bencode && _bencode->child("created by"))
        return toUnicode(_bencode->child("created by")->string());
    else
        return QString();
}

void BencodeModel::setCreationTime(const QDateTime &creationTime)
{
    if (!creationTime.isValid() && _bencode && _bencode->child("creation date")) {
        removeRow(_bencode->child("creation date")->row(), nodeToIndex(_bencode));
    }
    else if (creationTime.isValid()) {
        emit layoutAboutToBeChanged();
        _bencode->checkAndCreate(Bencode::Type::Integer, "creation date")->setInteger(static_cast<qlonglong>(creationTime.toMSecsSinceEpoch() / 1000));
        emit layoutChanged();
    }
}

QDateTime BencodeModel::creationTime() const
{
    QDateTime dateTime;
    if (_bencode && _bencode->child("creation date"))
        dateTime = QDateTime::fromMSecsSinceEpoch(_bencode->child("creation date")->integer() * 1000);
    return dateTime;
}

void BencodeModel::setPieceSize(int pieceSize)
{
    if (!pieceSize && _bencode && _bencode->child("info") && _bencode->child("info")->child("piece length")) { // -V807 PVS-Studio
        removeRow(_bencode->child("info")->child("piece length")->row(), nodeToIndex(_bencode->child("info")));
        if (!_bencode->child("info")->childCount()) {
            removeRow(_bencode->child("info")->row(), nodeToIndex(_bencode));
        }
    }
    else if (pieceSize) {
        emit layoutAboutToBeChanged();
        _bencode->checkAndCreate(Bencode::Type::Dictionary, "info")->checkAndCreate(Bencode::Type::Integer, "piece length")->setInteger(pieceSize);
        emit layoutChanged();
    }
}

int BencodeModel::pieceSize() const
{
    if (_bencode && _bencode->child("info") && _bencode->child("info")->child("piece length"))
        return _bencode->child("info")->child("piece length")->integer();
    else
        return 0;
}

int BencodeModel::pieces() const
{
    if (_bencode && _bencode->child("info") && _bencode->child("info")->child("pieces"))
        return _bencode->child("info")->child("pieces")->string().size() / 20;
    else
        return 0;
}

QString BencodeModel::hash() const
{
    QByteArray hash;
    if (_bencode && _bencode->child("info"))
        hash = QCryptographicHash::hash(_bencode->child("info")->toRaw(), QCryptographicHash::Sha1).toHex();
    return QString::fromUtf8(hash);
}

QString BencodeModel::magnetLink() const
{
    QByteArray link;

    if (!hash().isEmpty()) {
        link = "magnet:?xt=urn:btih:" + hash().toUtf8();
        if (!name().isEmpty()) {
            link += "&dn=" + QUrl::toPercentEncoding(name());
        }

        for (const QString &tracker: trackers()) {
            link += "&tr=" + QUrl::toPercentEncoding(tracker);
        }
    }

    return QString::fromUtf8(link);
}

void BencodeModel::setComment(const QString &comment)
{
    if (comment.isEmpty() && _bencode && _bencode->child("comment")) {
        removeRow(_bencode->child("comment")->row(), nodeToIndex(_bencode));
    }
    else if (!comment.isEmpty()) {
        emit layoutAboutToBeChanged();
        _bencode->checkAndCreate(Bencode::Type::String, "comment")->setString(fromUnicode(comment));
        emit layoutChanged();
    }
}

QString BencodeModel::comment() const
{
    if (_bencode && _bencode->child("comment"))
        return toUnicode(_bencode->child("comment")->string());
    else
        return QString();
}

void BencodeModel::setTrackers(const QStringList &trackers)
{
    if (_bencode->child("announce-list"))
        removeRow(_bencode->child("announce-list")->row(), nodeToIndex(_bencode));

    emit layoutAboutToBeChanged();
    _bencode->appendMapItem(new Bencode(Bencode::Type::List, "announce-list"));

    for (const QString &tracker: trackers) {
        if (tracker.trimmed().isEmpty())
            continue;

        Bencode *item = new Bencode(fromUnicode(tracker));
        Bencode *parentItem = new Bencode(Bencode::Type::List);
        parentItem->appendChild(item);
        _bencode->child("announce-list")->appendChild(parentItem); // -V595 // -V807 PVS-Studio
    }

    if (_bencode->child("announce-list") && !_bencode->child("announce-list")->children().isEmpty()) {
        _bencode->checkAndCreate(Bencode::Type::String, "announce")->setString(_bencode->child("announce-list")->child(0)->child(0)->string());
    }
    else {
        delete _bencode->child("announce-list");
        delete _bencode->child("announce");
    }
    emit layoutChanged();
}

QStringList BencodeModel::trackers() const
{
    QStringList trackers;
    Bencode *list = _bencode ? _bencode->child("announce-list") : nullptr;

    if (list) {
        for (int i = 0; i < list->childCount(); i++) {
            Bencode *bencode = list->child(i);
            if (bencode->isList() && bencode->childCount() == 1 && bencode->child(0)->isString())
                trackers << toUnicode(bencode->child(0)->string());
        }
    }

    if (trackers.isEmpty()) {
        if (_bencode->child("announce") &&  _bencode->child("announce")->isString())
            trackers << toUnicode(_bencode->child("announce")->string());
    }

    return trackers;
}

void BencodeModel::setFiles(const QList<QPair<QString, qlonglong>> &files)
{
    emit layoutAboutToBeChanged();

    if (files.size() == 1) {
        qlonglong totalSize = files.first().second;
        _bencode->child("info")->checkAndCreate(Bencode::Type::Integer, "length")->setInteger(totalSize);
    }
    else {
        delete _bencode->child("info")->child("files");
        _bencode->child("info")->appendMapItem(new Bencode(Bencode::Type::List, "files"));
        for (const auto &filePair: files) {
            QString file = filePair.first;
            qlonglong size = filePair.second;

            Bencode *fileItem = new Bencode(Bencode::Type::Dictionary);
            fileItem->appendMapItem(new Bencode(size, "length"));

            QStringList pathList = file.split(QStringLiteral("/"));
            fileItem->appendMapItem(new Bencode(Bencode::Type::List, "path"));
            for (const QString &path: pathList) {
                fileItem->child("path")->appendChild(new Bencode(fromUnicode(path)));
            }
            _bencode->child("info")->child("files")->appendChild(fileItem);
        }
    }

    emit layoutChanged();
}

QList<QPair<QString, qlonglong>> BencodeModel::files() const
{
    QList<QPair<QString, qlonglong>> res;

    Bencode *info = _bencode->child("info");
    if (!info)
        return res;

    // Torrent contains only one file
    if (!info->child("files")) {
        QString baseName;
        if (info->child("name") && info->child("name")->isString()) {
            baseName = toUnicode(info->child("name")->string());
            qlonglong length = 0;
            if (info->child("length") && info->child("length")->isInteger()) {
                length = info->child("length")->integer();
            }
            res << QPair<QString, qlonglong>(baseName, length);
        }
    }
    else {
        Bencode *list = info->child("files");
        if (!list)
            return res;

        for (int i = 0; i < list->childCount(); i++) {
            Bencode *item = list->child(i);
            QStringList path;
            Bencode *pathList = item->child("path");
            if (!pathList)
                continue;

            for (int i = 0; i < pathList->childCount(); i++) {
                path << toUnicode(pathList->child(i)->string());
            }

            res << QPair<QString, qlonglong>(path.join(QStringLiteral("/")), item->child("length")->integer());
        }
    }

    return res;
}

qulonglong BencodeModel::totalSize() const
{
    qulonglong res = 0;

    Bencode *info = _bencode->child("info");
    if (!info) {
        return res;
    }

    // Torrent contains only one file
    if (!info->child("files")) {
        QString baseName;
        if (info->child("name") && info->child("name")->isString()) {
            qlonglong length = 0;
            if (info->child("length") && info->child("length")->isInteger()) {
                length = info->child("length")->integer();
            }
            res += length;
        }
    }
    else {
        Bencode *list = info->child("files");
        if (!list) {
            return res;
        }

        for (int i = 0; i < list->childCount(); i++) {
            Bencode *item = list->child(i);
            QStringList path;
            Bencode *pathList = item->child("path");
            if (!pathList) {
                continue;
            }

            res += item->child("length")->integer();
        }
    }

    return res;
}

void BencodeModel::setPieces(const QByteArray &pieces)
{
    if (!pieces.isEmpty()) {
        emit layoutAboutToBeChanged();
        if (!_bencode->child("info"))
            _bencode->appendMapItem(new Bencode(Bencode::Type::Dictionary, "info"));

        if (!_bencode->child("info")->child("pieces"))
            _bencode->child("info")->appendMapItem(new Bencode("", "pieces"));

        _bencode->child("info")->child("pieces")->setString(pieces);
        _bencode->child("info")->child("pieces")->setHex(true);
        emit layoutChanged();
    }
    else {
        removeRows(0, rowCount());
        _bencode = new Bencode(Bencode::Type::Dictionary, "root");
        beginInsertRows(QModelIndex(), 0, 0);
        root()->appendChild(_bencode);
        endInsertRows();
    }
}

void BencodeModel::up(const QModelIndex &index)
{
    Bencode *item = indexToNode(index);
    if (!item || root() == item)
        return;

    if (index.row() == 0)
        return;

    if (!item->parent()->isList())
        return;

    beginMoveRows(index.parent(), index.row(), index.row(), index.parent(), index.row() - 1);
    item->setRow(index.row() - 1);
    endMoveRows();
}

void BencodeModel::down(const QModelIndex &index)
{
    Bencode *item = indexToNode(index);
    if (!item || root() == item)
        return;

    if (index.row() + 1 == rowCount(index.parent()))
        return;

    if (!item->parent()->isList())
        return;

    beginMoveRows(index.parent(), index.row(), index.row(), index.parent(), index.row() + 2);
    item->setRow(index.row() + 1);
    endMoveRows();
}

void BencodeModel::appendRow(const QModelIndex &parent)
{
    Bencode *parentItem = indexToNode(parent);
    if (!parentItem)
        return;
    QModelIndex parentIndex = parent;
    if (!parentItem->isList() && !parentItem->isDictionary()) {
        parentItem = parentItem->parent();

#ifndef __clang__
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wdeprecated-copy"
#endif
        // In Qt4 QModelIdex has user-defined copy constructor but hasn't user-defined assignment
        parentIndex = parent.parent();
#ifndef __clang__
# pragma GCC diagnostic pop
#endif
    }

    if (parentItem->isList()) {
        insertRow(rowCount(parentIndex), parentIndex);
    }
    else if (parentItem->isDictionary()) {
        insertRow(0, parentIndex);
    }
}

void BencodeModel::changeType(const QModelIndex &index, Bencode::Type type)
{
    Bencode *bencode = indexToNode(index);
    if (!bencode || bencode->type() == type)
        return;

    emit layoutAboutToBeChanged();
    bencode->setType(type);
    emit layoutChanged();
}

int BencodeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return static_cast<int>(Column::Count);
}

bool BencodeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    if (role != Qt::EditRole && role != Qt::CheckStateRole && role < Qt::UserRole)
        return false;

    Bencode *item = indexToNode(index);
    Column column = static_cast<Column>(index.column());

    bool res = false;

    switch (role) {
    case Qt::EditRole:
        if (column == Column::Name) {
            QByteArray newKey = fromUnicode(value.toString());
            Bencode *parentItem = item->parent();
            int newRow;
            for (newRow = 0; newRow < parentItem->childCount(); newRow++) {

                if (newKey < parentItem->child(newRow)->key()) {
                    break;
                }
            }

            // Fixed new item pos when going to right
            int realRow = newRow > item->row() ? newRow - 1 : newRow;

            item->setKey(newKey);
            res = true;

            if (realRow == item->row()) {
                emit dataChanged(index, index);
            }
            else {
                beginMoveRows(index.parent(), index.row(), index.row(), index.parent(), newRow);
                item->setRow(realRow);
                endMoveRows();
            }
        }
        else if (column == Column::Value) {
            if (item->isInteger()) {
                item->setInteger(value.toLongLong());
            }
            else if (item->isString()) {
                if (item->hex())
                    item->setString(QByteArray::fromHex(value.toByteArray()));
                else
                    item->setString(fromUnicode(value.toString()));
            }
            res = true;
            emit dataChanged(index, index);
        }
        break;

    case Qt::CheckStateRole:
        if (column == Column::Hex) {
            res = true;
            item->setHex(value.toBool());
            emit dataChanged(index, index.sibling(index.row(), static_cast<int>(Column::Value)));
        }

        break;

    case Qt::UserRole:
    case Qt::UserRole + 1:
        if (item->isInteger()) {
            item->setInteger(value.toLongLong());
        }
        else if (item->isString()) {
            if (role == Qt::UserRole) {
                item->setString(fromUnicode(value.toString()));
            }
            else {
                item->setString(QByteArray::fromHex(value.toByteArray()));
            }
        }
        res = true;
        emit dataChanged(index.sibling(index.row(), static_cast<int>(Column::Value)), index.sibling(index.row(), static_cast<int>(Column::Value)));
        break;

    default:
        break;
    }

    return res;
}

QVariant BencodeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    QVariant res;
    Column column = static_cast<Column>(index.column());
    Bencode *item = indexToNode(index);
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (column) {
        case Column::Name:
            if (item->parent()->isDictionary())
                res = toUnicode(item->key());
            else
                res = item->row();
            break;

        case Column::Type:
            if (role == Qt::DisplayRole)
                res = Bencode::typeToStr(item->type());
            else
                res = static_cast<int>(item->type());
            break;

        case Column::Value:
            if (item->isInteger()) {
                res = item->integer();
            }
            else if (item->isString()) {
                if (role == Qt::DisplayRole) {
                    if (item->hex())
                        res = QString::fromUtf8(item->string().toHex()).left(150);
                    else
                        res = toUnicode(item->string()).left(150);
                }
                else {
                    if (item->hex())
                        res = QString::fromUtf8(item->string().toHex());
                    else
                        res = toUnicode(item->string());
                }
            }
            break;

        default:
            break;
        }
    }
    else if (role == Qt::CheckStateRole) {
        if (column == Column::Hex && item->isString()) {
            res = item->hex() ? Qt::Checked : Qt::Unchecked;
        }
    }
    else if (role >= Qt::UserRole) {
        if (item->isInteger()) {
            res = QString::number(item->integer());
        }
        else if (item->isString()) {
            if (role == Qt::ItemDataRole::UserRole) {
                res = toUnicode(item->string());
            }
            else {
                res = QString::fromUtf8(item->string().toHex());
            }
        }

    }

    return res;
}

QVariant BencodeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant res = QVariant();

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        Column column = static_cast<Column>(section);
        switch (column) {
            case Column::Name:  res = QVariant(tr("Name"));   break;
            case Column::Type:  res = QVariant(tr("Type"));   break;
            case Column::Hex:   res = QVariant(tr("Hex"));    break;
            case Column::Value: res = QVariant(tr("Value"));  break;
            default: break;
        }
    }
    return res;
}

bool BencodeModel::insertRows(int row, int count, const QModelIndex &parent)
{
    Bencode *bencodeParent = indexToNode(parent);

    if (row > bencodeParent->childCount())
        return false;

    beginInsertRows(parent, row, row + count - 1);
    for (int i = 0; i < count; i++) {
        bencodeParent->insertChild(row, new Bencode(0));
    }
    endInsertRows();
    return true;
}

bool BencodeModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Bencode *bencodeParent = indexToNode(parent);

    if (bencodeParent->children().size() < row + count)
        return false;

    beginRemoveRows(parent, row, row + count - 1);
    for (int i = 0; i < count; i++) {
        Bencode *item = bencodeParent->child(row);
        if (item == _bencode) {
            _bencode = nullptr;
        }
        delete item;
    }
    endRemoveRows();

    return true;
}

Qt::ItemFlags BencodeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return AbstractTreeModel::flags(index);

    Column column = static_cast<Column>(index.column());
    Bencode *item = indexToNode(index);

    Qt::ItemFlags f = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if (item == _bencode)
        return f;

    switch (column){
    case Column::Name:
        if (item->parent()->isDictionary())
            f |= Qt::ItemIsEditable;
        break;

    case Column::Type:
        f |= Qt::ItemIsEditable;
        break;

    case Column::Hex:
        f |= Qt::ItemIsUserCheckable;
        break;

    case Column::Value:
        if (item->isInteger() || item->isString())
            f |= Qt::ItemIsEditable;
        break;

    default:
        break;
    }
    return f;
}

QString BencodeModel::toUnicode(const QByteArray &encoded) const
{
    return _textCodec->toUnicode(encoded);
}

QByteArray BencodeModel::fromUnicode(const QString &unicode) const
{
    return _textCodec->fromUnicode(unicode);
}
