#include "plain_text_edit.h"
#include <QDebug>
#include <QMimeData>

PlainTextEdit::PlainTextEdit(QWidget *parent) :
    QPlainTextEdit(parent)
{
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
