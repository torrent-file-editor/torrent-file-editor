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

#include "datewidget.h"

#include <QApplication>
#include <QCalendarWidget>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeyEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <QLocale>
#include <QPushButton>

// Always use format of current locale
inline QString dateFormat()
{
    QString format = QLocale().dateFormat(QLocale::ShortFormat) + QStringLiteral(" ") + QLocale().timeFormat(QLocale::LongFormat);
    if (format.endsWith(QLatin1String(" t")))
        format.chop(2);

    // Use the year as four digit number
    if (format.count(QLatin1Char('y')) == 2) {
        format.replace(QLatin1Char('y'), QLatin1String("yy"));
    }
    return format;
}

DateWidget::DateWidget(QWidget *parent)
    : LineEditWidget(parent)
    , _tbCalendar(new QPushButton(this))
    , _tbClean(new QPushButton(this))
    , _calendar(new QCalendarWidget(this))
{
    _tbClean->setObjectName(QStringLiteral("brClear"));
    _tbClean->setIcon(QIcon::fromTheme(QStringLiteral("edit-clear")));
    _tbClean->setContentsMargins(0, 0, 0, 0);
    _tbClean->setFocusPolicy(Qt::NoFocus);
    _tbClean->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    _tbClean->setAutoFillBackground(true);
    _tbClean->setCursor(Qt::PointingHandCursor);
    _tbClean->resize(0, 0);
    _tbClean->setFlat(true);
    _tbClean->setMinimumWidth(24);
    _tbClean->setMaximumWidth(24);
    addWidget(_tbClean);

    _tbCalendar->setObjectName(QStringLiteral("tbCalendar"));
    _tbCalendar->setIcon(QIcon::fromTheme(QStringLiteral("x-office-calendar")));
    _tbCalendar->setContentsMargins(0, 0, 0, 0);
    _tbCalendar->setFocusPolicy(Qt::NoFocus);
    _tbCalendar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    _tbCalendar->setAutoFillBackground(true);
    _tbCalendar->setCursor(QCursor(Qt::ArrowCursor));
    _tbCalendar->resize(0, 0);
    _tbCalendar->setFlat(true);
    _tbCalendar->setCursor(Qt::PointingHandCursor);
    _tbCalendar->setMinimumWidth(24);
    _tbCalendar->setMaximumWidth(24);
    addWidget(_tbCalendar);

    setPopup(_calendar);

    connect(_calendar, SIGNAL(clicked(const QDate&)), SLOT(closeCalendar(const QDate&)));
    connect(_tbCalendar, SIGNAL(clicked()), SLOT(showPopup()));
    connect(_tbCalendar, SIGNAL(clicked()), SLOT(calendarSetDate()));

    connect(_tbClean, SIGNAL(clicked()), SLOT(internalClear()));

    setOptimalLength(QDateTime(QDate(2000, 12, 12), QTime(10, 10, 10)).toString(dateFormat()).size());

    retranslateUi();
}

void DateWidget::setDate(const QDate &date)
{
    QDateTime dateTime = this->dateTime();
    dateTime.setDate(date);
    setDateTime(dateTime);
}

QDate DateWidget::date() const
{
    return dateTime().date();
}

void DateWidget::setTime(const QTime &time)
{
    QDateTime dateTime = this->dateTime();
    dateTime.setTime(time);
    setDateTime(dateTime);
}

QTime DateWidget::time() const
{
    return dateTime().time();
}

void DateWidget::setDateTime(const QDateTime &dateTime)
{
    if (dateTime.isValid())
        setText(dateTime.toString(dateFormat()));
    else
        clear();
}

QDateTime DateWidget::dateTime() const
{
    return QDateTime::fromString(text(), dateFormat());
}

void DateWidget::closeCalendar(const QDate &date)
{
    setDate(date);
    hidePopup();
    emit textEdited(text());
}

void DateWidget::calendarSetDate()
{
    if(date().isValid()) {
        _calendar->setSelectedDate(date());
    }
}

void DateWidget::internalClear()
{
    clear();
    emit textEdited(QString());
}

void DateWidget::retranslateUi()
{
    _tbClean->setToolTip(tr("Clean"));
    _tbCalendar->setToolTip(tr("Show calendar"));
}
