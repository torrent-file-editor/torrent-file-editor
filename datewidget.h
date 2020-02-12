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

#pragma once

#include "lineeditwidget.h"

#include <QDateTime>

class QPushButton;
class QCalendarWidget;

class DateWidget : public LineEditWidget
{
    Q_OBJECT

    Q_PROPERTY(QDate date
               READ date
               WRITE setDate)

    Q_PROPERTY(QTime time
               READ time
               WRITE setTime)

    Q_PROPERTY(QDateTime dateTime
               READ dateTime
               WRITE setDateTime)

public:
    explicit DateWidget(QWidget *parent = 0);

    // get/set date
    void setDate(const QDate &date);
    QDate date() const;

    // get/set time
    void setTime(const QTime &time);
    QTime time() const;

    // get/set dateTime
    void setDateTime(const QDateTime &dateTime);
    QDateTime dateTime() const;

protected slots:
    void closeCalendar(const QDate &text);
    void calendarSetDate();
    void internalClear();

protected:
    void retranslateUi() override;

private:

    // Inner widgets
    QPushButton *_tbCalendar;
    QPushButton *_tbClean;
    QCalendarWidget *_calendar;
};
