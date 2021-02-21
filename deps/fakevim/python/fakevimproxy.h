#include <fakevim/fakevimhandler.h>

#include <QObject>
#include <QTextCursor>
#include <QTextEdit>

class QString;

using ExCommand = FakeVim::Internal::ExCommand;
using FakeVimHandler = FakeVim::Internal::FakeVimHandler;

class FakeVimProxy : public QObject
{
    Q_OBJECT
public:
    explicit FakeVimProxy(QObject *handler, QObject *parent = 0);

    virtual void commandBufferChanged(const QString &, int, int, int, QObject *) {}
    virtual void statusDataChanged(const QString &) {}
    virtual void extraInformationChanged(const QString &) {}
    virtual void selectionChanged(const QList<QTextEdit::ExtraSelection> &) {}
    virtual void highlightMatches(const QString &) {}
    virtual QTextCursor moveToMatchingParenthesis() { return QTextCursor(); }
    virtual bool checkForElectricCharacter(QChar) { return false; }
    virtual void indentRegion(int, int, QChar) {}
    virtual void completionRequested() {}
    virtual void simpleCompletionRequested(const QString &, bool) {}
    virtual void windowCommandRequested(const QString &, int) {}
    virtual void findRequested(bool) {}
    virtual void findNextRequested(bool) {}
    virtual bool handleExCommand(const ExCommand &) { return false; }
    virtual void disableBlockSelection() {}
    virtual void enableBlockSelection(const QTextCursor &) {}
    virtual QTextCursor blockSelection() { return QTextCursor(); }
    virtual bool hasBlockSelection() { return false; }
    virtual void foldToggle(int) {}
    virtual void foldAll(bool) {}
    virtual void fold(int, bool) {}
    virtual void foldGoTo(int, bool) {}
    virtual void jumpToLocalMark(QChar, bool, const QString &) {}
    virtual void jumpToGlobalMark(QChar, bool, const QString &) {}

    virtual void tabPrevious() {}
    virtual void tabNext() {}

private:
    void call_commandBufferChanged(const QString &msg, int cursorPos,
            int anchorPos, int messageLevel, FakeVimHandler *eventFilter);
    void call_statusDataChanged(const QString &msg);
    void call_extraInformationChanged(const QString &msg);
    void call_selectionChanged(const QList<QTextEdit::ExtraSelection> &selection);
    void call_highlightMatches(const QString &needle);
    void call_moveToMatchingParenthesis(bool *moved, bool *forward, QTextCursor *cursor);
    void call_checkForElectricCharacter(bool *result, QChar c);
    void call_indentRegion(int beginLine, int endLine, QChar typedChar);
    void call_completionRequested();
    void call_simpleCompletionRequested(const QString &needle, bool forward);
    void call_windowCommandRequested(const QString &key, int count);
    void call_findRequested(bool reverse);
    void call_findNextRequested(bool reverse);
    void call_handleExCommandRequested(bool *handled, const ExCommand &cmd);
    void call_requestDisableBlockSelection();
    void call_requestSetBlockSelection(const QTextCursor&);
    void call_requestBlockSelection(QTextCursor *cursor);
    void call_requestHasBlockSelection(bool *on);
    void call_foldToggle(int depth);
    void call_foldAll(bool fold);
    void call_fold(int depth, bool fold);
    void call_foldGoTo(int count, bool current);
    void call_jumpToGlobalMark(QChar mark, bool backTickMode, const QString &fileName);

    void call_jumpToLocalMark(QChar, bool, const QString &);
    void call_tabPrevious();
    void call_tabNext();
};

