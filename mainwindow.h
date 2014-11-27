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

#include "bencode.h"

#include <QMainWindow>
#include <QStandardItem>

class QProgressDialog;
class QTextCodec;

class Worker : public QObject
{
    Q_OBJECT

public:
    Worker();

public slots:
    void doWork(const QStringList &files, int pieceSize);
    void cancel();

signals:
    void progress(int value);
    void resultReady(const QByteArray &result);

private:
    int _isCanceled;
};


namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    static MainWindow *instance();

    void addLog(const QString &log);

signals:
    void needHash(const QStringList &files, int pieceSize);

public slots:
    void create();
    void open(const QString &fileName);
    void open();
    void save();
    void saveAs();
    void showAbout();

    void openUrl();

    void updateTab(int n);
    void updateBencodeFromRaw();
    void updateBencodeFromSimple();

    void updateBencodeFromComment();
    void updateBencodeFromTrackers();

    void updateBencodeFromJsonTree();

    void updateEncoding(const QString &encoding);

    // Files tab
    void makeTorrent();
    void addFile();
    void addFolder();
    void removeFile();
    void upFile();
    void downFile();
    void updateFiles();
    void setPieces(const QByteArray &pieces);
    void updateRawPosition();

    // Tree tab
    void addTreeItem();
    void removeTreeItem();
    void upTreeItem();
    void downTreeItem();

private:
    enum Tabs { SimpleTab, FilesTab, JsonTreeTab, RawTab, LogTab };

    void fillCoding();
    QString toUnicode(const QByteArray &encoded) const;
    QByteArray fromUnicode(const QString &unicode) const;

    bool isModified() const;
    void updateTitle();

    void updateSimple();
    void updateRaw();
    void updateJsonTree();
    void bencodeToStandardItem(QStandardItem *parent, const Bencode &bencode);
    void standardItemToBencode(Bencode &parent, QStandardItem *item);

    void checkAndFixBencode();

    bool saveTo(const QString &fileName);

    void updateFilesSize();

    QString smartSize(qulonglong size);

    Ui::MainWindow *ui;
    QString _fileName;

    //  Here saved .torrent file
    Bencode _bencode;
    Bencode _originBencode;
    QProgressDialog *_progressDialog;

    static MainWindow *_instance;
    QTextCodec *_textCodec;
};
