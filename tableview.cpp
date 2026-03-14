// SPDX-FileCopyrightText: 2016-2020, 2023, 2026 Ivan Romanov <drizt72@zoho.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tableview.h"

#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QItemSelectionModel>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMenu>

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
#include <QStyleHints>
#define ACCEL_KEY(k)                                                                                                   \
    ((!QCoreApplication::testAttribute(Qt::AA_DontShowIconsInMenus)                                                    \
      && QGuiApplication::styleHints()->showShortcutsInContextMenus())                                                 \
             && !QKeySequence(k).toString(QKeySequence::NativeText).isEmpty()                                          \
         ? QLatin1Char('\t') + QKeySequence(k).toString(QKeySequence::NativeText)                                      \
         : QString())
#else
#define ACCEL_KEY(k)                                                                                                   \
    (!QCoreApplication::testAttribute(Qt::AA_DontShowIconsInMenus)                                                     \
             && !QKeySequence(k).toString(QKeySequence::NativeText).isEmpty()                                          \
         ? QLatin1Char('\t') + QKeySequence(k).toString(QKeySequence::NativeText)                                      \
         : QString())
#endif

TableView::TableView(QWidget *parent)
    : QTableView(parent)
    , _menu(new QMenu(this))
{
    setContextMenuPolicy(Qt::DefaultContextMenu);

    _copyAct = new QAction(this);
    _copyAct->setIcon(QIcon::fromTheme(QLatin1String("edit-copy")));
    _copyAct->setShortcut(QKeySequence::Copy);
    connect(_copyAct, SIGNAL(triggered()), SLOT(copy()));
    _menu->addAction(_copyAct);
    addAction(_copyAct);

    _copySizeAct = new QAction(tr("Copy with Size"), this);
    _copySizeAct->setShortcut(QKeySequence(Qt::ShiftModifier | Qt::ControlModifier | Qt::Key_C));
    connect(_copySizeAct, SIGNAL(triggered()), SLOT(copyWithSize()));
    _menu->addAction(_copySizeAct);
    addAction(_copySizeAct);

    _copyWoExtAct = new QAction(tr("Copy Filename"), this);
    connect(_copyWoExtAct, SIGNAL(triggered()), SLOT(copyWoExt()));
    _menu->addAction(_copyWoExtAct);
    addAction(_copyWoExtAct);

    updateTranslations();
}

void TableView::copy()
{
    QItemSelectionModel *selection = selectionModel();
    QModelIndexList rows = selection->selectedRows();
    QStringList files;
    for (const auto &row : rows) {
        files << row.data().toString();
    }

    if (!files.isEmpty()) {
        QApplication::clipboard()->setText(files.join(QStringLiteral("\n")));
    }
}

void TableView::copyWithSize()
{
    QItemSelectionModel *selection = selectionModel();
    QModelIndexList nameRows = selection->selectedRows(0);
    QModelIndexList sizeRows = selection->selectedRows(1);
    QStringList files;

    for (int i = 0; i < nameRows.length(); ++i) {
        files << nameRows.at(i).data().toString() + QLatin1String("\t") + sizeRows.at(i).data().toString();
    }

    if (!files.isEmpty()) {
        QApplication::clipboard()->setText(files.join(QStringLiteral("\n")));
    }
}

void TableView::copyWoExt()
{
    QItemSelectionModel *selection = selectionModel();
    QModelIndexList nameRows = selection->selectedRows(0);
    QStringList files;

    for (int i = 0; i < nameRows.length(); ++i) {
        QString filename = nameRows.at(i).data().toString();

        if (filename.contains(QDir::separator())) {
            filename = filename.section(QDir::separator(), -1);
        }

        if (filename.contains(QLatin1String("."))) {
            filename = filename.section(QLatin1String("."), 0, -2);
        }
        files << filename;
    }

    if (!files.isEmpty()) {
        QApplication::clipboard()->setText(files.join(QStringLiteral("\n")));
    }
}

QModelIndex TableView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    switch (cursorAction) {
    case QAbstractItemView::MoveHome:
        return model()->index(0, 0);
        break;

    case QAbstractItemView::MoveEnd:
        return model()->index(model()->rowCount() - 1, 0);
        break;

    default:
        return QTableView::moveCursor(cursorAction, modifiers);
        break;
    }
}

void TableView::keyPressEvent(QKeyEvent *event)
{
    if (event == QKeySequence::Delete) {
        emit deleteRow();
    } else {
        QTableView::keyPressEvent(event);
    }
}

void TableView::contextMenuEvent(QContextMenuEvent *event)
{
    bool hasSelection = selectionModel()->hasSelection();
    _copyAct->setEnabled(hasSelection);
    _copySizeAct->setEnabled(hasSelection);
    _copyWoExtAct->setEnabled(hasSelection);

    _menu->exec(event->globalPos());
}

void TableView::changeEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::LanguageChange:
        updateTranslations();
        break;

    default:
        break;
    }

    QTableView::changeEvent(event);
}

void TableView::updateTranslations()
{
    _copyAct->setText(tr("Copy") + ACCEL_KEY(QKeySequence::Copy));
    _copySizeAct->setText(tr("Copy with Size"));
    _copyWoExtAct->setText(tr("Copy Filename"));
}
