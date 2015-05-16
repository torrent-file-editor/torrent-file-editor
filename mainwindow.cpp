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
#include "bencodemodel.h"
#include "bencodedelegate.h"

#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
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
#include <QCryptographicHash>
#include <QTextDocument>

#ifdef HAVE_QT5
# include <QJsonDocument>
#else
# include <qjson/serializer.h>
# include <qjson/parser.h>
#endif

#define APP_NAME "Torrent File Editor 0.2.0"

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
    , _fileName(QString())
    , _bencodeModel(new BencodeModel(this))
    , _progressDialog(new QProgressDialog(this))
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
    ui->viewFiles->horizontalHeader()->setSectionsMovable(false);
#else
    ui->viewFiles->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->viewFiles->verticalHeader()->setResizeMode(QHeaderView::Fixed);
    ui->viewFiles->horizontalHeader()->setMovable(false);
#endif
    ui->treeJson->setModel(_bencodeModel);
    ui->treeJson->setItemDelegate(new BencodeDelegate(this));
#ifdef HAVE_QT5
    for (int i = 0; i < static_cast<int>(BencodeModel::Column::Count); i++) {
        ui->treeJson->header()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
    }
    ui->treeJson->header()->setSectionsMovable(false);
#else
    for (int i = 0; i < static_cast<int>(BencodeModel::Column::Count); i++) {
        ui->treeJson->header()->setResizeMode(i, QHeaderView::ResizeToContents);
    }
    ui->treeJson->header()->setMovable(false);
#endif

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

    connect(_bencodeModel, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), SLOT(updateTitle()));
    connect(_bencodeModel, SIGNAL(rowsMoved(const QModelIndex&, int, int, const QModelIndex&, int)), SLOT(updateTitle()));
    connect(_bencodeModel, SIGNAL(rowsRemoved(const QModelIndex&, int, int)), SLOT(updateTitle()));
    connect(_bencodeModel, SIGNAL(rowsInserted(const QModelIndex&, int, int)), SLOT(updateTitle()));
    connect(_bencodeModel, SIGNAL(layoutChanged()), SLOT(updateTitle()));
    connect(_bencodeModel, SIGNAL(modelReset()), SLOT(updateTitle()));
}

MainWindow::~MainWindow()
{
    _instance = 0;
    delete ui;
}

MainWindow *MainWindow::instance()
{
    return _instance;
}

void MainWindow::addLog(const QString &log)
{
#ifdef DEBUG
//    ui->pteLog->appendPlainText(log);
#endif
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

    _bencodeModel->setRaw("");
    _bencodeModel->resetModified();
    _fileName = "";
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

    _bencodeModel->setRaw(file.readAll());
    file.close();

    updateTab(ui->tabWidget->currentIndex());
    _bencodeModel->resetModified();
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

    saveTo(_fileName);
}

void MainWindow::saveAs()
{
    if (!_bencodeModel->isValid())
        return;

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"), "", tr("Torrents (*.torrent)"));
    if (fileName.isEmpty())
        return;

    if (!fileName.endsWith(".torrent"))
        fileName += ".torrent";

    if (saveTo(fileName))
        _fileName = fileName;
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

bool MainWindow::isModified() const
{
    return _bencodeModel->isModified();
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
    _bencodeModel->setName(ui->leName->text());
    _bencodeModel->setUrl(ui->leUrl->text());
    _bencodeModel->setPublisher(ui->lePublisher->text());
    _bencodeModel->setCreatedBy(ui->leCreatedBy->text());
    _bencodeModel->setCreationTime(ui->dateCreated->dateTime());
    ui->leHash->setText(_bencodeModel->hash());
}

void MainWindow::updateBencodeFromComment()
{
    _bencodeModel->setComment(ui->pteComment->toPlainText());
}

void MainWindow::updateBencodeFromTrackers()
{
    _bencodeModel->setTrackers(ui->pteTrackers->toPlainText().trimmed().split("\n"));
}

void MainWindow::updateEncoding(const QString &encoding)
{
    _bencodeModel->setTextCodec(QTextCodec::codecForName(encoding.toUtf8()));
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

    _bencodeModel->setCreationTime(QDateTime::currentDateTime());
    _bencodeModel->setPieceSize(pieceSize);
    if (files.size() == 1) {
        _bencodeModel->setName(QFileInfo(files.at(0)).fileName());
    }
    else {
        _bencodeModel->setName(baseDir.dirName());
    }

    QList<QPair<QString, qlonglong>> filePairs;
    if (files.size() == 1) {
        filePairs << QPair<QString, qlonglong>("", totalSize);
    }
    else {
        for (const QString &file: files) {
            filePairs << QPair<QString, qlonglong>(baseDir.relativeFilePath(file), QFileInfo(file).size());
        }
    }
    _bencodeModel->setFiles(filePairs);
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
        list.last()->setData(QFileInfo(file).size());

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
        list.last()->setData(QFileInfo(file).size());

        model->appendRow(list);
    }

    if (ui->leBaseFolder->text().isEmpty())
        ui->leBaseFolder->setText(path);

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
    QList<QPair<QString, qlonglong>> files = _bencodeModel->files();
    if (files.isEmpty())
        return;

    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->viewFiles->model());

    // Fill files from _bencode if empty
    if (!model->rowCount()) {
        qlonglong totalSize = 0;
        for (const auto file: files) {
            QList<QStandardItem*> list;
            totalSize += file.second;
            list << new QStandardItem(file.first);
            list << new QStandardItem(smartSize(file.second));
            list.last()->setData(file.second);
            model->appendRow(list);
        }
        qlonglong pieceSize = _bencodeModel->pieceSize();
        for (int i = 0; i < ui->cmbPieceSizes->count(); i++) {
            if (pieceSize == ui->cmbPieceSizes->itemData(i).toULongLong())
                ui->cmbPieceSizes->setCurrentIndex(i);
        }
    }

    QString dir = ui->leBaseFolder->text();
    if (!QDir(dir).exists())
        return;

    // Try to find files on disk and set full path if exists
    for (int i = 0; i < model->rowCount(); ++i) {
        QString file = model->item(i)->text();
        if (QFileInfo(file).isAbsolute())
            continue;

        file.prepend("/");
        file.prepend(dir);
        if (QFile::exists(file))
            model->item(i)->setText(QDir::toNativeSeparators(file));
    }

    updateFilesSize();
}

void MainWindow::setPieces(const QByteArray &pieces)
{
    _progressDialog->hide();
    _bencodeModel->setPieces(pieces);
    updateTab(ui->tabWidget->currentIndex());
}

void MainWindow::updateRawPosition()
{
    QTextCursor textCursor = ui->pteEditor->textCursor();
    ui->lblCursorPos->setText(QString(tr("Line: %1 of %2 Col: %3")).arg(textCursor.blockNumber() + 1).arg(ui->pteEditor->blockCount()).arg(textCursor.positionInBlock() + 1));
}

void MainWindow::addTreeItem()
{
    QModelIndex index = ui->treeJson->currentIndex();
    if (!index.isValid())
        return;

    index = index.sibling(index.row(), 0);
    _bencodeModel->appendRow(index);
}

void MainWindow::removeTreeItem()
{
    if (!ui->treeJson->currentIndex().isValid() || !ui->treeJson->currentIndex().parent().isValid())
        return;

    int row = -1;
    QModelIndex parent;
    if (ui->treeJson->selectionModel()->selectedRows().size() == 1) {
        row = ui->treeJson->currentIndex().row();
        parent = ui->treeJson->currentIndex().parent();
    }

    QList<QPersistentModelIndex> indexes;

    for (const QModelIndex &i: ui->treeJson->selectionModel()->selectedRows()) {
        // Skip root item
        if (!i.parent().isValid())
            continue;

        indexes << i;
    }

    for (const QPersistentModelIndex &i: indexes) {
        if (i.isValid())
            ui->treeJson->model()->removeRow(i.row(), i.parent());
    }

    if (row >= 0 && row < _bencodeModel->rowCount(parent)) {
        ui->treeJson->setCurrentIndex(parent.child(row, 0));
    }
    else {
        ui->treeJson->setCurrentIndex(QModelIndex());
    }
}

void MainWindow::upTreeItem()
{
    _bencodeModel->up(ui->treeJson->currentIndex());
}

void MainWindow::downTreeItem()
{
    _bencodeModel->down(ui->treeJson->currentIndex());
}

void MainWindow::updateSimple()
{
    // Avoid freezes
    processEvents();

    ui->leUrl->setText(_bencodeModel->url());
    ui->lePublisher->setText(_bencodeModel->publisher());
    ui->leCreatedBy->setText(_bencodeModel->createdBy());
    ui->pteComment->setPlainText(_bencodeModel->comment());
    ui->leName->setText(_bencodeModel->name());

    int pieceSize = _bencodeModel->pieceSize();
    ui->lePieceSize->setText(pieceSize ? smartSize(pieceSize) : 0);

    int pieces = _bencodeModel->pieces();
    ui->lePieces->setText(pieces ? QLocale::system().toString(pieces) : "");

    ui->dateCreated->setDateTime(_bencodeModel->creationTime());
    ui->pteTrackers->setPlainText(_bencodeModel->trackers().join("\n"));
    ui->leHash->setText(_bencodeModel->hash());
}

void MainWindow::updateBencodeFromRaw()
{
    if (!ui->pteEditor->document()->isModified())
        return;

    // Special case when no any text
    if (ui->pteEditor->toPlainText().trimmed().isEmpty()) {
        ui->lblRawError->setText("");
        _bencodeModel->setJson(QVariant());
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
    _bencodeModel->setJson(variant);
}

void MainWindow::updateRaw()
{
    // Avoid freezes
    processEvents();
    QVariant res = _bencodeModel->toJson();
    if (!res.isValid()) {
        ui->pteEditor->setPlainText("");
        ui->pteEditor->document()->setModified(false);
        return;
    }

#ifdef HAVE_QT5
    QByteArray ba = QJsonDocument::fromVariant(res).toJson();
#else
    QJson::Serializer serializer;
    serializer.setIndentMode(QJson::IndentFull);
    QByteArray ba = serializer.serialize(res);
#endif
    ui->pteEditor->setPlainText(QString::fromLatin1(ba));
    ui->pteEditor->document()->setModified(false);
}

bool MainWindow::saveTo(const QString &fileName)
{
    if (!_bencodeModel->isValid())
        return false;

    QByteArray raw = _bencodeModel->toRaw();
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);
    file.write(raw);
    file.close();

    _bencodeModel->resetModified();
    updateTitle();

    return true;
}

void MainWindow::updateFilesSize()
{
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->viewFiles->model());
    qulonglong totalSize = 0;

    for (int i = 0; i < model->rowCount(); ++i) {
        totalSize += model->item(i, 1)->data().toLongLong();
    }

    ui->leTotalSize->setText(smartSize(totalSize));
}

QString MainWindow::smartSize(qulonglong size)
{
    double kb = size;
    int i = 0;

    while (kb >= 1024.0) {
        kb /= 1024.0;
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
