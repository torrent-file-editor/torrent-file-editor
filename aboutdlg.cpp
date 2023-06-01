/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 *
 * Copyright (C) 2014  Ivan Romanov <drizt72@zoho.eu>
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

#include "aboutdlg.h"
#include "ui_aboutdlg.h"
#include "application.h"

#include <QKeySequence>
#include <QLocale>
#include <QShortcut>

#ifdef Q_OS_WIN
# include "checkupdate.h"
# include <QThread>
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
# include <QRegularExpression>
#else
# include <QRegExp>
#endif

#define VERSION_LABEL                               \
    "<style>"                                       \
    "h2 {"                                          \
    "margin-bottom: 0;"                             \
    "}"                                             \
    "p {"                                           \
    "margin-top: 0;"                                \
    "}"                                             \
    "</style>"                                      \
    "<h2>%1</h2><p>%2 (%3)</p>"


struct Version {
    int major;
    int minor;
    int patch;
    int rev;
    QString gitHash;
    bool dirty;
};

Version parseVersion(const QString &version)
{
    Version ver{0, 0, 0, 0, QString(), false};
    QString part1 = version.section(QLatin1Char('-'), 0, 0);
    QString part2 = version.section(QLatin1Char('-'), 1);

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    QRegularExpression rx(QStringLiteral("^v?(\\d+)\\.(\\d+)\\.(\\d+)$"));
    QRegularExpressionMatch match = rx.match(part1);
    if (match.hasMatch()) {
        ver.major = match.captured(1).toInt();
        ver.minor = match.captured(2).toInt();
        ver.patch = match.captured(3).toInt();
    }

    rx.setPattern(QStringLiteral("^(\\d+)-g([0-9a-f]{7})(-dirty)?$"));
    match = rx.match(part2);
    if (match.hasMatch()) {
        ver.rev = match.captured(1).toInt();
        ver.gitHash = match.captured(2);
        ver.dirty = !match.captured(3).isEmpty();
    }
#else
    QRegExp rx(QStringLiteral("^v?(\\d+)\\.(\\d+)\\.(\\d+)$"));
    if (rx.indexIn(part1) != -1) {
        ver.major = rx.cap(1).toInt();
        ver.minor = rx.cap(2).toInt();
        ver.patch = rx.cap(3).toInt();
    }

    rx.setPattern(QStringLiteral("^(\\d+)-g([0-9a-f]{7})(-dirty)?$"));
    if (rx.indexIn(part2) != -1) {
        ver.rev = rx.cap(1).toInt();
        ver.gitHash = rx.cap(2);
        ver.dirty = !rx.cap(3).isEmpty();
    }
#endif
    return ver;
}

bool versionLessThan(const Version &left, const Version &right)
{
    if (left.major < right.major) {
        return true;
    }
    else if (left.major > right.major) {
        return false;
    }

    if (left.minor < right.minor) {
        return true;
    }
    else if (left.minor > right.minor) {
        return false;
    }

    if (left.patch < right.patch) {
        return true;
    }
    else if (left.patch > right.patch) {
        return false;
    }

    if (left.rev < right.rev) {
        return true;
    }
    else if (left.rev > right.rev) {
        return false;
    }

    if (left.gitHash == right.gitHash && left.dirty == right.dirty) {
        return false;
    }

    if (left.gitHash.isEmpty() && !right.gitHash.isEmpty()) {
        return true;
    }
    else if (!left.gitHash.isEmpty() && right.gitHash.isEmpty()) {
        return false;
    }

    if (left.gitHash == right.gitHash && !left.dirty && right.dirty) {
        return true;
    }

    return false;
}

AboutDlg::AboutDlg(QWidget *parent)
#ifdef Q_OS_WIN
    : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint)
#else
    : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
#endif
    , ui(new Ui::AboutDlg)
    , thread(nullptr)
{
    ui->setupUi(this);

    QString buildDate = QLocale::system().toString(Application::buildDateTime().date());

    ui->label->setText(QStringLiteral(VERSION_LABEL)
                       .arg(qApp->applicationName())
                       .arg(qApp->applicationVersion())
                       .arg(buildDate));
    setWindowTitle(QString(tr("About %1")).arg(qApp->applicationName()));

#ifdef NO_DONATION
    ui->wdgDonation->hide();
#endif
    ui->lbShowUpdate->hide();

#ifndef Q_OS_WIN
    ui->btCheckUpdate->hide();
#endif

    new QShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_Q), this, SLOT(aboutQt()));
}

AboutDlg::~AboutDlg()
{
#ifdef Q_OS_WIN
    if (thread) {
        thread->quit();
        thread->wait();
        delete thread;
    }
#endif
    delete ui;
}

void AboutDlg::checkUpdate()
{
#ifdef Q_OS_WIN
    ui->btCheckUpdate->setEnabled(false);

    thread = new QThread;
    CheckUpdate *cUpdate = new CheckUpdate;
    cUpdate->moveToThread(thread);
    connect(thread, SIGNAL(started()), cUpdate, SLOT(start()));
    connect(cUpdate, SIGNAL(finished(const QString&, const QString&)), SLOT(showUpdate(const QString&, const QString&)));
    thread->start();
#endif
}

void AboutDlg::showUpdate(const QString &version, const QString &url)
{
#ifdef Q_OS_WIN
    if (thread) {
        thread->quit();
        thread->wait();
        delete thread;
        thread = nullptr;
    }

    ui->btCheckUpdate->setEnabled(true);

    ui->lbShowUpdate->show();

    if (version.isEmpty()) {
        ui->lbShowUpdate->setText(tr("Something went wrong"));
    }
    else {
        if (versionLessThan(parseVersion(qApp->applicationVersion()), parseVersion(version))) {
            ui->lbShowUpdate->setText(QString(tr("New version <a href=\"%2\">%1</a> has been detected")).arg(version).arg(url));
        }
        else {
            ui->lbShowUpdate->setText(tr("The latest version is installed"));
        }
    }

#else
    Q_UNUSED(version);
    Q_UNUSED(url);
#endif
}

void AboutDlg::aboutQt()
{
    qApp->aboutQt();
}
