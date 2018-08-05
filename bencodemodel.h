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

#pragma once

#include "abstracttreemodel.h"
#include "bencode.h"

#include <QString>
#include <QList>
#include <QPair>

class QTextCodec;

class BencodeModel : public AbstractTreeModel<Bencode>
{
    Q_OBJECT

public:
    enum class Column
    {
        Name,
        Type,
        Hex,
        Value,
        Count // Trick to count elements in enum
    };

    explicit BencodeModel(QObject *parent = 0);
    ~BencodeModel();

    void setJson(const QVariant &json);
    QVariant toJson() const;

    void setRaw(const QByteArray &raw);
    QByteArray toRaw() const;

    bool isValid() const;
    void resetModified();
    bool isModified() const;

    void setTextCodec(QTextCodec *textCodec);
    QTextCodec *textCodec() const;

    void setName(const QString &name);
    QString name() const;

    void setPrivateTorrent(bool privateTorrent);
    bool privateTorrent() const;

    void setUrl(const QString &url);
    QString url() const;

    void setPublisher(const QString &publisher);
    QString publisher() const;

    void setCreatedBy(const QString &createdBy);
    QString createdBy() const;

    void setCreationTime(const QDateTime &creationTime);
    QDateTime creationTime() const;

    void setPieceSize(int pieceSize);
    int pieceSize() const;

    int pieces() const;
    QString hash() const;
    QString magnetLink() const;

    void setComment(const QString &comment);
    QString comment() const;

    void setTrackers(const QStringList &trackers);
    QStringList trackers() const;

    void setFiles(const QList<QPair<QString, qlonglong>> &files);
    QList<QPair<QString, qlonglong>> files() const;
    qulonglong totalSize() const;

    void setPieces(const QByteArray &pieces);

    void up(const QModelIndex &index);
    void down(const QModelIndex &index);
    void appendRow(const QModelIndex &parent);

    void changeType(const QModelIndex &index, Bencode::Type type);

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    QString toUnicode(const QByteArray &encoded) const;
    QByteArray fromUnicode(const QString &unicode) const;

    // Here saved .torrent file
    Bencode *_bencode;
    Bencode *_originBencode;

    QTextCodec *_textCodec;
};
