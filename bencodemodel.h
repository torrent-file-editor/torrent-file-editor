// SPDX-FileCopyrightText: 2015, 2017-2018, 2026 Ivan Romanov <drizt72@zoho.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "abstracttreemodel.h"
#include "bencode.h"

#include <QList>
#include <QPair>
#include <QString>

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
