#include "epad_text_edit.h"
#include <QRegularExpression>

EpadTextEdit::EpadTextEdit(QWidget *parent) : QPlainTextEdit(parent)
{
}

bool EpadTextEdit::eventFilter(QObject *obj, QEvent *event)
{
    // we need this strange newline character we are getting in the
    // selected text for newlines
    const QString newLine =
        QString::fromUtf8(QByteArray::fromHex(QByteArrayLiteral("e280a9")));
    const QString indentCharacters = "\t";

    if(event->type() == QEvent::KeyPress)
        return false;

    if(!this->hasFocus())
        return false;

    auto *keyEvent = static_cast<QKeyEvent *>(event);
    QTextCursor cursor = textCursor();
    QString selectedText = cursor.selectedText();
    if(selectedText.isEmpty())
        return false;

    if (keyEvent->key() == Qt::Key_Tab)
    {
        // replace trailing new line to prevent an indent of the line after
        // the selection
        // QString newText = selectedText.replace(
        //     QRegularExpression(QRegularExpression::escape(newLine) +
        //                        QStringLiteral("$")),
        //     QStringLiteral("\n"));
        QString newText = selectedText;
        auto debugS = selectedText.toStdString();

        // indent text
        newText.replace(newLine, QStringLiteral("\n") + indentCharacters)
            .prepend(indentCharacters);
        debugS = newText.toStdString();

        // remove trailing \t
        newText.remove(QRegularExpression(QStringLiteral("\\t$")));
        debugS = newText.toStdString();
        // insert the new text
        cursor.insertText(newText);
        //
        // update the selection to the new text
        cursor.setPosition(cursor.position() - newText.size(),
                           QTextCursor::KeepAnchor);
        setTextCursor(cursor);
        return true;
    }
    else if(keyEvent->key() == Qt::Key_Backtab)
    {

    }

    return false;
}
