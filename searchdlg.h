/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 *
 * Copyright (C) 2016-2017  Ivan Romanov <drizt72@zoho.eu>
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

#include <QDialog>
#include <QModelIndex>

class BencodeModel;

#ifdef Q_OS_MAC
class QSizeGrip;
#endif

namespace Ui { class SearchDlg; }

class SearchDlg : public QDialog
{
    Q_OBJECT

public:
    explicit SearchDlg(BencodeModel *model, QWidget *parent = 0);
    ~SearchDlg();
    void setReplaceModeEnabled(bool b);

public slots:
    void searchNext();
    void updateSearchNext();
    void resetSearchList();
    void replace();
    void replaceAll();

signals:
    void foundItem(const QModelIndex &index);

#ifdef Q_OS_MAC
protected:
    void resizeEvent(QResizeEvent *event);
#endif

private:
#ifdef Q_OS_MAC
    void updateSizeGripPos();
#endif
    Ui::SearchDlg *ui;
    QModelIndexList _searchList;
    int _searchIndex;
    BencodeModel *_model;
#ifdef Q_OS_MAC
    QSizeGrip *_sizeGrip;
#endif
};
