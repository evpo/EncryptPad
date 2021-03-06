#include "fake_vim_wrapper.h"
#include <QStandardPaths>

std::unique_ptr<FakeVimWrapper> CreateFakeVimWrapper(PlainTextEdit *editor, QMainWindow *mainWindow)
{
    std::unique_ptr<FakeVimWrapper> retVal(new FakeVimWrapper());
    // Create FakeVimHandler instance which will emulate Vim behavior in editor widget.
    retVal->handler.reset(new FakeVim::Internal::FakeVimHandler(editor, 0));
    auto *handlerPtr = retVal->handler.get();
    retVal->proxy.reset(connectSignals(handlerPtr, mainWindow, editor));

    QObject::connect(retVal->proxy.get(), &Proxy::handleInput,
        retVal->handler.get(), [handlerPtr] (const QString &text) {
        handlerPtr->handleInput(text);
        });

    initHandler(handlerPtr);

    // Load vimrc if it exists
    QString vimrc = QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
#ifdef Q_OS_WIN
        + QLatin1String("/_encryptpad/vimrc");
#else
        + QLatin1String("/.encryptpad/vimrc");
#endif

    if (QFile::exists(vimrc))
    {
        handlerPtr->handleCommand(QLatin1String("source ") + vimrc);
    }
    else
    {
        // Set some Vim options.
        handlerPtr->handleCommand(QLatin1String("set expandtab"));
        handlerPtr->handleCommand(QLatin1String("set shiftwidth=4"));
        handlerPtr->handleCommand(QLatin1String("set tabstop=4"));
        handlerPtr->handleCommand(QLatin1String("set autoindent"));
        handlerPtr->handleCommand(QLatin1String("set smartindent"));
    }

    clearUndoRedo(editor);

    return retVal;
}
