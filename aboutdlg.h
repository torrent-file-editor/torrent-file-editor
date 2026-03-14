// SPDX-FileCopyrightText: 2014, 2017-2018, 2026 Ivan Romanov <drizt72@zoho.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDialog>

class QThread;

namespace Ui {
class AboutDlg;
}

class AboutDlg : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDlg(QWidget *parent = 0);
    ~AboutDlg();

private slots:
    void checkUpdate();
    void showUpdate(const QString &version, const QString &url);
    void aboutQt();

private:
    Ui::AboutDlg *ui;
    QThread *thread;
};
