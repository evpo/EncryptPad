/*
    Copyright (c) 2017, Lukas Holecek <hluk@email.cz>

    This file is part of CopyQ.

    CopyQ is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    CopyQ is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CopyQ.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "editor.h"
#include <fakevim/fakevimhandler.h>

#include <QApplication>
#include <QMainWindow>
#include <QStandardPaths>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    const QStringList args = qApp->arguments();
    const QString fileToEdit = args.value(1);

    // If FAKEVIM_PLAIN_TEXT_EDIT environment variable is 1 use QPlainTextEdit instead on QTextEdit;
    bool usePlainTextEdit = qgetenv("FAKEVIM_PLAIN_TEXT_EDIT") == "1";

    // Create editor widget.
    QWidget *editor = createEditorWidget(usePlainTextEdit);

    // Create FakeVimHandler instance which will emulate Vim behavior in editor widget.
    FakeVim::Internal::FakeVimHandler handler(editor, 0);

    // Create main window.
    QMainWindow mainWindow;
    initMainWindow(&mainWindow, editor, usePlainTextEdit ? "QPlainTextEdit" : "QTextEdit");

    // Connect slots to FakeVimHandler signals.
    Proxy *proxy = connectSignals(&handler, &mainWindow, editor);

    QObject::connect(proxy, &Proxy::handleInput,
        &handler, [&handler] (const QString &text) { handler.handleInput(text); });

    QString fileName = fileToEdit;
    QObject::connect(proxy, &Proxy::requestSave, proxy, [proxy, fileName] () {
        proxy->save(fileName);
    });

    QObject::connect(proxy, &Proxy::requestSaveAndQuit, proxy, [proxy, fileName] () {
        if (proxy->save(fileName)) {
            proxy->cancel(fileName);
        }
    });
    QObject::connect(proxy, &Proxy::requestQuit, proxy, [proxy, fileName] () {
        proxy->cancel(fileName);
    });

    // Initialize FakeVimHandler.
    initHandler(&handler);

    // Load vimrc if it exists
    QString vimrc = QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
#ifdef Q_OS_WIN
        + QLatin1String("/_vimrc");
#else
        + QLatin1String("/.vimrc");
#endif
    if (QFile::exists(vimrc)) {
        handler.handleCommand(QLatin1String("source ") + vimrc);
    } else {
        // Set some Vim options.
        handler.handleCommand(QLatin1String("set expandtab"));
        handler.handleCommand(QLatin1String("set shiftwidth=8"));
        handler.handleCommand(QLatin1String("set tabstop=16"));
        handler.handleCommand(QLatin1String("set autoindent"));
        handler.handleCommand(QLatin1String("set smartindent"));
    }

    // Clear undo and redo queues.
    clearUndoRedo(editor);

    if (!fileToEdit.isEmpty()) {
        proxy->openFile(fileToEdit);
    }

    if (args.size() > 2) {
        for (const QString &cmd : args.mid(2))
            handler.handleInput(cmd);
        return 0;
    }

    return app.exec();
}
