// SPDX-FileCopyrightText: 2016-2020, 2026 Ivan Romanov <drizt72@zoho.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QTableView>

class TableView : public QTableView
{
    Q_OBJECT

public:
    TableView(QWidget *parent = 0);
    QByteArray GetFileNameAndSize() const;

signals:
    void deleteRow();

private slots:
    void copy();
    void copyWithSize();
    void copyWoExt();

protected:
    QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) override;
    void keyPressEvent(QKeyEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    void updateTranslations();

    QMenu *_menu{};
    QAction *_copyAct{};
    QAction *_copySizeAct{};
    QAction *_copyWoExtAct{};
};
