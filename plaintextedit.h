/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 *
 * Copyright (C) 2019  Ivan Romanov <drizt72@zoho.eu>
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

#include <QPlainTextEdit>
#include <QObject>

class PlainTextEditNumber : public QWidget
{
    Q_OBJECT

public:
    PlainTextEditNumber(QWidget *editor);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
};


class PlainTextEdit : public QPlainTextEdit
{
    Q_OBJECT

public:
    PlainTextEdit(QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateWidth(int blockCount);
    void highlightCurrentLine();

private:
    QColor higlightColor() const;
    QColor bgColor() const;

    PlainTextEditNumber *_numberWidget;

    friend class PlainTextEditNumber;
};

