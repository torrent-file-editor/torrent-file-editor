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
#include "searchdlg.h"

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
#include <QMimeData>
#include <QElapsedTimer>

#ifdef HAVE_QT5
# include <QJsonDocument>
#else
# include <qjson/serializer.h>
# include <qjson/parser.h>
#endif

#define PROGRESS_TIMEOUT 500 /* ms */

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

    QElapsedTimer timer;
    timer.start();

    for (int i = 0; i < files.size(); ++i) {
        QFile f(files[i]);
        if (!f.open(QIODevice::ReadOnly)) {
            QString errorString = QString(tr("Can't open %1")).arg(QDir::toNativeSeparators(files[i]));
            emit resultReady(QByteArray(), errorString);
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

            // Do not send progress signal very often. It can leads to crash.
            if (timer.hasExpired(PROGRESS_TIMEOUT)) {
                emit progress(value / 1024);
                timer.restart();
            }

            qApp->processEvents();
            if (_isCanceled) {
                emit resultReady(QByteArray(), "");
                f.close();
                return;
            }
        }

        // Some error
        if (readed < 0) {
            QString errorString = QString(tr("Can't read from %1")).arg(QDir::toNativeSeparators(files[i]));
            emit resultReady(QByteArray(), errorString);
            f.close();
            return;
        }

        f.close();
    }

    emit resultReady(pieceHashes, "");
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
#ifdef Q_OS_WIN
    , _progressDialog(new QProgressDialog(this, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint))
#else
    , _progressDialog(new QProgressDialog(this, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint))
#endif
    , _formatFilters(QStringList())
    , _lastFolder()
    , _searchDlg(0)
{
    ui->setupUi(this);

    _formatFilters << tr("Torrents (*.torrent)");
    _formatFilters << tr("uTorrent resume files (*.dat)");

#ifndef Q_OS_MAC
    // Show menu bar only on Mac OS X.
    // Menu bar on Mac OS is persistent. No sense to not use it.
    ui->menuBar->hide();

    // Workaround: shortcuts are not working for "hidden" actions. Fix it.
    ui->btnNew->addAction(ui->actNew);
    ui->btnOpen->addAction(ui->actOpen);
    ui->btnSave->addAction(ui->actSave);
    ui->btnSaveAs->addAction(ui->actSaveAs);
#else
    // Hide About button on Mac OS X to comply good GUI style on Mac OS X.
    ui->btnAbout->hide();
#endif

    _progressDialog->setWindowModality(Qt::ApplicationModal);
    _progressDialog->setLabelText(tr("Need to calculate piece hashes"));
    _progressDialog->setWindowTitle(tr("Please wait"));
    _progressDialog->setValue(_progressDialog->maximum());
    _progressDialog->ensurePolished();
    _progressDialog->adjustSize();
    _progressDialog->setFixedSize(_progressDialog->size().width() * 2, _progressDialog->size().height());

    updateTitle();

    _instance = this;

    ui->cmbPieceSizes->addItem(tr("Auto"), 0);
    for (int i = 5; i < 16; ++i) {
        qulonglong pieceSize = 1024 * static_cast<qulonglong>(qPow(2, i));
        ui->cmbPieceSizes->addItem(smartSize(pieceSize), pieceSize);
    }

    QStandardItemModel *model = new QStandardItemModel(0, 4, this);
    QStringList headers;
    headers << tr("Path") << tr("Size") << tr("# Pieces") << "" /* dummy */;
    model->setHorizontalHeaderLabels(headers);
    model->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
    model->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignRight);
    model->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignRight);
    ui->viewFiles->setModel(model);
    ui->viewFiles->horizontalHeader()->setHighlightSections(false);
#ifdef HAVE_QT5
    ui->viewFiles->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->viewFiles->horizontalHeader()->setSectionsMovable(false);
#else
    ui->viewFiles->verticalHeader()->setResizeMode(QHeaderView::Fixed);
    ui->viewFiles->horizontalHeader()->setMovable(false);
#endif
    ui->viewFiles->horizontalHeader()->setMinimumSectionSize(0);
    ui->viewFiles->horizontalHeader()->resizeSection(3, 0);
    connect(ui->viewFiles, SIGNAL(deleteRow()), SLOT(removeFile()));

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
    ui->btnFilesFilter->setIcon(QIcon(":/icons/files-filter.png"));

    ui->btnAbout->setIcon(qApp->style()->standardIcon(QStyle::SP_MessageBoxQuestion));

    ui->btnAddTreeItem->setIcon(QIcon::fromTheme("list-add", QIcon(":/icons/list-add.png")));
    ui->btnRemoveTreeItem->setIcon(QIcon::fromTheme("edit-delete", QIcon(":/icons/edit-delete.png")));
    ui->btnUpTreeItem->setIcon(qApp->style()->standardIcon(QStyle::SP_ArrowUp));
    ui->btnDownTreeItem->setIcon(qApp->style()->standardIcon(QStyle::SP_ArrowDown));
    ui->btnFindTreeItem->setIcon(QIcon::fromTheme("edit-find", QIcon(":/icons/edit-find")));
    ui->btnReplaceTreeItem->setIcon(QIcon::fromTheme("edit-find-replace", QIcon(":/icons/edit-find-replace")));

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
    ui->pteLog->appendPlainText(log);
#endif
}

void MainWindow::create()
{
    if (!showNeedSaveFile())
        return;

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
    if (!showNeedSaveFile())
        return;

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open"), "", _formatFilters.join(";;"));

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

    QString filter;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"), "", _formatFilters.join(";;"), &filter);
    if (fileName.isEmpty())
        return;

    if (_formatFilters.at(0) == filter && !fileName.endsWith(".torrent"))
        fileName += ".torrent";
    else if (_formatFilters.at(1) == filter && !fileName.endsWith(".dat"))
        fileName += ".dat";

    if (saveTo(fileName)) {
        _fileName = fileName;
        updateTitle();
    }
}

void MainWindow::showAbout()
{
    AboutDlg dlg(this);
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

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (!event->mimeData()->urls().isEmpty() && event->mimeData()->urls().first().isLocalFile()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    if (!event->mimeData()->urls().isEmpty() && event->mimeData()->urls().first().isLocalFile()) {
        open(event->mimeData()->urls().first().toLocalFile());
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (showNeedSaveFile())
        event->accept();
    else
        event->ignore();
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
        setWindowTitle(qApp->applicationName());
    else if (isModified())
        setWindowTitle(QString("* %2 - %1").arg(qApp->applicationName(), _fileName));
    else
        setWindowTitle(QString("%2 - %1").arg(qApp->applicationName(), _fileName));
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
    _bencodeModel->setPrivateTorrent(ui->chkPrivateTorrent->isChecked());
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

    // Check for relative paths
    bool hasRelative = false;

    do {
        for (int i = 0; i < model->rowCount(); ++i) {
            QString file = model->item(i)->text();
            if (QFileInfo(file).isRelative()) {
                hasRelative = true;
                break;
            }
        }

        if (hasRelative) {
            QMessageBox::StandardButton button;
            button = QMessageBox::question(this,
                                           tr("Torrent root folder is not set"),
                                           tr(
"Path to files on the disk in not known. Torrent can be generated only from fully downloaded files.\n\n"
"If you want to edit file list in the current torrent you need to set torrent root folder. The torrent root folder is a folder where all files can be located on the disk. Actual file path on the disk is torrent root folder with relative path from torrent file. If something files are missing then torrent can't be generated.\n\n"
"Do you want to set torrent root folder and try again?"
                                              ), QMessageBox::Yes | QMessageBox::No);
            if (button == QMessageBox::Yes) {
                ui->leBaseFolder->openFolder();
            }
            else {
                break;
            }
        }
    } while(hasRelative);


    if (hasRelative)
        return;

    // Check for common origin folder and calculate total torrent size
    for (int i = 0; i < model->rowCount(); ++i) {
        QString file = model->item(i)->text();

        if (model->rowCount() > 1 && baseDir.relativeFilePath(file).startsWith("..")) {
            QMessageBox::warning(this, tr("Warning"), tr("The torrent root folder is not common."));
            return;
        }

        // Check for base folder
        files << QDir::fromNativeSeparators(file);
        totalSize += QFileInfo(file).size();
    }

    qulonglong pieceSize = autoPieceSize();

    _progressDialog->setMaximum(totalSize / 1024);
    _progressDialog->show();

    QThread *thread = new QThread;
    Worker *worker = new Worker;
    worker->moveToThread(thread);
    connect(thread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(this, SIGNAL(needHash(const QStringList&, int)), worker, SLOT(doWork(const QStringList&, int)));
    connect(_progressDialog, SIGNAL(canceled()), worker, SLOT(cancel()));
    connect(worker, SIGNAL(resultReady(const QByteArray&, const QString&)), this, SLOT(setPieces(const QByteArray&, const QString&)));
    connect(worker, SIGNAL(progress(int)), _progressDialog, SLOT(setValue(int)));
    connect(worker, SIGNAL(resultReady(const QByteArray&, const QString&)), thread, SLOT(quit()));
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
    QStringList files = QFileDialog::getOpenFileNames(this, tr("Add File"), _lastFolder);
    if (files.isEmpty())
        return;

    _lastFolder = QFileInfo(files.first()).absolutePath();
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->viewFiles->model());
    foreach (const QString &file, files) {
        addFilesRow(file, QFileInfo(file).size());
    }

    if (ui->leBaseFolder->text().isEmpty()) {
        ui->leBaseFolder->setText(QDir::toNativeSeparators(files.first().section('/', 0, -2)));
    }

    updateFilesSize();
    ui->viewFiles->scrollToBottom();
}

void MainWindow::addFolder()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Add Folder"), _lastFolder);
    if (path.isEmpty())
        return;

    _lastFolder = QFileInfo(path).absolutePath();
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
        addFilesRow(file, QFileInfo(file).size());
    }

    if (ui->leBaseFolder->text().isEmpty())
        ui->leBaseFolder->setText(path);

    updateFilesSize();
    ui->viewFiles->scrollToBottom();
}

void MainWindow::removeFile()
{
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->viewFiles->model());
    QItemSelectionModel *selectionModel = ui->viewFiles->selectionModel();

    if (!selectionModel->hasSelection())
        return;

    QModelIndexList indexes = selectionModel->selectedRows();
    int row = indexes.size() == 1 ? indexes.first().row() : -1;

    for (int i = indexes.size() - 1; i >= 0; --i) {
        model->removeRow(indexes[i].row());
    }

    updateFilesSize();
    ui->viewFiles->selectRow(row);
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
            addFilesRow(file.first, file.second);
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

void MainWindow::setPieces(const QByteArray &pieces, const QString &errorString)
{
    _progressDialog->hide();
    _bencodeModel->setPieces(pieces);
    updateTab(ui->tabWidget->currentIndex());
    if (!errorString.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), errorString);
    }
}

void MainWindow::updateRawPosition()
{
    QTextCursor textCursor = ui->pteEditor->textCursor();
    ui->lblCursorPos->setText(QString(tr("Line: %1 of %2 Col: %3")).arg(textCursor.blockNumber() + 1).arg(ui->pteEditor->blockCount()).arg(textCursor.positionInBlock() + 1));
}

void MainWindow::filterFiles()
{

    FilesFilters filter = static_cast<FilesFilters>(ui->cmbFilesFilter->currentIndex());
    QString pattern = QDir::fromNativeSeparators(ui->lneFilesFilter->text());
    if (pattern.isEmpty())
        return;

    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->viewFiles->model());
    for (int i =  model->rowCount() - 1; i >= 0; --i) {
        QString file = model->item(i)->text();
        QFileInfo fi(file);

#ifdef Q_OS_LINUX
        // Linux Ext2/3/4 file systems are case sensetive
        Qt::CaseSensitivity cs = Qt::CaseSensitive;
#else
        // On Windows FAT32 and NTFS file systems are case insensetive
        // On Mac OS HFS file system is case insensetive by default
        Qt::CaseSensitivity cs = Qt::CaseInsensitive;
#endif

        bool removeFile = false;
        switch (filter) {
        case FilesFilters::NameFilter:
            if (!fi.fileName().compare(pattern, cs))
                removeFile = true;
            break;

        case FilesFilters::ExtenstionFilter:
            if (!fi.suffix().compare(pattern, cs))
                removeFile = true;
            break;

        case FilesFilters::TemplateFilter: {
            QRegExp rx(pattern, cs, QRegExp::Wildcard);
            if (rx.indexIn(fi.fileName()) >= 0)
                removeFile = true;
            } break;

        case FilesFilters::RegExpFilter: {
            QRegExp rx(pattern, cs);
            if (rx.indexIn(fi.filePath()) >= 0)
                removeFile = true;
            } break;
        }

        if (removeFile) {
            model->removeRow(i);
        }
    }

    updateFiles();
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

void MainWindow::showTreeSearchWindow()
{
    if (!_searchDlg) {
        _searchDlg = new SearchDlg(_bencodeModel, this);
        connect(_searchDlg, SIGNAL(foundItem(const QModelIndex&)), SLOT(selectTreeItem(QModelIndex)));
    }

    _searchDlg->setReplaceModeEnabled(false);
    _searchDlg->show();
}

void MainWindow::showTreeReplaceWindow()
{
    if (!_searchDlg) {
        _searchDlg = new SearchDlg(_bencodeModel, this);
        connect(_searchDlg, SIGNAL(foundItem(const QModelIndex&)), SLOT(selectTreeItem(QModelIndex)));
    }

    _searchDlg->setReplaceModeEnabled(true);
    _searchDlg->show();
}

void MainWindow::selectTreeItem(const QModelIndex &index)
{
    ui->treeJson->setCurrentIndex(index);
    ui->treeJson->scrollTo(index);
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
    ui->chkPrivateTorrent->setChecked(_bencodeModel->privateTorrent());
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

qulonglong MainWindow::autoPieceSize() const
{
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->viewFiles->model());
    qulonglong totalSize = 0;

    for (int i = 0; i < model->rowCount(); ++i) {
        totalSize += model->item(i, 1)->data().toLongLong();
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
    return pieceSize;
}

void MainWindow::updateFilesSize()
{
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->viewFiles->model());
    qulonglong totalSize = 0;

    for (int i = 0; i < model->rowCount(); ++i) {
        totalSize += model->item(i, 1)->data().toLongLong();
    }

    ui->leTotalSize->setText(smartSize(totalSize));

    updateFilesPieces();
}

void MainWindow::addFilesRow(const QString &path, qulonglong size)
{
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->viewFiles->model());
    Q_ASSERT(model);

    QList<QStandardItem*> list;
    list << new QStandardItem(QDir::toNativeSeparators(path));
    list << new QStandardItem(smartSize(size));
    list.last()->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    list.last()->setData(size);
    list << new QStandardItem();
    list.last()->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

    model->appendRow(list);
}

void MainWindow::updateFilesPieces()
{
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->viewFiles->model());
    if (!model)
        return;

    qulonglong pieceSize = autoPieceSize();
    qulonglong totalSize = 0;

    for (int i = 0; i < model->rowCount(); ++i) {
        qulonglong size = model->item(i, 1)->data().toLongLong();
        int firstPiece = (totalSize + 1) / pieceSize;
        int lastPiece = (totalSize + size) / pieceSize;
        totalSize += size;
        model->item(i, 2)->setText(QString::number(lastPiece - firstPiece + 1));
    }

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

bool MainWindow::showNeedSaveFile()
{
    bool res = true;
    if (isModified()) {
        QString title = tr("Save file");
        QString filename = _fileName.isEmpty() ? tr("Untitled") : _fileName;
        QString question = tr("Save file \"%1\"?").arg(QDir::toNativeSeparators(filename));
        QMessageBox::StandardButton bt = QMessageBox::question(this, title, question, QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        switch (bt) {
        case QMessageBox::StandardButton::Yes:
            save();
            break;

        case QMessageBox::StandardButton::Cancel:
            res = false;
            break;

        default:
            break;
        }
    }

    return res;
}
