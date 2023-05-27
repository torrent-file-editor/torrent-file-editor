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

#include <QLineEdit>
#include <QToolButton>
#include <QList>

class QFrame;
class QHBoxLayout;

class LineEditWidget : public QLineEdit
{
    Q_OBJECT
    Q_PROPERTY(int optimalLength READ optimalLenth WRITE setOptimalLength)
    Q_PROPERTY(QString rxValidator READ rxValidator WRITE setRxValidator)
public:
    explicit LineEditWidget(QWidget *parent = 0);
    ~LineEditWidget();

    // reimplemented
    QSize sizeHint() const override;
    void showEvent(QShowEvent *e) override;
    bool eventFilter(QObject *o, QEvent *e) override;

    // Properties
    int optimalLenth() const { return _optimalLength; }
    void setOptimalLength(int optimalLength) { _optimalLength = optimalLength; }

    QString rxValidator() const { return _rxValidator; }
    void setRxValidator(const QString &str);

protected:
    // reimplemented
    void changeEvent(QEvent *event) override;

    void addWidget(QWidget *w);
    void setPopup(QWidget* w);
    QFrame *popup() const { return _popup; }
    virtual void retranslateUi() {}

protected slots:
    virtual void showPopup();
    virtual void hidePopup();

private:

    QHBoxLayout *_layout;
    QList<QWidget*> _toolbuttons;
    QFrame *_popup;

    // Properties
    int _optimalLength;
    QString _rxValidator;
};
