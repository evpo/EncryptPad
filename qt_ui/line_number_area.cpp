#include "line_number_area.h"
#include <cmath>

// Qt
#include <QAbstractTextDocumentLayout>
#include <QPaintEvent>
#include <QPainter>
#include <QScrollBar>
#include <QTextBlock>
#include "plain_text_edit.h"
#include "fakevimactions.h"
#include "plog/Log.h"

LineNumberArea::LineNumberArea(PlainTextEdit *parent)
    : QWidget(parent), m_codeEditParent(parent)
{
}

QSize LineNumberArea::sizeHint() const
{
    if(isHidden())
    {
        return {0, 0};
    }

    if (m_codeEditParent == nullptr)
    {
        return QWidget::sizeHint();
    }

    unsigned maxShownNumber = 0;
    if(!getShowRelativeNumbers())
    {
        maxShownNumber = m_codeEditParent->document()->blockCount();
    }
    else
    {
        const auto h = m_codeEditParent->cursorRect(m_codeEditParent->textCursor()).height();
        maxShownNumber = h > 0 ? m_codeEditParent->viewport()->height() / h : 1;
    }
    const int digits = QString::number(maxShownNumber).length();
    int space;

#if QT_VERSION >= 0x050B00
    space = 15 + m_codeEditParent->fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
#else
    space = 15 + m_codeEditParent->fontMetrics().width(QLatin1Char('9')) * digits;
#endif

    return {space, 0};
}

bool LineNumberArea::getShowRelativeNumbers() const
{
    return theFakeVimSetting(FakeVim::Internal::FakeVimSettingsCode::ConfigRelativeNumber)->value().toBool();
}

void LineNumberArea::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    // Clearing rect to update
    auto backgroundColor = m_codeEditParent->palette().window().color();
    auto foregroundColor = m_codeEditParent->palette().windowText().color();
    painter.fillRect(event->rect(), backgroundColor);

    auto blockNumber = m_codeEditParent->getFirstVisibleBlock();
    auto block = m_codeEditParent->document()->findBlockByNumber(blockNumber);

    auto top = m_codeEditParent->getContentOffsetY();
    auto bottom = top + (int)m_codeEditParent->document()->documentLayout()->blockBoundingRect(block).height();

    auto currentLine = m_codeEditParent->palette().highlight().color();
    auto currentLineNumber = m_codeEditParent->textCursor().blockNumber();
    auto otherLines = foregroundColor;

    painter.setFont(m_codeEditParent->font());

    while (block.isValid() && top <= event->rect().bottom())
    {
        if (block.isVisible() && bottom >= event->rect().top())
        {
            auto isCurrentLine = currentLineNumber == blockNumber;
            painter.setPen(isCurrentLine ? currentLine : otherLines);
            auto numberToShow = getShowRelativeNumbers() ? std::abs(blockNumber - currentLineNumber) : blockNumber + 1;
            QString number = QString::number(numberToShow);
            painter.drawText(-5, top, sizeHint().width(), m_codeEditParent->fontMetrics().height(), Qt::AlignRight,
                             number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int)m_codeEditParent->document()->documentLayout()->blockBoundingRect(block).height();
        ++blockNumber;
    }
}
