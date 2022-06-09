#include "plain_text_edit.h"
#include <QDebug>
#include <QMimeData>
#include <QApplication>
#include <QRegularExpression>
#include <QScrollBar>
#include <QTextBlock>
#include <iostream>
#include "line_number_area.h"
#include "plog/Log.h"

PlainTextEdit::PlainTextEdit(QWidget *parent) :
    QPlainTextEdit(parent),
    lineNumberArea(new LineNumberArea(this))
{
    installEventFilter(this);
    connect(document(), &QTextDocument::blockCountChanged, this, &PlainTextEdit::updateLineNumberAreaWidth);
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &PlainTextEdit::updateLineNumberArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &PlainTextEdit::updateLineNumberArea);
}

void PlainTextEdit::updateLineNumberArea()
{
    lineNumberArea->update();
}

int PlainTextEdit::getFirstVisibleBlock() const
{
    return firstVisibleBlock().blockNumber();
}

int PlainTextEdit::getContentOffsetY() const
{
    return contentOffset().y();
}

void PlainTextEdit::dragEnterEvent(QDragEnterEvent *event)
{
    if(!event->mimeData()->hasUrls())
    {
        QPlainTextEdit::dragEnterEvent(event);
        return;
    }

    event->acceptProposedAction();
}

void PlainTextEdit::dragMoveEvent(QDragMoveEvent *event)
{
    if(!event->mimeData()->hasUrls())
    {
        QPlainTextEdit::dragMoveEvent(event);
        return;
    }

    event->acceptProposedAction();
}

void PlainTextEdit::dropEvent(QDropEvent *event)
{
    if(!event->mimeData()->hasUrls() || event->mimeData()->urls().size() < 1)
    {
        QPlainTextEdit::dropEvent(event);
        return;
    }

    QUrl url = event->mimeData()->urls().at(0);
    urlDropped(url);
}

void PlainTextEdit::leaveEvent(QEvent *event)
{
    leaveControl();
    super::leaveEvent(event);
}
void PlainTextEdit::enterEvent(QEvent *event)
{
    enterControl();
    super::enterEvent(event);
}

bool PlainTextEdit::eventFilter(QObject *, QEvent *event)
{
    // I need to give credit to QOwnNotes project for the code below
    // https://github.com/pbek/QOwnNotes


    // we need this strange newline character we are getting in the
    // selected text for newlines
    const QString newLine =
        QString::fromUtf8(QByteArray::fromHex(QByteArrayLiteral("e280a9")));
    const QString indentCharacters = "\t";

    if(event->type() != QEvent::KeyPress)
        return false;

    if(isReadOnly())
        return false;

    if(!this->hasFocus())
        return false;

    auto *keyEvent = static_cast<QKeyEvent *>(event);
    QTextCursor cursor = textCursor();
    QString selectedText = cursor.selectedText();
    if(selectedText.isEmpty())
        return false;

    QString newText;
    if (keyEvent->key() == Qt::Key_Tab)
    {
        // replace trailing new line to prevent an indent of the line after
        // the selection
        newText = selectedText.replace(
            QRegularExpression(QRegularExpression::escape(newLine) +
                               QStringLiteral("$")),
            QStringLiteral("\n"));

        // indent text
        newText.replace(newLine, QStringLiteral("\n") + indentCharacters)
            .prepend(indentCharacters);

        // remove trailing \t
        newText.remove(QRegularExpression(QStringLiteral("\\t$")));
    }
    else if(keyEvent->key() == Qt::Key_Backtab)
    {
        const int indentSize = 1;
        // remove leading \t or spaces in following lines
        newText = selectedText.replace(
                QRegularExpression(newLine + QStringLiteral("(\\t| {1,") +
                    QString::number(indentSize) +
                    QStringLiteral("})")),
                QStringLiteral("\n"));

        // remove leading \t or spaces in first line
        newText.remove(QRegularExpression(QStringLiteral("^(\\t| {1,") +
                    QString::number(indentSize) +
                    QStringLiteral("})")));
    }
    else
    {
        return false;
    }
    // insert the new text
    cursor.insertText(newText);
    //
    // update the selection to the new text
    cursor.setPosition(cursor.position() - newText.size(),
                       QTextCursor::KeepAnchor);
    setTextCursor(cursor);
    return true;
}

void PlainTextEdit::updateLineNumberAreaWidth(int)
{
    setViewportMargins(lineNumberArea->sizeHint().width(), 0, 0, 0);
}

void PlainTextEdit::paintEvent(QPaintEvent *event)
{
    QPlainTextEdit::paintEvent(event);
    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberArea->sizeHint().width(), cr.height()));
    if (event->rect().contains(viewport()->rect()))
    {
        updateLineNumberAreaWidth(0);
    }
}
