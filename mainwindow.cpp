/*
 * Copyright (C) 2014-2015  Ivan Romanov <drizt@land.ru>
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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "aboutdlg.h"
#include "bencode.h"

#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QDesktopServices>
#include <QIcon>
#include <QUrl>
#include <QLocale>
#include <QApplication>
#include <qmath.h>
#include <QStandardItemModel>
#include <QDirIterator>
#include <QProgressDialog>
#include <QThread>
#include <QTextCodec>
#include <QAbstractItemDelegate>
#include <QPersistentModelIndex>
#include <QInputDialog>

#ifdef HAVE_QT5
# include <QJsonDocument>
#else
# include <qjson/serializer.h>
# include <qjson/parser.h>
#endif

#define APP_NAME "Torrent File Editor 0.1.0"

Worker::Worker()
    : QObject()
    , _isCanceled(false)
{
}

void Worker::doWork(const QStringList &files, int pieceSize)
{
    QByteArray pieceHashes;

    QByteArray piece;
    int piecePos = 0;
    piece.resize(pieceSize);

    QCryptographicHash hash(QCryptographicHash::Sha1);
    qulonglong value = 0;

    for (int i = 0; i < files.size(); ++i) {
        QFile f(files[i]);
        if (!f.open(QIODevice::ReadOnly)) {
            emit resultReady(QByteArray());
            return;
        }

        int readed;
        int j = 0;
        while ((readed = f.read(piece.data() + piecePos, pieceSize - piecePos)) > 0) {
            piecePos += readed;
            j++;
            if (piecePos == pieceSize || ((i == files.size() - 1) && f.atEnd())) {
                piece.resize(piecePos);
                piecePos = 0;
                hash.addData(piece);
                pieceHashes += hash.result();
                hash.reset();
            }
            value += readed;
            emit progress(value / 1024);

            qApp->processEvents();
            if (_isCanceled) {
                emit resultReady(QByteArray());
                f.close();
                return;
            }
        }

        // Some error
        if (readed < 0) {
            emit resultReady(QByteArray());
            f.close();
            return;
        }

        f.close();
    }

    emit resultReady(pieceHashes);
}

void Worker::cancel()
{
    _isCanceled = true;
}

MainWindow *MainWindow::_instance;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , _bencode(new Bencode(Bencode::Type::Dictionary))
    , _originBencode(nullptr)
    , _fileName(QString())
    , _progressDialog(new QProgressDialog(this))
    , _textCodec(QTextCodec::codecForName("UTF-8"))
{
    ui->setupUi(this);

    _progressDialog->setWindowModality(Qt::ApplicationModal);
    _progressDialog->setLabelText(tr("Need to calculate piece hashes"));
    _progressDialog->setWindowTitle(tr("Please wait"));

    updateTitle();

    _instance = this;

    ui->cmbPieceSizes->addItem(tr("Auto"), 0);
    for (int i = 5; i < 16; ++i) {
        qulonglong pieceSize = 1024 * static_cast<qulonglong>(qPow(2, i));
        ui->cmbPieceSizes->addItem(smartSize(pieceSize), pieceSize);
    }

    QStandardItemModel *model = new QStandardItemModel(0, 2, this);
    QStringList headers;
    headers << tr("Path") << tr("Size");
    model->setHorizontalHeaderLabels(headers);
    model->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
    model->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);
    ui->viewFiles->setModel(model);
#ifdef HAVE_QT5
    ui->viewFiles->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->viewFiles->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
#else
    ui->viewFiles->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->viewFiles->verticalHeader()->setResizeMode(QHeaderView::Fixed);
#endif

    model = new QStandardItemModel(0, 3, this);
    headers.clear();
    headers << tr("Name") << tr("Type") << tr("Value");
    model->invisibleRootItem()->setData(Bencode::Dictionary);
    model->setHorizontalHeaderLabels(headers);
    model->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
    model->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);
    model->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignLeft);
    ui->treeJson->setModel(model);
#ifdef HAVE_QT5
    ui->treeJson->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->treeJson->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
#else
    ui->treeJson->header()->setResizeMode(0, QHeaderView::ResizeToContents);
    ui->treeJson->header()->setResizeMode(1, QHeaderView::ResizeToContents);
#endif
    QAbstractItemDelegate *delegate = ui->treeJson->itemDelegate();
    connect(delegate, SIGNAL(closeEditor(QWidget*, QAbstractItemDelegate::EndEditHint)), SLOT(updateBencodeFromJsonTree()));
    connect(model, SIGNAL(itemChanged(QStandardItem*)), SLOT(sortJsonTree(QStandardItem*)));

    ui->btnNew->setIcon(QIcon(":/icons/text-x-generic.png"));
    ui->btnOpen->setIcon(qApp->style()->standardIcon(QStyle::SP_DialogOpenButton));
    ui->btnSave->setIcon(qApp->style()->standardIcon(QStyle::SP_DialogSaveButton));
    ui->btnSaveAs->setIcon(qApp->style()->standardIcon(QStyle::SP_DialogSaveButton));

    ui->btnRemoveFiles->setIcon(QIcon::fromTheme("list-remove", QIcon(":/icons/list-remove.png")));

    ui->btnMakeTorrent->setIcon(QIcon(":/icons/hammer.png"));
    ui->btnAddFile->setIcon(QIcon::fromTheme("document-new", QIcon(":/icons/document-new.png")));
    ui->btnAddFolder->setIcon(QIcon::fromTheme("folder-new", QIcon(":/icons/folder-new.png")));
    ui->btnUpFile->setIcon(qApp->style()->standardIcon(QStyle::SP_ArrowUp));
    ui->btnDownFile->setIcon(qApp->style()->standardIcon(QStyle::SP_ArrowDown));

    ui->btnAbout->setIcon(qApp->style()->standardIcon(QStyle::SP_MessageBoxQuestion));

    ui->btnAddTreeItem->setIcon(QIcon::fromTheme("list-add", QIcon(":/icons/list-add.png")));
    ui->btnRemoveTreeItem->setIcon(QIcon::fromTheme("list-remove", QIcon(":/icons/list-remove.png")));
    ui->btnUpTreeItem->setIcon(qApp->style()->standardIcon(QStyle::SP_ArrowUp));
    ui->btnDownTreeItem->setIcon(qApp->style()->standardIcon(QStyle::SP_ArrowDown));

    fillCoding();
    updateFilesSize();
}

MainWindow::~MainWindow()
{
    _instance = 0;
    delete ui;
    delete _bencode;
    delete _originBencode;
}

MainWindow *MainWindow::instance()
{
    return _instance;
}

void MainWindow::addLog(const QString &log)
{
    // ui->pteLog->appendPlainText(log);
}

void MainWindow::create()
{
    if (isModified()) {
        if (QMessageBox::question(this,
                                  tr("Create a new file"),
                                  tr("Current file is not saved. Save the file?"),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::Yes) == QMessageBox::Yes) {

            save();
        }
    }

    _fileName = "";
    delete _bencode;
    delete _originBencode;
    _originBencode = new Bencode(Bencode::Type::Dictionary);
    _bencode = new Bencode(Bencode::Type::Dictionary);
    updateTitle();
    updateTab(ui->tabWidget->currentIndex());

    // Files tab
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->viewFiles->model());
    model->removeRows(0, model->rowCount());

    ui->leBaseFolder->setText("");
    updateFilesSize();
    ui->cmbPieceSizes->setCurrentIndex(0);
}

void MainWindow::open(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("Error"), tr("Can't open file"));
        return;
    }

    _fileName = fileName;

    delete _originBencode;
    delete _bencode;
    _bencode = Bencode::fromRaw(file.readAll());
    file.close();

    _originBencode = _bencode->clone();

    updateTab(ui->tabWidget->currentIndex());
    updateTitle();

    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->viewFiles->model());
    model->removeRows(0, model->rowCount());
    updateFiles();
}

void MainWindow::open()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open"), "", tr("Torrents (*.torrent)"));

    if (fileName.isEmpty())
        return;

    open(fileName);
}

void MainWindow::save()
{
    if (_fileName.isEmpty())
        saveAs();

    if (saveTo(_fileName))
        updateTitle();
}

void MainWindow::saveAs()
{
    if (!_bencode->isValid())
        return;

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"), "", tr("Torrents (*.torrent)"));
    if (!fileName.endsWith(".torrent"))
        fileName += ".torrent";

    if (fileName.isEmpty())
        return;

    if (saveTo(fileName)) {
        _fileName = fileName;
        updateTitle();
    }
}

void MainWindow::showAbout()
{
    AboutDlg dlg(this);
    dlg.setWindowTitle(QString(tr("About %1")).arg(APP_NAME));
    dlg.ensurePolished();
    dlg.adjustSize();
    dlg.setFixedSize(dlg.size());
    dlg.exec();
}

void MainWindow::openUrl()
{
    QUrl url(ui->leUrl->text());
    if (url.isValid())
        QDesktopServices::openUrl(url);
}

// Token from qmmp
void MainWindow::fillCoding()
{
    QMap<QString, QTextCodec*> codecMap;
    QRegExp iso8859RegExp("ISO[- ]8859-([0-9]+).*");

    for (int mib: QTextCodec::availableMibs())
    {
        QTextCodec *codec = QTextCodec::codecForMib(mib);

        QString sortKey = codec->name().toUpper();
        int rank;

        if (sortKey.startsWith("UTF-8")) {
            rank = 1;
        }
        else if (sortKey.startsWith("UTF-16")) {
            rank = 2;
        }
        else if (iso8859RegExp.exactMatch(sortKey)) {
            if (iso8859RegExp.cap(1).size() == 1)
                rank = 3;
            else
                rank = 4;
        }
        else {
            rank = 5;
        }
        sortKey.prepend(QChar('0' + rank));
        codecMap.insert(sortKey, codec);
    }

    foreach (QTextCodec *textCodec, codecMap.values()) {
        ui->cmbCoding->addItem(textCodec->name());
    }
}

QString MainWindow::toUnicode(const QByteArray &encoded) const
{
    return _textCodec->toUnicode(encoded);
}

QByteArray MainWindow::fromUnicode(const QString &unicode) const
{
    return _textCodec->fromUnicode(unicode);
}

bool MainWindow::isModified() const
{
    Q_ASSERT(_bencode);
    Q_ASSERT(_originBencode);
    Q_ASSERT(_bencode != _originBencode);

    return !_bencode->compare(_originBencode);
}

void MainWindow::updateTitle()
{
    if (_fileName.isEmpty())
        setWindowTitle(APP_NAME);
    else if (isModified())
        setWindowTitle(QString("%1 - %2 *").arg(APP_NAME, _fileName));
    else
        setWindowTitle(QString("%1 - %2").arg(APP_NAME, _fileName));
}

void MainWindow::updateTab(int n)
{
    switch (n) {
    case SimpleTab:
        updateSimple();
        break;

    case JsonTreeTab:
        updateJsonTree();
        break;

    case RawTab:
        updateRaw();
        break;

    default:
        break;
    }
}

void MainWindow::updateBencodeFromSimple()
{
    if (!ui->leUrl->text().isEmpty()) {
        _bencode->checkAndCreate(Bencode::Type::String, "publisher-url")->setString(fromUnicode(ui->leUrl->text()));
    }
    else {
        delete _bencode->child("publisher-url");
    }

    if (!ui->lePublisher->text().isEmpty()) {
        _bencode->checkAndCreate(Bencode::Type::String, "publisher")->setString(fromUnicode(ui->lePublisher->text()));
    }
    else {
        delete _bencode->child("publisher");
    }

    if (!ui->leCreatedBy->text().isEmpty()) {
        _bencode->checkAndCreate(Bencode::Type::String, "created by")->setString(fromUnicode(ui->leCreatedBy->text()));
    }
    else {
        delete _bencode->child("created by");
    }

    if (!ui->leName->text().isEmpty()) {
        _bencode->checkAndCreate(Bencode::Type::Dictionary, "info")->checkAndCreate(Bencode::Type::String, "name")->setString(fromUnicode(ui->leName->text()));
    }
    else if (_bencode->child("info")) {
        delete _bencode->child("info")->child("name");
        if (!_bencode->child("info")->childCount())
            delete _bencode->child("info");
    }

    if (ui->dateCreated->dateTime().isValid()) {
        _bencode->checkAndCreate(Bencode::Type::Integer, "creation date")->setInteger(static_cast<qlonglong>(ui->dateCreated->dateTime().toMSecsSinceEpoch() / 1000));
    }
    else {
        delete _bencode->child("creation date");
    }

    QByteArray hash;
    if (_bencode->child("info") && _bencode->child("info")->childCount())
        hash = QCryptographicHash::hash(_bencode->child("info")->toRaw(), QCryptographicHash::Sha1).toHex();

    ui->leHash->setText(hash);
    updateTitle();
}

void MainWindow::updateBencodeFromComment()
{
    if (!ui->pteComment->toPlainText().isEmpty()) {
        _bencode->checkAndCreate(Bencode::Type::String, "comment")->setString(fromUnicode(ui->pteComment->toPlainText()));
    }
    else {
        delete _bencode->child("comment");
    }

    updateTitle();
}

void MainWindow::updateBencodeFromTrackers()
{
    delete _bencode->child("announce-list");
    _bencode->appendMapItem(new Bencode(Bencode::Type::List, "announce-list"));

    QStringList trackers = ui->pteTrackers->toPlainText().trimmed().split("\n");
    for (const QString &tracker: trackers) {
        if (tracker.trimmed().isEmpty())
            continue;

        Bencode *item = new Bencode(fromUnicode(tracker));
        Bencode *parentItem = new Bencode(Bencode::Type::List);
        parentItem->appendChild(item);
        _bencode->child("announce-list")->appendChild(parentItem);
    }

    if (_bencode->child("announce-list") && !_bencode->child("announce-list")->children().isEmpty()) {
        _bencode->checkAndCreate(Bencode::Type::String, "announce")->setString(_bencode->child("announce-list")->child(0)->child(0)->string());
    }
    else {
        delete _bencode->child("announce-list");
        delete _bencode->child("announce");
    }

    updateTitle();
}

void MainWindow::updateBencodeFromJsonTree()
{
    delete _bencode;
    _bencode = new Bencode(Bencode::Type::Dictionary);
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->treeJson->model());
    if (model->invisibleRootItem()->rowCount()) {
        standardItemToBencode(_bencode, model->invisibleRootItem()->child(0, 0));
    }

    updateTitle();
}

void MainWindow::updateEncoding(const QString &encoding)
{
    _textCodec = QTextCodec::codecForName(encoding.toUtf8());

    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->viewFiles->model());
    model->removeRows(0, model->rowCount());
    updateFiles();

    updateTab(ui->tabWidget->currentIndex());
}

void MainWindow::makeTorrent()
{
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->viewFiles->model());
    if (!model->rowCount())
        return;

    qulonglong totalSize = 0;
    QStringList files;
    QString baseFolder = ui->leBaseFolder->text();
    QDir baseDir(baseFolder);

    for (int i = 0; i < model->rowCount(); ++i) {
        QString file = model->item(i)->text();
        if (QFileInfo(file).isRelative()) {
            QMessageBox::warning(this, tr("Warning"), tr("Can't make torrent for file without full path. Need to set base folder."));
            return;
        }

        if (model->rowCount() > 1 && baseDir.relativeFilePath(file).startsWith("..")) {
            QMessageBox::warning(this, tr("Warning"), tr("Base folder is not common."));
            return;
        }

        // Check for base folder
        files << QDir::fromNativeSeparators(file);
        totalSize += QFileInfo(file).size();
    }

    qulonglong pieceSize = ui->cmbPieceSizes->itemData(ui->cmbPieceSizes->currentIndex()).toULongLong();
    // http://torrentfreak.com/how-to-make-the-best-torrents-081121/
    // Find out optimal piece size
    if (!pieceSize) {
        for (int i = 1; i < ui->cmbPieceSizes->count(); ++i) {
            pieceSize = ui->cmbPieceSizes->itemData(i).toULongLong();
            if (totalSize / pieceSize < 2000)
                break;
        }
    }

    _progressDialog->setMaximum(totalSize / 1024);
    _progressDialog->show();

    QThread *thread = new QThread;
    Worker *worker = new Worker;
    worker->moveToThread(thread);
    connect(thread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(this, SIGNAL(needHash(const QStringList&, int)), worker, SLOT(doWork(const QStringList&, int)));
    connect(_progressDialog, SIGNAL(canceled()), worker, SLOT(cancel()));
    connect(worker, SIGNAL(resultReady(const QByteArray&)), this, SLOT(setPieces(const QByteArray&)));
    connect(worker, SIGNAL(progress(int)), _progressDialog, SLOT(setValue(int)));
    connect(worker, SIGNAL(resultReady(const QByteArray&)), thread, SLOT(quit()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    thread->start();

    emit needHash(files, pieceSize);

    _bencode->checkAndCreate(Bencode::Type::Dictionary, "info")->checkAndCreate(Bencode::Type::Integer, "piece length")->setInteger(pieceSize);
    _bencode->checkAndCreate(Bencode::Type::Integer, "creation date")->setInteger(QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000);
    if (files.size() == 1) {
        QString file = files.first();

        _bencode->child("info")->checkAndCreate(Bencode::Type::Integer, "length")->setInteger(totalSize);
        _bencode->child("info")->checkAndCreate(Bencode::Type::String, "name")->setString(fromUnicode(QFileInfo(file).fileName()));
    }
    else {
        _bencode->child("info")->checkAndCreate(Bencode::Type::String, "name")->setString(fromUnicode(baseDir.dirName()));
        _bencode->child("info")->checkAndCreate(Bencode::Type::List, "files");
        for (const QString &file: files) {
            Bencode *fileItem = new Bencode(Bencode::Type::Dictionary);
            fileItem->appendMapItem(new Bencode(QFileInfo(file).size(), "length"));

            QStringList pathList = baseDir.relativeFilePath(file).split("/");
            fileItem->appendMapItem(new Bencode(Bencode::Type::List, "path"));
            for (const QString &path: pathList) {
                fileItem->child("path")->appendChild(new Bencode(fromUnicode(path)));
            }
            _bencode->child("info")->child("files")->appendChild(fileItem);
        }
    }
}

void MainWindow::addFile()
{
    QStringList files = QFileDialog::getOpenFileNames(this, tr("Add File"));
    if (files.isEmpty())
        return;

    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->viewFiles->model());
    foreach (const QString &file, files) {
        QList<QStandardItem*> list;
        list << new QStandardItem(QDir::toNativeSeparators(file));
        list << new QStandardItem(smartSize(QFileInfo(file).size()));

        model->appendRow(list);
    }

    if (ui->leBaseFolder->text().isEmpty()) {
        ui->leBaseFolder->setText(QDir::toNativeSeparators(files.first().section('/', 0, -2)));
    }

    updateFilesSize();
}

void MainWindow::addFolder()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Add Folder"));
    if (path.isEmpty())
        return;

    QDirIterator it(path, QDirIterator::Subdirectories);

    QStringList files;

    while (it.hasNext()) {
        it.next();
        QFileInfo fileInfo = it.fileInfo();

        if (fileInfo.isFile())
            files << fileInfo.absoluteFilePath();
    }

    files.sort();

    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->viewFiles->model());
    qulonglong totalSize = 0;
    foreach (const QString &file, files) {
        QList<QStandardItem*> list;
        list << new QStandardItem(QDir::toNativeSeparators(file));
        list << new QStandardItem(smartSize(QFileInfo(file).size()));

        model->appendRow(list);
    }

    if (ui->leBaseFolder->text().isEmpty())
        ui->leBaseFolder->setText(path);
    ui->leTotalSize->setText(smartSize(totalSize));

    updateFilesSize();
}

void MainWindow::removeFile()
{
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->viewFiles->model());
    QItemSelectionModel *selectionModel = ui->viewFiles->selectionModel();

    if (!selectionModel->hasSelection())
        return;

    QModelIndexList indexes = selectionModel->selectedRows();

    for (int i = indexes.size() - 1; i >= 0; --i) {
        model->removeRow(indexes[i].row());
    }

    updateFilesSize();
}

void MainWindow::upFile()
{
    QItemSelectionModel *selectionModel = ui->viewFiles->selectionModel();

    if (!selectionModel->hasSelection())
        return;

    int row = selectionModel->selectedRows().at(0).row();
    if (row == 0)
        return;

    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->viewFiles->model());
    QList<QStandardItem*> list = model->takeRow(row);
    model->insertRow(row - 1, list);
    ui->viewFiles->selectRow(row - 1);
}

void MainWindow::downFile()
{
    QItemSelectionModel *selectionModel = ui->viewFiles->selectionModel();

    if (!selectionModel->hasSelection())
        return;

    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->viewFiles->model());
    int row = selectionModel->selectedRows().at(0).row();
    if (row == model->rowCount() - 1)
        return;

    QList<QStandardItem*> list = model->takeRow(row);
    model->insertRow(row + 1, list);
    ui->viewFiles->selectRow(row + 1);
}

void MainWindow::updateFiles()
{
    Bencode *info = _bencode->child("info");
    if (!info)
        return;

    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->viewFiles->model());

    // Fill files from _bencode if empty
    if (!model->rowCount()) {

        // Torrent contains only one file
        if (!info->child("files")) {
            QString baseName = toUnicode(info->child("name")->string());
            QList<QStandardItem*> list;
            list << new QStandardItem(baseName);
            list << new QStandardItem(smartSize(info->child("length")->integer()));
            model->appendRow(list);
        }
        else {
            Bencode *list = info->child("files");
            if (!list)
                return;

            for (int i = 0; i < list->childCount(); i++) {
                Bencode *item = list->child(i);
                QStringList path;
                Bencode *pathList = item->child("path");
                if (!pathList)
                    continue;

                for (int i = 0; i < pathList->childCount(); i++) {
                    path << toUnicode(pathList->child(i)->string());
                }

                QList<QStandardItem*> list2;
                list2 << new QStandardItem(path.join("/"));
                list2 << new QStandardItem(smartSize(item->child("length")->integer()));
                model->appendRow(list2);
            }
        }
    }

    QString dir = ui->leBaseFolder->text();
    if (!QDir(dir).exists())
        return;


    for (int i = 0; i < model->rowCount(); ++i) {
        QString file = model->item(i)->text();
        if (QFileInfo(file).isAbsolute())
            continue;

        file.prepend("/");
        file.prepend(dir);
        if (QFile::exists(file))
            model->item(i)->setText(QDir::toNativeSeparators(file));
    }
}

void MainWindow::setPieces(const QByteArray &pieces)
{
    _progressDialog->hide();

    if (!pieces.isEmpty()) {
        if (!_bencode->child("info"))
            _bencode->appendMapItem(new Bencode(Bencode::Type::Dictionary, "info"));

        if (!_bencode->child("pieces"))
            _bencode->child("info")->appendMapItem(new Bencode("", "pieces"));

        _bencode->child("info")->child("pieces")->setString(pieces);
    }
    else {
        delete _bencode;
        _bencode = new Bencode(Bencode::Type::Dictionary);
    }


    updateTab(ui->tabWidget->currentIndex());
}

void MainWindow::updateRawPosition()
{
    QTextCursor textCursor = ui->pteEditor->textCursor();
    ui->lblCursorPos->setText(QString(tr("Line: %1 of %2 Col: %3")).arg(textCursor.blockNumber() + 1).arg(ui->pteEditor->blockCount()).arg(textCursor.positionInBlock() + 1));
}

void MainWindow::addTreeItem()
{
    QItemSelectionModel *selectionModel = ui->treeJson->selectionModel();

    if (!selectionModel->hasSelection())
        return;

    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->treeJson->model());
    QStandardItem *item = model->itemFromIndex(selectionModel->selectedRows().at(0));
    if (item->data() != Bencode::List && item->data() != Bencode::Dictionary)
        return;

    QStringList items;
    items << Bencode::typeToStr(Bencode::Dictionary)
          << Bencode::typeToStr(Bencode::List)
          << Bencode::typeToStr(Bencode::String)
          << Bencode::typeToStr(Bencode::Integer);

    bool ok;
    QString type = QInputDialog::getItem(this, tr("New item type"), tr("Select a type of the new item"), items, 0, false, &ok);
    if (!ok)
        return;

    QList<QStandardItem*> row;
    row << new QStandardItem();
    row << new QStandardItem(type);
    row << new QStandardItem();
    row[1]->setEditable(false);
    if (Bencode::typeToStr(Bencode::Dictionary) == type) {
        row[2]->setEditable(false);
        row[0]->setData(Bencode::Dictionary);
    }
    else if (Bencode::typeToStr(Bencode::List) == type) {
        row[2]->setEditable(false);
        row[0]->setData(Bencode::List);
    }
    else if (Bencode::typeToStr(Bencode::Integer) == type) {
        row[0]->setData(Bencode::Integer);
    }
    else if (Bencode::typeToStr(Bencode::String) == type) {
        row[0]->setData(Bencode::String);
    }

    if (item->data() == Bencode::List) {
        row[0]->setEditable(false);
        row[0]->setText(QString::number(item->rowCount()));
    }
    else {
        row[0]->setText("new item");
    }

    item->appendRow(row);

    selectionModel->select(model->indexFromItem(row[0]), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    selectionModel->setCurrentIndex(model->indexFromItem(row[0]), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

void MainWindow::removeTreeItem()
{
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->treeJson->model());
    QItemSelectionModel *selectionModel = ui->treeJson->selectionModel();

    if (!selectionModel->hasSelection())
        return;

    QList<QPersistentModelIndex> indexes;
    foreach (const QModelIndex &index, ui->treeJson->selectionModel()->selectedIndexes()) {
        indexes << index;
    }

    foreach (const QPersistentModelIndex &index, indexes) {
        if (index.isValid())
            ui->treeJson->model()->removeRow(index.row(), index.parent());
    }

    updateBencodeFromJsonTree();
    updateJsonTree();
}

void MainWindow::upTreeItem()
{
    QItemSelectionModel *selectionModel = ui->treeJson->selectionModel();

    if (!selectionModel->hasSelection())
        return;

    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->treeJson->model());
    QStandardItem *item = model->itemFromIndex(selectionModel->selectedRows().at(0));
    QStandardItem *parent = item->parent();
    int row = item->row();
    if (!parent || row == 0 || parent->data().toInt() != Bencode::List)
        return;

    QList<QStandardItem*> list = parent->takeRow(row);
    parent->insertRow(row - 1, list);
    parent->child(row)->setText(QString::number(row));
    parent->child(row - 1)->setText(QString::number(row - 1));
    ui->treeJson->expand(model->indexFromItem(item));
    selectionModel->select(model->indexFromItem(item), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    selectionModel->setCurrentIndex(model->indexFromItem(item), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

    updateBencodeFromJsonTree();
    updateJsonTree();
}

void MainWindow::downTreeItem()
{
    QItemSelectionModel *selectionModel = ui->treeJson->selectionModel();

    if (!selectionModel->hasSelection())
        return;

    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->treeJson->model());
    QStandardItem *item = model->itemFromIndex(selectionModel->selectedRows().at(0));
    QStandardItem *parent = item->parent();
    int row = item->row();
    if (!parent || row == parent->rowCount() - 1 || parent->data().toInt() != Bencode::List)
        return;

    QList<QStandardItem*> list = parent->takeRow(row);
    parent->insertRow(row + 1, list);
    parent->child(row)->setText(QString::number(row));
    parent->child(row + 1)->setText(QString::number(row + 1));
    ui->treeJson->expand(model->indexFromItem(item));
    selectionModel->select(model->indexFromItem(item), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    selectionModel->setCurrentIndex(model->indexFromItem(item), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

    updateBencodeFromJsonTree();
    updateJsonTree();
}

void MainWindow::sortJsonTree(QStandardItem *item)
{
    if (item->column() != 0 || !item->parent() || item->parent()->data().toInt() != Bencode::Dictionary)
        return;

    QString key = item->text();
    QStandardItem *parent = item->parent();
    QList<QStandardItem*> row = parent->takeRow(item->row());

    for (int i = 0; i <= parent->rowCount(); ++i) {
        if (i == parent->rowCount() || parent->child(i)->text() > key) {
            parent->insertRow(i, row);
            break;
        }
    }

    QItemSelectionModel *selectionModel = ui->treeJson->selectionModel();
    selectionModel->setCurrentIndex(item->model()->indexFromItem(item), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    selectionModel->select(item->model()->indexFromItem(item), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    ui->treeJson->setFocus();
}

void MainWindow::updateSimple()
{
    // Avoid freezes
    processEvents();

    if (_bencode && _bencode->child("publisher-url"))
        ui->leUrl->setText(toUnicode(_bencode->child("publisher-url")->string()));
    else
        ui->leUrl->setText("");


    if (_bencode && _bencode->child("publisher"))
        ui->lePublisher->setText(toUnicode(_bencode->child("publisher")->string()));
    else
        ui->lePublisher->setText("");

    if (_bencode && _bencode->child("created by"))
        ui->leCreatedBy->setText(toUnicode(_bencode->child("created by")->string()));
    else
        ui->leCreatedBy->setText("");

    if (_bencode && _bencode->child("publisher-url"))
        ui->leUrl->setText(toUnicode(_bencode->child("publisher-url")->string()));
    else
        ui->leUrl->setText("");

    if (_bencode && _bencode->child("comment"))
        ui->pteComment->setPlainText(toUnicode(_bencode->child("comment")->string()));
    else
        ui->pteComment->setPlainText("");

    if (_bencode && _bencode->child("info") && _bencode->child("info")->child("name"))
        ui->leName->setText(toUnicode(_bencode->child("info")->child("name")->string()));
    else
        ui->leName->setText("");

    if (_bencode && _bencode->child("info") && _bencode->child("info")->child("piece length"))
        ui->lePieceSize->setText(smartSize(_bencode->child("info")->child("piece length")->integer()));
    else
        ui->lePieceSize->setText("");

    if (_bencode && _bencode->child("info") && _bencode->child("info")->child("pieces"))
        ui->lePieces->setText(QLocale::system().toString(_bencode->child("info")->child("pieces")->string().size() / 20));
    else
        ui->lePieces->setText("");

    QDateTime dateTime;
    if (_bencode && _bencode->child("creation date"))
        dateTime = QDateTime::fromMSecsSinceEpoch(_bencode->child("creation date")->integer() * 1000);

    ui->dateCreated->setDateTime(dateTime);


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


    ui->pteTrackers->setPlainText(trackers.join("\n"));

    QByteArray hash;
    if (_bencode && _bencode->child("info"))
        hash = QCryptographicHash::hash(_bencode->child("info")->toRaw(), QCryptographicHash::Sha1).toHex();
    ui->leHash->setText(hash);
}

void MainWindow::updateBencodeFromRaw()
{
    // Special case when no any text
    if (ui->pteEditor->toPlainText().trimmed().isEmpty()) {
        ui->lblRawError->setText("");
        delete _bencode;
        _bencode = new Bencode(Bencode::Type::Dictionary);
        updateTitle();
        return;
    }

    QByteArray ba = ui->pteEditor->toPlainText().toLatin1();
#ifdef HAVE_QT5
    QJsonParseError error;
    QVariant variant = QJsonDocument::fromJson(ba, &error).toVariant();
    if (error.error) {
        int line = ba.left(error.offset).count("\n") + 1;
        ui->lblRawError->setText(QString(tr("Error on %1 line: %2")).arg(line).arg(error.errorString()));
        return;
    }
#else
    QJson::Parser parser;
    bool ok;
    QVariant variant = parser.parse(ba, &ok);
    if (!ok) {
        ui->lblRawError->setText(QString(tr("Error on %1 line: %2")).arg(parser.errorLine()).arg(parser.errorString()));
        return;
    }
#endif

    ui->lblRawError->setText("");
    delete _bencode;
    _bencode = Bencode::fromJson(variant);
    updateTitle();
}

void MainWindow::updateRaw()
{
    // Avoid freezes
    processEvents();
    if (!_bencode->isValid()) {
        ui->pteEditor->setPlainText("");
        return;
    }

    QVariant res = _bencode->toJson();
#ifdef HAVE_QT5
    QByteArray ba = QJsonDocument::fromVariant(res).toJson();
#else
    QJson::Serializer serializer;
    serializer.setIndentMode(QJson::IndentFull);
    QByteArray ba = serializer.serialize(res);
#endif
    ui->pteEditor->setPlainText(QString::fromLatin1(ba));
}

void MainWindow::updateJsonTree()
{
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->treeJson->model());
    model->removeRows(0, model->rowCount());

    QList<QStandardItem*> row;
    row << new QStandardItem("root");
    row << new QStandardItem(Bencode::typeToStr(Bencode::Dictionary));
    row << new QStandardItem();
    row[0]->setData(Bencode::Dictionary);
    row[0]->setEditable(false);
    row[1]->setEditable(false);
    row[2]->setEditable(false);
    model->appendRow(row);

    if (!_bencode->isDictionary())
        return;

    processEvents();

    bencodeToStandardItem(row[0], _bencode);
    ui->treeJson->expandAll();
}

void MainWindow::bencodeToStandardItem(QStandardItem *parent, Bencode *bencode)
{
    switch (bencode->type()) {
    case Bencode::Dictionary: {
        // Will hope key is always in ascii
        QList<AbstractTreeItem*> map = bencode->children();
        for (int i = 0; i < map.size(); ++i) {
            Bencode *item = static_cast<Bencode*>(map.at(i));
            QByteArray key = item->key();

            QList<QStandardItem*> row;
            row << new QStandardItem(QString::fromLatin1(key));
            row << new QStandardItem();
            row << new QStandardItem();
            row[1]->setEditable(false);

            row[0]->setData(item->type());

            switch (item->type()) {
            case Bencode::List:
                row[2]->setEditable(false);
                row[1]->setText(Bencode::typeToStr(Bencode::List));
                bencodeToStandardItem(row[0], item);
                break;

            case Bencode::Dictionary:
                row[2]->setEditable(false);
                row[1]->setText(Bencode::typeToStr(Bencode::Dictionary));
                bencodeToStandardItem(row[0], item);
                break;

            case Bencode::String:
                row[1]->setText(Bencode::typeToStr(Bencode::String));
                if (key != QByteArray("pieces") &&
                    key != QByteArray("originator") &&
                    key != QByteArray("certificate") &&
                    key != QByteArray("signature")) {

                    row[2]->setText(toUnicode(item->string()));
                }
                else {
                    row[2]->setText(QString::fromLatin1(item->string().toHex()));
                }

                break;

            case Bencode::Integer:
                row[1]->setText(Bencode::typeToStr(Bencode::Integer));
                row[2]->setText(QString::number(item->integer()));
                break;

            default:
                break;
            }
            parent->appendRow(row);
        }
        break; }

    case Bencode::List: {
        QList<AbstractTreeItem*> list = bencode->children();
        for (int i = 0; i < list.size(); ++i) {
            Bencode *item = static_cast<Bencode*>(list.at(i));
            QList<QStandardItem*> row;
            row << new QStandardItem(QString::number(i));
            row << new QStandardItem();
            row << new QStandardItem();
            row[0]->setEditable(false);
            row[1]->setEditable(false);

            row[0]->setData(item->type());

            switch (item->type()) {
            case Bencode::List:
                row[2]->setEditable(false);
                row[1]->setText(Bencode::typeToStr(Bencode::List));
                bencodeToStandardItem(row[0], item);
                break;

            case Bencode::Dictionary:
                row[2]->setEditable(false);
                row[1]->setText(Bencode::typeToStr(Bencode::Dictionary));
                bencodeToStandardItem(row[0], item);
                break;

            case Bencode::String:
                row[1]->setText(Bencode::typeToStr(Bencode::String));
                row[2]->setText(toUnicode(item->string()));
                break;

            case Bencode::Integer:
                row[1]->setText(Bencode::typeToStr(Bencode::Integer));
                row[2]->setText(QString::number(item->integer()));
                break;

            default:
                break;
            }
            parent->appendRow(row);
        }
        break; }

    default:
        break;
    }
}

void MainWindow::standardItemToBencode(Bencode *parent, QStandardItem *item)
{
    switch (item->data().toInt()) {
    case Bencode::Dictionary:
        for (int i = 0; i < item->rowCount(); ++i) {
            QByteArray key = item->child(i)->text().toLatin1();

            Bencode *bencode = new Bencode(static_cast<Bencode::Type>(item->child(i)->data().toInt()), key);
            parent->appendMapItem(bencode);

            switch (item->child(i)->data().toInt()) {
            case Bencode::Dictionary:
            case Bencode::List:
                standardItemToBencode(bencode, item->child(i));
                break;

            case Bencode::Integer:
                bencode->setInteger(item->child(i, 2)->text().toLongLong());
                break;

            case Bencode::String:
                if (key != QByteArray("pieces"))
                    bencode->setString(fromUnicode(item->child(i, 2)->text()));
                else
                    bencode->setString(QByteArray::fromHex(item->child(i, 2)->text().toLatin1()));
                break;

            default:
                break;
            }
        }
        break;

    case Bencode::List:
        for (int i = 0; i < item->rowCount(); ++i) {
            Bencode *bencode = new Bencode(static_cast<Bencode::Type>(item->child(i)->data().toInt()));
            parent->appendChild(bencode);
            switch (item->child(i)->data().toInt()) {
            case Bencode::Dictionary:
            case Bencode::List:
                standardItemToBencode(bencode, item->child(i));
                break;

            case Bencode::Integer:
                bencode->setInteger(item->child(i, 2)->text().toLongLong());
                break;

            case Bencode::String:
                bencode->setString(fromUnicode(item->child(i, 2)->text()));
                break;

            default:
                break;
            }
        }
        break;

    default:
        break;
    }
}

bool MainWindow::saveTo(const QString &fileName)
{
    if (!_bencode->isValid())
        return false;

    QFile file(fileName);
    file.open(QIODevice::WriteOnly);
    file.write(_bencode->toRaw());
    file.close();

    delete _originBencode;
    _originBencode = _bencode->clone();

    return true;
}

void MainWindow::updateFilesSize()
{
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->viewFiles->model());
    qulonglong totalSize = 0;

    for (int i = 0; i < model->rowCount(); ++i) {
        QString file = model->item(i)->text();
        totalSize += QFileInfo(file).size();
    }

    ui->leTotalSize->setText(smartSize(totalSize));
}

QString MainWindow::smartSize(qulonglong size)
{
    double kb = size;
    int i = 0;

    // clang doesn't support 'd' suffix
#ifdef __clang__
    while (kb >= 1024.0) {
        kb /= 1024.0;
#else
    while (kb >= 1024.0d) {
        kb /= 1024.0d;
#endif
        i++;
    }

    QString res = QLocale::system().toString(kb, 'g', 4);

    // Drop zeroes
    while (res.contains(QLocale::system().decimalPoint()) && res.right(1) == "0")
        res.chop(1);

    if (res.right(1)[0] == QLocale::system().decimalPoint())
        res.chop(1);

    switch (i) {
        case 0:
            res += " " + tr("B");
            break;

        case 1:
            res += " " + tr("KiB");
            break;

        case 2:
            res += " " + tr("MiB");
            break;

        case 3:
            res += " " + tr("GiB");
            break;

        case 4:
            res += " " + tr("TiB");
            break;
    }

    return res;
}

void MainWindow::processEvents()
{
    // Hack to prevent processEvents when application is not starting yet
    if (isVisible())
        qApp->processEvents();
}
