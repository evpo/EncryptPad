#pragma once

#include <QObject>
#include <QTextEdit>
#include "plain_text_edit.h"

class QMainWindow;
class QTextDocument;
class QString;
class QWidget;
class QTextCursor;

class Proxy;

namespace FakeVim {
namespace Internal {
class FakeVimHandler;
struct ExCommand;
} // namespace Internal
} // namespace FakeVim

PlainTextEdit *createEditorWidget(QWidget *parent = 0);
void initHandler(FakeVim::Internal::FakeVimHandler *handler);
void clearUndoRedo(QWidget *editor);
Proxy *connectSignals( FakeVim::Internal::FakeVimHandler *handler, QMainWindow *mainWindow, QWidget *editor);

class Proxy : public QObject
{
    Q_OBJECT

public:
    explicit Proxy(QWidget *widget, QMainWindow *mw, QObject *parent = nullptr);
    void openFile(const QString &fileName);

signals:
    void handleInput(const QString &keys);

    void requestRead(const QString &path);

    void requestSave();
    void requestSaveAndQuit();

    void requestSave(const QString &path);
    void requestSaveAndQuit(const QString &path);

    void requestQuit();
    void requestRun();

public slots:
    void changeStatusData(const QString &info);
    void highlightMatches(const QString &pattern);
    void changeStatusMessage(const QString &contents, int cursorPos);
    void changeExtraInformation(const QString &info);
    void updateStatusBar();
    void handleExCommand(bool *handled, const FakeVim::Internal::ExCommand &cmd);
    void requestSetBlockSelection(const QTextCursor &tc);
    void requestDisableBlockSelection();
    void updateBlockSelection();
    void requestHasBlockSelection(bool *on);
    void indentRegion(int beginBlock, int endBlock, QChar typedChar);
    void checkForElectricCharacter(bool *result, QChar c);

private:
    static int firstNonSpace(const QString &text);

    void updateExtraSelections();
    bool wantRead(const FakeVim::Internal::ExCommand &cmd);
    bool wantSaveAndQuit(const FakeVim::Internal::ExCommand &cmd);
    bool wantSave(const FakeVim::Internal::ExCommand &cmd);
    bool wantQuit(const FakeVim::Internal::ExCommand &cmd);
    bool wantRun(const FakeVim::Internal::ExCommand &cmd);

    void invalidate();
    bool hasChanges(const QString &fileName);

    QTextDocument *document() const;
    QString content() const;

    QWidget *m_widget;
    QMainWindow *m_mainWindow;
    QString m_statusMessage;
    QString m_statusData;

    QList<QTextEdit::ExtraSelection> m_searchSelection;
    QList<QTextEdit::ExtraSelection> m_clearSelection;
    QList<QTextEdit::ExtraSelection> m_blockSelection;
};
