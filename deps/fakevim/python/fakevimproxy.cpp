#include "python/fakevimproxy.h"

#include <QObject>
#include <QString>
#include <QTextEdit>

FakeVimProxy::FakeVimProxy(QObject *handler, QObject *parent)
    : QObject(parent)
{
    FakeVimHandler *h = qobject_cast<FakeVimHandler*>(handler);

    h->commandBufferChanged.connect([this, h](const QString &msg, int cursorPos, int anchorPos, int messageLevel) {
        call_commandBufferChanged(msg, cursorPos, anchorPos, messageLevel, h);
    });
    h->statusDataChanged.connect([this](const QString &msg) {
        call_statusDataChanged(msg);
    });
    h->extraInformationChanged.connect([this](const QString &msg) {
        call_extraInformationChanged(msg);
    });
    h->selectionChanged.connect([this](const QList<QTextEdit::ExtraSelection> &selection) {
        call_selectionChanged(selection);
    });
    h->highlightMatches.connect([this](const QString &needle) {
        call_highlightMatches(needle);
    });
    h->moveToMatchingParenthesis.connect([this](bool *moved, bool *forward, QTextCursor *cursor) {
        call_moveToMatchingParenthesis(moved, forward, cursor);
    });
    h->checkForElectricCharacter.connect([this](bool *result, QChar c) {
        call_checkForElectricCharacter(result, c);
    });
    h->indentRegion.connect([this](int beginLine, int endLine, QChar typedChar) {
        call_indentRegion(beginLine, endLine, typedChar);
    });
    h->simpleCompletionRequested.connect([this](const QString &needle, bool forward) {
        call_simpleCompletionRequested(needle, forward);
    });
    h->windowCommandRequested.connect([this](const QString &key, int count) {
        call_windowCommandRequested(key, count);
    });
    h->findRequested.connect([this](bool reverse) {
        call_findRequested(reverse);
    });
    h->findNextRequested.connect([this](bool reverse) {
        call_findNextRequested(reverse);
    });
    h->handleExCommandRequested.connect([this](bool *handled, const ExCommand &cmd) {
        call_handleExCommandRequested(handled, cmd);
    });
    h->requestDisableBlockSelection.connect([this]() {
        call_requestDisableBlockSelection();
    });
    h->requestSetBlockSelection.connect([this](const QTextCursor &cursor) {
        call_requestSetBlockSelection(cursor);
    });
    h->requestBlockSelection.connect([this](QTextCursor *cursor) {
        call_requestBlockSelection(cursor);
    });
    h->requestHasBlockSelection.connect([this](bool *on) {
        call_requestHasBlockSelection(on);
    });
    h->foldToggle.connect([this](int depth) {
        call_foldToggle(depth);
    });
    h->foldAll.connect([this](bool fold) {
        call_foldAll(fold);
    });
    h->fold.connect([this](int depth, bool dofold) {
        call_fold(depth, dofold);
    });
    h->foldGoTo.connect([this](int count, bool current) {
        call_foldGoTo(count, current);
    });
    h->requestJumpToLocalMark.connect([this](QChar mark, bool backTickMode, const QString &fileName) {
        call_jumpToLocalMark(mark, backTickMode, fileName);
    });
    h->requestJumpToGlobalMark.connect([this](QChar mark, bool backTickMode, const QString &fileName) {
        call_jumpToGlobalMark(mark, backTickMode, fileName);
    });
    h->completionRequested.connect([this]() {
        call_completionRequested();
    });
    h->tabPreviousRequested.connect([this]() {
        call_tabPrevious();
    });
    h->tabNextRequested.connect([this]() {
        call_tabNext();
    });
}

void FakeVimProxy::call_commandBufferChanged(const QString &msg, int cursorPos,
        int anchorPos, int messageLevel, FakeVimHandler *eventFilter)
{
    commandBufferChanged(msg, cursorPos, anchorPos, messageLevel, eventFilter);
}

void FakeVimProxy::call_statusDataChanged(const QString &msg)
{
    statusDataChanged(msg);
}

void FakeVimProxy::call_extraInformationChanged(const QString &msg)
{
    extraInformationChanged(msg);
}

void FakeVimProxy::call_selectionChanged(const QList<QTextEdit::ExtraSelection> &selection)
{
    selectionChanged(selection);
}

void FakeVimProxy::call_highlightMatches(const QString &needle)
{
    highlightMatches(needle);
}

void FakeVimProxy::call_moveToMatchingParenthesis(bool *moved, bool *forward, QTextCursor *cursor)
{
    const QTextCursor cursor2 = moveToMatchingParenthesis();
    *moved = cursor2.isNull();
    if (*moved) {
        *moved = true;
        *forward = cursor2.position() > cursor->position();
        *cursor = cursor2;
    }
}

void FakeVimProxy::call_checkForElectricCharacter(bool *result, QChar c)
{
    *result = checkForElectricCharacter(c);
}

void FakeVimProxy::call_indentRegion(int beginLine, int endLine, QChar typedChar)
{
    indentRegion(beginLine, endLine, typedChar);
}

void FakeVimProxy::call_completionRequested()
{
    completionRequested();
}

void FakeVimProxy::call_simpleCompletionRequested(const QString &needle, bool forward)
{
    simpleCompletionRequested(needle, forward);
}

void FakeVimProxy::call_windowCommandRequested(const QString &key, int count)
{
    windowCommandRequested(key, count);
}

void FakeVimProxy::call_findRequested(bool reverse)
{
    findRequested(reverse);
}

void FakeVimProxy::call_findNextRequested(bool reverse)
{
    findNextRequested(reverse);
}

void FakeVimProxy::call_handleExCommandRequested(bool *handled, const FakeVim::Internal::ExCommand &cmd)
{
    *handled = handleExCommand(cmd);
}

void FakeVimProxy::call_requestDisableBlockSelection()
{
    disableBlockSelection();
}

void FakeVimProxy::call_requestSetBlockSelection(const QTextCursor &cursor)
{
    enableBlockSelection(cursor);
}

void FakeVimProxy::call_requestBlockSelection(QTextCursor *cursor)
{
    *cursor = blockSelection();
}

void FakeVimProxy::call_requestHasBlockSelection(bool *on)
{
    *on = hasBlockSelection();
}

void FakeVimProxy::call_foldToggle(int depth)
{
    foldToggle(depth);
}

void FakeVimProxy::call_foldAll(bool fold)
{
    foldAll(fold);
}

void FakeVimProxy::call_fold(int depth, bool fold)
{
    this->fold(depth, fold);
}

void FakeVimProxy::call_foldGoTo(int count, bool current)
{
    foldGoTo(count, current);
}

void FakeVimProxy::call_jumpToGlobalMark(QChar mark, bool backTickMode, const QString &fileName)
{
    jumpToGlobalMark(mark, backTickMode, fileName);
}

void FakeVimProxy::call_jumpToLocalMark(QChar mark, bool backTickMode, const QString &fileName)
{
    jumpToLocalMark(mark, backTickMode, fileName);
}

void FakeVimProxy::call_tabPrevious()
{
    tabPrevious();
}

void FakeVimProxy::call_tabNext()
{
    tabNext();
}
