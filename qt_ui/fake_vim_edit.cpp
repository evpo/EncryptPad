#include "fake_vim_edit.h"
#include "fakevimactions.h"
#include "fakevimhandler.h"

#include <QApplication>
#include <QFontMetrics>
#include <QMainWindow>
#include <QMessageBox>
#include <QPainter>
#include <QPlainTextEdit>
#include <QStatusBar>
#include <QTextBlock>
#include <QTextEdit>
#include <QTextStream>
#include <QTemporaryFile>
#include <QStandardPaths>
#include <QDebug>
#include <QDir>

#include "plog/Log.h"

using namespace FakeVim::Internal;

typedef QLatin1String _;

class FakeVimEditor : public PlainTextEdit
{
private:
    int getCursorWidth()
    {
        QFontMetrics fm(QPlainTextEdit::font());
        // LOG_DEBUG << "font: " << QPlainTextEdit::font().toString();
        const int position = QPlainTextEdit::textCursor().position();
        const QChar c = QPlainTextEdit::document()->characterAt(position);

#if QT_VERSION < QT_VERSION_CHECK(5, 11, 0)
        int width = fm.width(c);
#else
        int width = fm.horizontalAdvance(c);
#endif
        return width;
    }

    void resizeCursorWidth()
    {
        QPlainTextEdit::setCursorWidth(getCursorWidth());
    }

    void onTextChanged()
    {
        if(!this->m_isThinCursorRequested)
        {
            resizeCursorWidth();
        }
        else
        {
            QPlainTextEdit::setCursorWidth(1);
        }
    }
public:
    FakeVimEditor(QWidget *parent = 0) : PlainTextEdit(parent)
    {
        this->connect(this, &QPlainTextEdit::textChanged, [this]() { onTextChanged(); });
    }

    virtual ~FakeVimEditor()
    {

    }

protected:
    void paintEvent(QPaintEvent *e) override
    {
        LOG_INFO << "x=" << e->rect().x() << " y=" << e->rect().y() << " width=" << e->rect().width() << " height=" << e->rect().height();
        PlainTextEdit::paintEvent(e);
        if(m_skipNextPaint)
        {
            m_skipNextPaint = false;
            return;
        }
        if(!m_initialCursorSet)
        {
            onTextChanged();
            m_initialCursorSet = true;
        }

        QRect rect = QPlainTextEdit::cursorRect();
        if(!e->rect().intersects(rect))
        {
            return;
        }

        // m_skipNextPaint = true;
        auto width = getCursorWidth();
        rect.setWidth(width + 2);
        if(e->rect() != rect)
        {
            QPlainTextEdit::viewport()->update(rect);
        }

    }

public:
    void SetIsThinCursorRequested(bool flag)
    {
        this->m_isThinCursorRequested = flag;
        this->onTextChanged();
    }
private:
    bool m_initialCursorSet = false;
    bool m_skipNextPaint = false;
    bool m_isThinCursorRequested = false;
};

PlainTextEdit *createEditorWidget(QWidget *parent)
{
    FakeVimEditor *editor = new FakeVimEditor(parent);
    editor->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    editor->setObjectName(_("Editor"));
    editor->setFocus();
    return editor;
}

void initHandler(FakeVimHandler *handler)
{
    handler->handleCommand(_("set nopasskeys"));
    handler->handleCommand(_("set nopasscontrolkey"));

    handler->installEventFilter();
    handler->setupWidget();
}

void clearUndoRedo(QPlainTextEdit *editor)
{
    editor->setUndoRedoEnabled(false);
    editor->setUndoRedoEnabled(true);
}

Proxy *connectSignals(FakeVimHandler *handler, QMainWindow *mainWindow, PlainTextEdit *editor)
{
    Proxy *proxy = new Proxy(dynamic_cast<FakeVimEditor*>(editor), mainWindow, handler);

    handler->commandBufferChanged
        .connect([proxy](const QString &contents, int cursorPos, int /*anchorPos*/, int /*messageLevel*/) {
        proxy->changeStatusMessage(contents, cursorPos);
    });
    handler->extraInformationChanged.connect([proxy](const QString &text) {
        proxy->changeExtraInformation(text);
    });
    handler->statusDataChanged.connect([proxy](const QString &text) {
        proxy->changeStatusData(text);
    });
    handler->highlightMatches.connect([proxy](const QString &needle) {
        proxy->highlightMatches(needle);
    });
    handler->handleExCommandRequested.connect([proxy](bool *handled, const ExCommand &cmd) {
        proxy->handleExCommand(handled, cmd);
    });
    handler->requestSetBlockSelection.connect([proxy](const QTextCursor &cursor) {
        proxy->requestSetBlockSelection(cursor);
    });
    handler->requestDisableBlockSelection.connect([proxy] {
        proxy->requestDisableBlockSelection();
    });
    handler->requestHasBlockSelection.connect([proxy](bool *on) {
        proxy->requestHasBlockSelection(on);
    });

    handler->indentRegion.connect([proxy](int beginBlock, int endBlock, QChar typedChar) {
        proxy->indentRegion(beginBlock, endBlock, typedChar);
    });
    handler->checkForElectricCharacter.connect([proxy](bool *result, QChar c) {
            proxy->checkForElectricCharacter(result, c);
    });
    handler->thinCursorModeUpdated.connect([proxy](bool thinCursorMode) {
            proxy->thinCursorModeUpdated(thinCursorMode);
    });

    return proxy;
}

Proxy::Proxy(FakeVimEditor *widget, QMainWindow *mw, QObject *parent)
    : QObject(parent), m_widget(widget), m_mainWindow(mw)
{
}

void Proxy::openFile(const QString &fileName)
{
    emit handleInput(QString(_(":r %1<CR>")).arg(fileName));
}

void Proxy::changeStatusData(const QString &info)
{
    m_statusData = info;
    updateStatusBar();
}

void Proxy::thinCursorModeUpdated(bool thinCursorMode)
{
    m_widget->SetIsThinCursorRequested(thinCursorMode);
}

void Proxy::highlightMatches(const QString &pattern)
{
    QTextDocument *doc = m_widget->document();
    Q_ASSERT(doc);
    QTextEdit::ExtraSelection selection;
    selection.format.setBackground(Qt::yellow);
    selection.format.setForeground(Qt::black);

    // Highlight matches.
    QRegExp re(pattern);
    QTextCursor cur = doc->find(re);

    m_searchSelection.clear();

    int a = cur.position();
    while ( !cur.isNull() ) {
        if ( cur.hasSelection() ) {
            selection.cursor = cur;
            m_searchSelection.append(selection);
        } else {
            cur.movePosition(QTextCursor::NextCharacter);
        }
        cur = doc->find(re, cur);
        int b = cur.position();
        if (a == b) {
            cur.movePosition(QTextCursor::NextCharacter);
            cur = doc->find(re, cur);
            b = cur.position();
            if (a == b) break;
        }
        a = b;
    }

    updateExtraSelections();
}

void Proxy::changeStatusMessage(const QString &contents, int cursorPos)
{
    m_statusMessage = cursorPos == -1 ? contents
                                      : contents.left(cursorPos) + QChar(10073) + contents.mid(cursorPos);
    updateStatusBar();
}

void Proxy::changeExtraInformation(const QString &info)
{
    QMessageBox::information(m_widget, tr("Information"), info);
}

void Proxy::updateStatusBar()
{
    int slack = 80 - m_statusMessage.size() - m_statusData.size();
    QString msg = m_statusMessage + QString(slack, QLatin1Char(' ')) + m_statusData;
    m_mainWindow->statusBar()->showMessage(msg);
}

QString replaceHome(QString path)
{
    return path.replace("~", QDir::homePath());
}

void Proxy::handleExCommand(bool *handled, const ExCommand &cmd)
{

    if(wantRead(cmd)) {
        if(cmd.args.size() == 0)
            *handled = false;

        emit requestRead(replaceHome(cmd.args)); // :r <file>
    }
    else if ( wantSaveAndQuit(cmd) ) {
        if(cmd.args.size() > 0)
        {
            emit requestSaveAndQuit(replaceHome(cmd.args)); // :wq <file>
        }
        else
        {
            emit requestSaveAndQuit(); // :wq
        }
    } else if ( wantSave(cmd) ) {
        if(cmd.args.size() > 0)
        {
            emit requestSave(replaceHome(cmd.args)); // :w <file>
        }
        else
        {
            emit requestSave(); // :w
        }
    } else if ( wantQuit(cmd) ) {
        emit requestQuit(); // :q
    } else if ( wantRun(cmd) ) {
        emit requestRun();
    } else {
        *handled = false;
        return;
    }

    *handled = true;
}

void Proxy::requestSetBlockSelection(const QTextCursor &tc)
{
    QPlainTextEdit *plainEditor = m_widget;

    QPalette pal = m_widget->parentWidget() != nullptr ? m_widget->parentWidget()->palette()
                                                       : QApplication::palette();

    m_blockSelection.clear();
    m_clearSelection.clear();

    QTextCursor cur = tc;

    QTextEdit::ExtraSelection selection;
    selection.format.setBackground( pal.color(QPalette::Base) );
    selection.format.setForeground( pal.color(QPalette::Text) );
    selection.cursor = cur;
    m_clearSelection.append(selection);

    selection.format.setBackground( pal.color(QPalette::Highlight) );
    selection.format.setForeground( pal.color(QPalette::HighlightedText) );

    int from = cur.positionInBlock();
    int to = cur.anchor() - cur.document()->findBlock(cur.anchor()).position();
    const int min = qMin(cur.position(), cur.anchor());
    const int max = qMax(cur.position(), cur.anchor());
    for ( QTextBlock block = cur.document()->findBlock(min);
          block.isValid() && block.position() < max; block = block.next() ) {
        cur.setPosition( block.position() + qMin(from, block.length()) );
        cur.setPosition( block.position() + qMin(to, block.length()), QTextCursor::KeepAnchor );
        selection.cursor = cur;
        m_blockSelection.append(selection);
    }

    disconnect(plainEditor, &QPlainTextEdit::selectionChanged,
               this, &Proxy::updateBlockSelection);
    plainEditor->setTextCursor(tc);
    connect(plainEditor, &QPlainTextEdit::selectionChanged,
            this, &Proxy::updateBlockSelection);


    QPalette pal2 = m_widget->palette();
    pal2.setColor(QPalette::Highlight, Qt::transparent);
    pal2.setColor(QPalette::HighlightedText, Qt::transparent);
    m_widget->setPalette(pal2);

    updateExtraSelections();
}

void Proxy::requestDisableBlockSelection()
{
    QPlainTextEdit *plainEditor = m_widget;

    QPalette pal = m_widget->parentWidget() != nullptr ? m_widget->parentWidget()->palette()
                                                       : QApplication::palette();

    m_blockSelection.clear();
    m_clearSelection.clear();

    m_widget->setPalette(pal);

    disconnect(plainEditor, &QPlainTextEdit::selectionChanged,
               this, &Proxy::updateBlockSelection);

    updateExtraSelections();
}

void Proxy::updateBlockSelection()
{
    requestSetBlockSelection(m_widget->textCursor());
}

void Proxy::requestHasBlockSelection(bool *on)
{
    *on = !m_blockSelection.isEmpty();
}

void Proxy::indentRegion(int beginBlock, int endBlock, QChar typedChar)
{
    QTextDocument *doc = m_widget->document();
    Q_ASSERT(doc);

    const int indentSize = theFakeVimSetting(ConfigShiftWidth)->value().toInt();

    QTextBlock startBlock = doc->findBlockByNumber(beginBlock);

    // Record line lenghts for mark adjustments
    QVector<int> lineLengths(endBlock - beginBlock + 1);
    QTextBlock block = startBlock;

    for (int i = beginBlock; i <= endBlock; ++i) {
        const auto line = block.text();
        lineLengths[i - beginBlock] = line.length();
        if (typedChar.unicode() == 0 && line.simplified().isEmpty()) {
            // clear empty lines
            QTextCursor cursor(block);
            while (!cursor.atBlockEnd())
                cursor.deleteChar();
        } else {
            const auto previousBlock = block.previous();
            const auto previousLine = previousBlock.isValid() ? previousBlock.text() : QString();

            int indent = firstNonSpace(previousLine);
            if (typedChar == '}')
                indent = std::max(0, indent - indentSize);
            else if ( previousLine.endsWith("{") )
                indent += indentSize;
            const auto indentString = QString(" ").repeated(indent);

            QTextCursor cursor(block);
            cursor.beginEditBlock();
            cursor.movePosition(QTextCursor::StartOfBlock);
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, firstNonSpace(line));
            cursor.removeSelectedText();
            cursor.insertText(indentString);
            cursor.endEditBlock();
        }
        block = block.next();
    }
}

void Proxy::checkForElectricCharacter(bool *result, QChar c)
{
    *result = c == '{' || c == '}';
}

int Proxy::firstNonSpace(const QString &text)
{
    int indent = 0;
    while ( indent < text.length() && text.at(indent) == ' ' )
        ++indent;
    return indent;
}

void Proxy::updateExtraSelections()
{
    m_widget->setExtraSelections(m_clearSelection + m_searchSelection + m_blockSelection);
}

bool Proxy::wantRead(const ExCommand &cmd)
{
    return cmd.cmd == "r" || cmd.matches("r", "read");
}

bool Proxy::wantSaveAndQuit(const ExCommand &cmd)
{
    return cmd.cmd == "wq" || cmd.cmd == "x";
}

bool Proxy::wantSave(const ExCommand &cmd)
{
    return cmd.matches("w", "write") || cmd.matches("wa", "wall");
}

bool Proxy::wantQuit(const ExCommand &cmd)
{
    return cmd.matches("q", "quit") || cmd.matches("qa", "qall");
}

bool Proxy::wantRun(const ExCommand &cmd)
{
    return cmd.matches("run", "run") || cmd.matches("make", "make");
}

bool Proxy::hasChanges(const QString &fileName)
{
    if (fileName.isEmpty() && content().isEmpty())
        return false;

    QFile f(fileName);
    if (!f.open(QIODevice::ReadOnly))
        return true;

    QTextStream ts(&f);
    return content() != ts.readAll();
}

QTextDocument *Proxy::document() const
{
    return m_widget->document();
}

QString Proxy::content() const
{
    return document()->toPlainText();
}
