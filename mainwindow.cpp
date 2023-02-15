/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 *
 * Copyright (C) 2014-2015  Ivan Romanov <drizt72@zoho.eu>
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
#include <QShortcut>
#include <QClipboard>
#include <QTranslator>
#include <QLibraryInfo>

#ifdef HAVE_QT5
# include <QJsonDocument>
#else
# include <qjson/serializer.h>
# include <qjson/parser.h>
#endif

#ifdef Q_OS_WIN
# include <io.h>
# include <windows.h>
#endif

#define PROGRESS_TIMEOUT 500 /* ms */

// FIXME: workaround for symlink wrong size https://bugreports.qt.io/browse/QTBUG-24831

static qint64 fileSize(const QString &path)
{
#ifdef Q_OS_UNIX
    return QFileInfo(path).size();
#else
    QFileInfo fi(path);
    if (!fi.isSymLink()) {
        return fi.size();
    }
    else {
        QFile file(path);
        file.open(QFile::ReadOnly); // it must be open to get a windows file handle
        HANDLE hFile = reinterpret_cast<HANDLE>(_get_osfhandle(file.handle()));
        DWORD size = GetFileSize(hFile, nullptr);
        file.close();
        return static_cast<qint64>(size);
    }
#endif
}

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
        while ((readed = f.read(piece.data() + piecePos, pieceSize - piecePos)) > 0) {  // -V104 PVS-Studio
            piecePos += readed;
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
                emit resultReady(QByteArray(), QString());
                f.close();
                return;
            }

            if (f.atEnd()) {
                break;
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

    emit resultReady(pieceHashes, QString());
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
    , _torrentLastFolder()
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

    ui->cmbPieceSizes->addItem(QString(), 0);
    for (int i = 5; i < 16; ++i) {
        qulonglong pieceSize = 1024 * static_cast<qulonglong>(qPow(2, i));
        ui->cmbPieceSizes->addItem(smartSize(pieceSize), pieceSize);
    }

#ifdef Q_OS_WIN
    setStyleSheet(QStringLiteral("QSplitter::handle { background-color: rgba(0, 0, 0, 0) }\n"
                                 "QSplitter { background-color: rgba(0, 0, 0, 0) }"));
#endif

    QStandardItemModel *model = new QStandardItemModel(0, 4, this);
    QStringList headers;
    headers << QString() << QString() << QString() << QString() /* dummy */;
    model->setHorizontalHeaderLabels(headers);
    model->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft); // -V525 PVS-Studio
    model->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignRight);
    model->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignRight);
    ui->viewFiles->setModel(model);
    ui->viewFiles->horizontalHeader()->setHighlightSections(false); // -V807 PVS-Studio
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

    ui->btnNew->setIcon(QIcon::fromTheme(QStringLiteral("text-x-generic")));
    ui->btnOpen->setIcon(qApp->style()->standardIcon(QStyle::SP_DialogOpenButton)); // -V807 PVS-Studio
    ui->btnSave->setIcon(qApp->style()->standardIcon(QStyle::SP_DialogSaveButton));
    ui->btnSaveAs->setIcon(qApp->style()->standardIcon(QStyle::SP_DialogSaveButton));

    ui->btnRemoveFiles->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));

    ui->btnMakeTorrent->setIcon(QIcon(QStringLiteral(":/icons/hammer.png")));
    ui->btnAddFile->setIcon(QIcon::fromTheme(QStringLiteral("document-new")));
    ui->btnAddFolder->setIcon(QIcon::fromTheme(QStringLiteral("folder-new")));
    ui->btnReloadFiles->setIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));
    ui->btnUpFile->setIcon(qApp->style()->standardIcon(QStyle::SP_ArrowUp));
    ui->btnDownFile->setIcon(qApp->style()->standardIcon(QStyle::SP_ArrowDown));
    ui->btnFilesFilter->setIcon(QIcon(QStringLiteral(":/icons/files-filter.png")));

    ui->btnAbout->setIcon(qApp->style()->standardIcon(QStyle::SP_MessageBoxQuestion));

    ui->btnAddTreeItem->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    ui->btnRemoveTreeItem->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));
    ui->btnUpTreeItem->setIcon(qApp->style()->standardIcon(QStyle::SP_ArrowUp));
    ui->btnDownTreeItem->setIcon(qApp->style()->standardIcon(QStyle::SP_ArrowDown));
    ui->btnFindTreeItem->setIcon(QIcon::fromTheme(QStringLiteral("edit-find")));
    ui->btnReplaceTreeItem->setIcon(QIcon::fromTheme(QStringLiteral("edit-find-replace")));

    new QShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_G), this, SLOT(copyMagnetLink()));
    new QShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_T), this, SLOT(copyMagnetExtra()));

    fillCoding();
    updateFilesSize();

    connect(_bencodeModel, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), SLOT(updateTitle()));
    connect(_bencodeModel, SIGNAL(rowsMoved(const QModelIndex&, int, int, const QModelIndex&, int)), SLOT(updateTitle()));
    connect(_bencodeModel, SIGNAL(rowsRemoved(const QModelIndex&, int, int)), SLOT(updateTitle()));
    connect(_bencodeModel, SIGNAL(rowsInserted(const QModelIndex&, int, int)), SLOT(updateTitle()));
    connect(_bencodeModel, SIGNAL(layoutChanged()), SLOT(updateTitle()));
    connect(_bencodeModel, SIGNAL(modelReset()), SLOT(updateTitle()));

    ui->cmbTranslation->hide();
    _showTranslations = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_T), this, SLOT(showTranslations()));

    changeTranslation(-1);
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

void MainWindow::showTranslations()
{
    _showTranslations->setEnabled(false);

    QDir dir(QStringLiteral(":/translations"));
    QStringList translations = dir.entryList(QDir::Filter::NoDotAndDotDot | QDir::Filter::Files);
    QString langs;
    QString locales;
    for (const QString &translation: translations) {
        QString lang = translation.section(QLatin1Char('_'), 1).section(QLatin1Char('.'), 0, 0);
        langs += lang + QLatin1String(" ");
        QLocale locale(lang);
        locales += locale.name() + QLatin1String(" ");

        QString item = QString::fromUtf8("%1 (%2) - %3 - %4").arg(QLocale::languageToString(locale.language()), QLocale::countryToString(locale.country()), locale.name(), lang);
        ui->cmbTranslation->addItem(item, locale.name());
    }

    ui->pteComment->setPlainText(langs + QLatin1Char('\n') + locales);

    int index = ui->cmbTranslation->findData(QLocale().name());
    if (index < 0) {
        index = ui->cmbTranslation->findData(QStringLiteral("en_US"));
    }
    ui->cmbTranslation->setCurrentIndex(index);
    ui->cmbTranslation->show();
    connect(ui->cmbTranslation, SIGNAL(currentIndexChanged(int)), this, SLOT(changeTranslation(int)));
}

void MainWindow::changeTranslation(int index)
{
    if (_translator) {
        qApp->removeTranslator(_translator);
        qApp->removeTranslator(_translatorQt);
    }
    else {
        _translator = new QTranslator(this);
        _translatorQt = new QTranslator(this);
    }

    QLocale locale = index < 0 ? QLocale() : QLocale(ui->cmbTranslation->itemData(index).toString());
    if (_translator->load(locale, QStringLiteral("torrentfileeditor"), QStringLiteral("_"), QStringLiteral(":/translations"))) {
        qApp->installTranslator(_translator);
    }

#ifdef Q_OS_WIN
    QString qtTranslationsPath(QStringLiteral(":/translations"));
#else
    QString qtTranslationsPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#endif

    QString qtTranslationsName(QStringLiteral("qt"));

    if (_translatorQt->load(locale, qtTranslationsName, QStringLiteral("_"), qtTranslationsPath)) {
        qApp->installTranslator(_translatorQt);
    }
}

void MainWindow::create()
{
    if (!showNeedSaveFile())
        return;

    _bencodeModel->setRaw("");
    _bencodeModel->resetModified();
    _fileName = QString(); // -V815 PVS-Studio
    updateTitle();
    updateTab(ui->tabWidget->currentIndex());

    // Files tab
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->viewFiles->model());
    model->removeRows(0, model->rowCount());

    ui->leBaseFolder->setText(QString());
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

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open"), _torrentLastFolder, _formatFilters.join(QStringLiteral(";;")));

    if (fileName.isEmpty())
        return;

    _torrentLastFolder = fileName.section(QLatin1Char('/'), 0, -2);
    open(fileName);
}

void MainWindow::save()
{
    if (_fileName.isEmpty())
        saveAs();
    else
        saveTo(_fileName);
}

void MainWindow::saveAs()
{
    if (!_bencodeModel->isValid()) {
        QMessageBox::warning(this, tr("Can't save file"), tr("BEncoded data is not valid"));
        return;
    }

    QString filter;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"), _torrentLastFolder, _formatFilters.join(QStringLiteral(";;")), &filter);
    if (fileName.isEmpty())
        return;

    _torrentLastFolder = fileName.section(QLatin1Char('/'), 0, -2);

    if (_formatFilters.at(0) == filter && !fileName.endsWith(QLatin1String(".torrent")))
        fileName += QLatin1String(".torrent");
    else if (_formatFilters.at(1) == filter && !fileName.endsWith(QLatin1String(".dat")))
        fileName += QLatin1String(".dat");

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

void MainWindow::copyMagnetLink()
{
    QString link = ui->leMagnetLink->text();
    if (!link.isEmpty()) {
        qApp->clipboard()->setText(link);
    }
}

void MainWindow::copyMagnetExtra()
{
    QString name = _bencodeModel->name();
    qulonglong size = _bencodeModel->totalSize();
    QString link = ui->leMagnetLink->text();
    if (size && !name.isEmpty() && !link.isEmpty()) {
        QString str = QString(QLatin1String("%1\t%2\t%3")).arg(name, smartSize(size), link);
        qApp->clipboard()->setText(str);
    }
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

void MainWindow::dropEvent(QDropEvent *event) // -V2009 PVS-Studio
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (!urls.isEmpty() && urls.first().isLocalFile()) {
        QString path = urls.first().toLocalFile();
#ifdef Q_OS_WIN
        QFileInfo fi(path);
        if (fi.isSymLink()) {
            path = fi.symLinkTarget();
        }
#endif
        if (showNeedSaveFile()) {
            open(path);
        }
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (showNeedSaveFile())
        event->accept();
    else
        event->ignore();
}

void MainWindow::changeEvent(QEvent *event)
{
    switch(event->type()) {
    case QEvent::LanguageChange:
        retranslateUi();
        break;

    default:
        break;
    }

    QMainWindow::changeEvent(event);
}

// Token from qmmp
void MainWindow::fillCoding()
{
    QMap<QString, QTextCodec*> codecMap;
    QRegExp iso8859RegExp(QStringLiteral("ISO[- ]8859-([0-9]+).*"));

    for (int mib: QTextCodec::availableMibs())
    {
        QTextCodec *codec = QTextCodec::codecForMib(mib);

        QString sortKey = QString::fromUtf8(codec->name().toUpper());
        int rank;

        if (sortKey.startsWith(QLatin1String("UTF-8"))) {
            rank = 1; // -V112 PVS-Studio
        }
        else if (sortKey.startsWith(QLatin1String("UTF-16"))) {
            rank = 2; // -V112 PVS-Studio
        }
        else if (iso8859RegExp.exactMatch(sortKey)) {
            if (iso8859RegExp.cap(1).size() == 1) {
                rank = 3; // -V112 PVS-Studio
            }
            else {
                rank = 4; // -V112 PVS-Studio
            }
        }
        else {
            rank = 5; // -V112 PVS-Studio
        }
        sortKey.prepend(QChar('0' + rank));
        codecMap.insert(sortKey, codec);
    }

    foreach (QTextCodec *textCodec, codecMap.values()) {
        ui->cmbCoding->addItem(QString::fromUtf8(textCodec->name()));
    }
}

bool MainWindow::isModified() const
{
    return _bencodeModel->isModified();
}

void MainWindow::updateTitle()
{
    if (_fileName.isEmpty()) {
        setWindowTitle(qApp->applicationName());
    }
    else if (isModified()) {
        setWindowTitle(QStringLiteral("* %2 - %1").arg(qApp->applicationName(), QDir::toNativeSeparators(_fileName)));
    }
    else {
        setWindowTitle(QStringLiteral("%2 - %1").arg(qApp->applicationName(), QDir::toNativeSeparators(_fileName)));
    }
}

void MainWindow::updateTab(int n)
{
    switch (n) {
    case SimpleTab:
        updateSimple();
        break;

    case FilesTab:
        ui->viewFiles->setFocus();
        break;

    case JsonTreeTab:
        ui->treeJson->setFocus();
        ui->treeJson->expand(ui->treeJson->model()->index(0, 0));
        break;

    case RawTab:
        updateRaw();
        ui->pteEditor->setFocus();
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
    ui->leMagnetLink->setText(_bencodeModel->magnetLink());
}

void MainWindow::updateBencodeFromComment()
{
    _bencodeModel->setComment(ui->pteComment->toPlainText());
}

void MainWindow::updateBencodeFromTrackers()
{
    QStringList trackers = ui->pteTrackers->toPlainText().trimmed().split(QStringLiteral("\n"));

    if (trackers == _bencodeModel->trackers()) {
        return;
    }

    _bencodeModel->setTrackers(trackers);
    ui->leMagnetLink->setText(_bencodeModel->magnetLink());
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
    QDir baseDir(ui->leBaseFolder->text());

    // Check for relative paths
    bool hasRelative = false;

    do {
        hasRelative = false;
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
                baseDir = QDir(ui->leBaseFolder->text());
            }
            else {
                break;
            }
        }
    } while(hasRelative);


    if (hasRelative)
        return;

    if (model->rowCount() > 1) {
        if (baseDir.isRoot()) {
            QMessageBox::warning(this, tr("Warning"), tr("The filesystem root can't be used as a torrent root folder."));
            return;
        }
        else if (ui->leBaseFolder->text().isEmpty()) {
            QMessageBox::warning(this, tr("Warning"), tr("The torrent root folder is not set."));
            return;
        }
    }

    // Check for common origin folder and calculate total torrent size
    for (int i = 0; i < model->rowCount(); ++i) {
        QString file = model->item(i)->text();

        if (model->rowCount() > 1 && baseDir.relativeFilePath(file).startsWith(QLatin1String(".."))) {
            QMessageBox::warning(this, tr("Warning"), tr("The torrent root folder is not common."));
            return;
        }

        // Check for base folder
        files << QDir::fromNativeSeparators(file);
        totalSize += fileSize(file);
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
        filePairs << QPair<QString, qlonglong>(QString(), totalSize);
    }
    else {
        for (const QString &file: files) {
            filePairs << QPair<QString, qlonglong>(baseDir.relativeFilePath(file), fileSize(file));
        }
    }
    _bencodeModel->setFiles(filePairs);
}

void MainWindow::addFile()
{
    // Will believe that it's very rare case when need to add symlink.
    // Native dialog looks very nice. So use it.

//#ifdef Q_OS_WIN
//    // On Windows symbolic link is real file. So use it.
//    // Also native dialog always returns resolved path. So use Qt dialog.
//    QStringList files = QFileDialog::getOpenFileNames(this, tr("Add File"), _lastFolder, QString(), nullptr, QFileDialog::DontResolveSymlinks | QFileDialog::DontUseNativeDialog);
//#else
    QStringList files = QFileDialog::getOpenFileNames(this, tr("Add File"), _lastFolder, QString());
//#endif
    if (files.isEmpty())
        return;

    _lastFolder = QFileInfo(files.first()).absolutePath();
    ui->leBaseFolder->setFolder(_lastFolder);
    foreach (const QString &file, files) {
        addFilesRow(file, fileSize(file));
    }

    if (ui->leBaseFolder->text().isEmpty()) {
        ui->leBaseFolder->setText(QDir::toNativeSeparators(files.first().section(QLatin1Char('/'), 0, -2)));
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
    ui->leBaseFolder->setFolder(QDir::toNativeSeparators(_lastFolder));
    QDirIterator it(path, QDirIterator::Subdirectories);

    QStringList files;

    while (it.hasNext()) {
        it.next();
        QFileInfo fileInfo = it.fileInfo();

        if (fileInfo.isFile())
            files << fileInfo.absoluteFilePath();
    }

    files.sort();

    for (const QString &file: files) {
        addFilesRow(file, fileSize(file));
    }

    if (ui->leBaseFolder->text().isEmpty())
        ui->leBaseFolder->setText(QDir::toNativeSeparators(path));

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

void MainWindow::reloadFiles()
{
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->viewFiles->model());
    model->removeRows(0, model->rowCount());
    updateFiles();
}

void MainWindow::updateFiles()
{
    QList<QPair<QString, qlonglong>> files = _bencodeModel->files();
    if (files.isEmpty())
        return;

    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->viewFiles->model());

    // Fill files from _bencode if empty
    if (!model->rowCount()) {
        for (const auto &file: files) {
            addFilesRow(file.first, file.second);
        }
        qulonglong pieceSize = _bencodeModel->pieceSize();
        for (int i = 0; i < ui->cmbPieceSizes->count(); i++) {
            if (pieceSize == ui->cmbPieceSizes->itemData(i).toULongLong())
                ui->cmbPieceSizes->setCurrentIndex(i);
        }
    }

    QString dir = ui->leBaseFolder->text();
    if (!QDir(dir).exists())
        return;

    _lastFolder = dir;

    // Try to find files on disk and set full path if exists
    for (int i = 0; i < model->rowCount(); ++i) {
        QString file = model->item(i)->text();
        if (QFileInfo(file).isAbsolute())
            continue;

        file.prepend(QLatin1String("/"));
        file.prepend(dir);
        if (QFile::exists(file))
            model->item(i)->setText(QDir::toNativeSeparators(file));
    }

    updateFilesSize();
    ui->viewFiles->scrollToTop();
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
    QModelIndex currentIndex = ui->treeJson->currentIndex();
    if (!currentIndex.isValid() || !currentIndex.parent().isValid()) {
        return;
    }

    int row = -1;
    QModelIndex parent;
    if (ui->treeJson->selectionModel()->selectedRows().size() == 1) {
        row = currentIndex.row();
        parent = currentIndex.parent();
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
        ui->treeJson->setCurrentIndex(_bencodeModel->index(row, 0, parent));
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
    ui->lePieceSize->setText(pieceSize ? smartSize(pieceSize) : QStringLiteral("0"));

    int pieces = _bencodeModel->pieces();
    ui->lePieces->setText(pieces ? QLocale::system().toString(pieces) : QString());

    qulonglong totalSize = _bencodeModel->totalSize();
    ui->leTorrentTotalSize->setText(totalSize ? smartSize(totalSize) : QStringLiteral("0"));

    ui->dateCreated->setDateTime(_bencodeModel->creationTime());
    ui->chkPrivateTorrent->setChecked(_bencodeModel->privateTorrent());
    ui->pteTrackers->setPlainText(_bencodeModel->trackers().join(QStringLiteral("\n")));
    ui->leHash->setText(_bencodeModel->hash());
    ui->leMagnetLink->setText(_bencodeModel->magnetLink());
}

void MainWindow::updateBencodeFromRaw()
{
    if (!ui->pteEditor->document()->isModified())
        return;

    // Special case when no any text
    if (ui->pteEditor->toPlainText().trimmed().isEmpty()) {
        ui->lblRawError->setText(QString());
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

    ui->lblRawError->setText(QString());
    _bencodeModel->setJson(variant);
}

void MainWindow::updateRaw()
{
    // Avoid freezes
    processEvents();
    QVariant res = _bencodeModel->toJson();
    if (!res.isValid()) {
        ui->pteEditor->setPlainText(QString());
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
    QString newRawText = QString::fromLatin1(ba);
    if (newRawText != ui->pteEditor->toPlainText()) {
        ui->pteEditor->setPlainText(newRawText);
        ui->pteEditor->document()->setModified(false);
    }
}

bool MainWindow::saveTo(const QString &fileName)
{
    if (!_bencodeModel->isValid()) {
        QMessageBox::warning(this, tr("Can't save file"), tr("BEncoded data is not valid"));
        return false;
    }

    QByteArray raw = _bencodeModel->toRaw();
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        if (file.write(raw) == -1) {
            QMessageBox::warning(this, tr("Can't save file"), file.errorString());
            return false;
        }
        else {
            file.close();
            _bencodeModel->resetModified();
            updateTitle();
            return true;
        }
    }
    else {
        QMessageBox::warning(this, tr("Can't save file"), file.errorString());
        return false;
    }
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
    list.last()->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter); // -V807 PVS-Studio
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

    QString res = QLocale::system().toString(kb, 'g', 4); // -V112 PVS-Studio

    // Drop zeroes
    while (res.contains(QLocale::system().decimalPoint()) && res.right(1) == QLatin1String("0"))
        res.chop(1);

    if (res.right(1)[0] == QLocale::system().decimalPoint())
        res.chop(1);

    switch (i) {
        case 0:
            res += QStringLiteral(" ") + tr("B");
            break;

        case 1:
            res += QStringLiteral(" ") + tr("KiB");
            break;

        case 2:
            res += QStringLiteral(" ") + tr("MiB");
            break;

        case 3:
            res += QStringLiteral(" ") + tr("GiB");
            break;

        case 4:
            res += QStringLiteral(" ") + tr("TiB");
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

void MainWindow::retranslateUi()
{
    ui->retranslateUi(this);

    ui->cmbPieceSizes->setItemText(0, tr("Auto"));

    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->viewFiles->model());
    QStringList headers;
    headers << tr("Path") << tr("Size") << tr("# Pieces") << QString() /* dummy */;
    model->setHorizontalHeaderLabels(headers);
}
