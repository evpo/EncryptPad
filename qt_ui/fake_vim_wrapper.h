#pragma once
#include <memory>
#include <QMainWindow>
#include "fake_vim_edit.h"
#include <fakevimhandler.h>
#include "plain_text_edit.h"

struct FakeVimWrapper
{
    std::unique_ptr<FakeVim::Internal::FakeVimHandler> handler;
    std::unique_ptr<Proxy> proxy;
};

FakeVimWrapper CreateFakeVimWrapper(PlainTextEdit *editor, QMainWindow *mainWindow);
