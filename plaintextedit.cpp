// SPDX-FileCopyrightText: 2019, 2026 Ivan Romanov <drizt72@zoho.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

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
    PlainTextEdit *parent = qobject_cast<PlainTextEdit *>(parentWidget());

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

    PlainTextEdit *parent = qobject_cast<PlainTextEdit *>(parentWidget());

    QPainter painter;
    painter.begin(this);

    painter.fillRect(rect(), parent->palette().base());
    painter.fillRect(rect().adjusted(0, 0, -3, 0), parent->bgColor());

    QTextBlock block = parent->firstVisibleBlock();
    int width = this->width();
    int height = parent->fontMetrics().height();
    int currentBlock = parent->textCursor().block().blockNumber() + 1;
    QPointF contentOffset = parent->contentOffset();
    int widgetHeight = this->height();
    while (block.isVisible() && block.isValid()) {
        QRect blockRect = parent->blockBoundingGeometry(block).translated(contentOffset).toRect();

        if (blockRect.top() > widgetHeight) {
            break;
        }

        QString lineNumber = QString::number(block.blockNumber() + 1);
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
    connect(this,
            SIGNAL(updateRequest(QRect, int)),
            _numberWidget,
            SLOT(update())); // clazy:exclude=connect-not-normalized
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

    document()->setDocumentMargin(0.);
    _numberWidget->move(viewport()->geometry().topLeft());
}

void PlainTextEdit::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);
    _numberWidget->resize(_numberWidget->sizeHint());
    updateWidth(0);
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
        } else {
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
        } else {
            color = color.lighter().lighter(120);
        }
    }

    return color;
}
