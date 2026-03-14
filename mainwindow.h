// SPDX-FileCopyrightText: 2014-2020, 2023, 2026 Ivan Romanov <drizt72@zoho.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QMainWindow>
#include <QModelIndex>
#include <QStandardItem>
#include <QStringList>

class QProgressDialog;
class Bencode;
class BencodeModel;
class SearchDlg;
class QShortcut;
class QTranslator;

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
    void resultReady(const QByteArray &result, const QString &errorString);

private:
    bool _isCanceled;
};

namespace Ui {
class MainWindow;
}

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
    void showTranslations();
    void changeTranslation(int index);

    void create();
    void open(const QString &fileName);
    void open();
    void save();
    void saveAs();
    void showAbout();
    void copyMagnetLink();
    void copyMagnetExtra();

    void openUrl();

    void updateTab(int n);
    void updateBencodeFromRaw();
    void updateBencodeFromSimple();

    void updateBencodeFromComment();
    void updateBencodeFromTrackers();

    void updateEncoding();

    void updateTitle();

    // Files tab
    void makeTorrent();
    void addFile();
    void addFolder();
    void removeFile();
    void upFile();
    void downFile();
    void reloadFiles();
    void updateFiles();
    void setPieces(const QByteArray &pieces, const QString &errorString);
    void updateRawPosition();
    void filterFiles();
    void updateFilesPieces();

    // Tree tab
    void addTreeItem();
    void removeTreeItem();
    void upTreeItem();
    void downTreeItem();
    void showTreeSearchWindow();
    void showTreeReplaceWindow();
    void selectTreeItem(const QModelIndex &index);

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void closeEvent(QCloseEvent *event);
    void changeEvent(QEvent *event);

private:
    enum Tabs
    {
        SimpleTab,
        FilesTab,
        JsonTreeTab,
        RawTab,
        LogTab
    };
    enum class FilesFilters
    {
        NameFilter,
        ExtenstionFilter,
        TemplateFilter,
        RegExpFilter
    };

    void fillCoding();

    bool isModified() const;

    void updateSimple();
    void updateRaw();

    bool saveTo(const QString &fileName);

    qulonglong autoPieceSize() const;
    void updateFilesSize();
    void addFilesRow(const QString &path, qulonglong size);

    QString smartSize(qulonglong size);
    void processEvents();
    bool showNeedSaveFile();

    void retranslateUi();

    Ui::MainWindow *ui;
    QString _fileName;

    BencodeModel *_bencodeModel;

    QProgressDialog *_progressDialog;
    QStringList _formatFilters;
    QString _lastFolder;
    SearchDlg *_searchDlg;
    QString _torrentLastFolder;
    QShortcut *_showTranslations;
    QTranslator *_translator{};
    QTranslator *_translatorQt{};

    static MainWindow *_instance;
};
