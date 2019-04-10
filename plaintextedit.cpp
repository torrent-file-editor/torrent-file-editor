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

#include "plaintextedit.h"

#include <QColor>
#include <QPainter>
#include <QTextBlock>

PlainTextEditNumber::PlainTextEditNumber(QWidget *editor)
    : QWidget(editor)
{
}

QSize PlainTextEditNumber::sizeHint() const
{
    PlainTextEdit *parent = qobject_cast<PlainTextEdit*>(parentWidget());

    int blockCount = parent->document()->blockCount();

    int digits = 1;
    while (blockCount >= 10) {
        digits++;
        blockCount /= 10;
    }

    int width = parent->fontMetrics().boundingRect(QString(digits, QChar::fromLatin1('0'))).width() + 8;

    return QSize(width, parent->viewport()->rect().height());
}

void PlainTextEditNumber::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    PlainTextEdit *parent = qobject_cast<PlainTextEdit*>(parentWidget());

    QPainter painter;
    painter.begin(this);

    painter.fillRect(rect(), parent->palette().base());
    painter.fillRect(rect().adjusted(0, 0, -3, 0), parent->bgColor());

    QTextBlock block = parent->firstVisibleBlock();
    int width = this->width();
    int height = parent->fontMetrics().height();
    int currentBlock = parent->textCursor().block().blockNumber() + 1;
    while (block.isVisible() && block.isValid()) {
        QRect blockRect = parent->blockBoundingGeometry(block).translated(parent->contentOffset()).toRect();
        QString lineNumber =  QString::number(block.blockNumber() + 1);
        int top = qMax(0, blockRect.top());
        if (currentBlock == lineNumber.toInt()) {
            painter.fillRect(0, top, width, height, parent->higlightColor());
        }
        painter.drawText(0, top, width - 5, height, Qt::AlignRight | Qt::AlignBottom, lineNumber);

        block = block.next();
    }
    painter.end();
}

PlainTextEdit::PlainTextEdit(QWidget *parent)
    : QPlainTextEdit(parent)
    , _numberWidget(new PlainTextEditNumber(this))
{
    connect(document(), SIGNAL(blockCountChanged(int)), this, SLOT(updateWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), _numberWidget, SLOT(update()));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

    document()->setDocumentMargin(0.);
    _numberWidget->move(viewport()->geometry().topLeft());
}

void PlainTextEdit::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);
    _numberWidget->resize(_numberWidget->sizeHint());
}

void PlainTextEdit::updateWidth(int blockCount)
{
    Q_UNUSED(blockCount);

    QSize size = _numberWidget->sizeHint();
    setViewportMargins(size.width(), 0, 0, 0);
    _numberWidget->resize(size);
}

void PlainTextEdit::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    QTextEdit::ExtraSelection selection;

    selection.format.setBackground(higlightColor());
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();
    extraSelections.append(selection);

    setExtraSelections(extraSelections);

}

QColor PlainTextEdit::higlightColor() const
{
    static QColor color;
    if (!color.isValid()) {
        color = Qt::GlobalColor::cyan;
        if (palette().text().color().value() > 150) {
            color = color.darker().darker();
        }
        else {
            color = color.lighter().lighter(120);
        }
    }

    return color;
}

QColor PlainTextEdit::bgColor() const
{
    static QColor color;
    if (!color.isValid()) {
        color = Qt::GlobalColor::green;
        if (palette().text().color().value() > 150) {
            color = color.darker().darker();
        }
        else {
            color = color.lighter().lighter(120);
        }
    }

    return color;
}
