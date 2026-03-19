// SPDX-FileCopyrightText: 2019, 2026 Ivan Romanov <drizt72@zoho.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QPlainTextEdit>

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
    void showEvent(QShowEvent *event) override;

private slots:
    void updateWidth(int blockCount);
    void highlightCurrentLine();

private:
    QColor higlightColor() const;
    QColor bgColor() const;

    PlainTextEditNumber *_numberWidget;
    bool _numberWidgetInitialized;

    friend class PlainTextEditNumber;
};
