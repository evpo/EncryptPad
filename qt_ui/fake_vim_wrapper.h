#pragma once
#include <fakevimhandler.h>
#include <memory>
#include <QMainWindow>
#include "fake_vim_edit.h"

struct FakeVimWrapper
{
    std::unique_ptr<FakeVim::Internal::FakeVimHandler> handler;
    std::unique_ptr<Proxy> proxy;
};

std::unique_ptr<FakeVimWrapper> CreateFakeVimWrapper(PlainTextEdit *editor, QMainWindow *mainWindow);
