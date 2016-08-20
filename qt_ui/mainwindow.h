//**********************************************************************************
//EncryptPad Copyright 2016 Evgeny Pokhilko 
//<http://www.evpo.net/encryptpad>
//
//This file is part of EncryptPad
//
//EncryptPad is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 2 of the License, or
//(at your option) any later version.
//
//EncryptPad is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with EncryptPad.  If not, see <http://www.gnu.org/licenses/>.
//**********************************************************************************
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QDateTime>
#include <QStringList>
#include "async_load.h"
#include "encryptor.h"
#include "packet_composer.h"
#include "plain_text_switch.h"
#include "recent_files_service.h"
#include "load_save_handler.h"
#include "m_window_load_adapter.h"
#include "file_request_service.h"
#include "find_dialog.h"
#include "find_and_replace.h"

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QPlainTextEdit;
QT_END_NAMESPACE

struct ReplaceContext
{
    QString foundText;
    QString enteredText;
    bool matchCase;
    bool wholeWord;
};

class MainWindow : public QMainWindow {
private:
    Q_OBJECT
public:
    MainWindow();

    void onApplicationActive();
    bool getIsBusy() const;
    void setEncryptionKeyFile(const QString &file, bool clearKFKeyService = true);
    const QString &getEncryptionKeyFile() const;

protected:
    void closeEvent(QCloseEvent *event);

public slots:
    void AsyncOperationCompleted();
    void UpdateStatus(const QString& text);
    void UpdateEncryptedPlainSwitch(bool encrypted);
    void open(QString fileName);

private slots:
    bool newFile();
    void open();
    bool save();
    bool saveAs();
    void setFileProperties();
    void closeAndReset();
    void createNewKey();
    void setPassphrase();
    void setEncryptionKey();
    void clearPassphrase(bool makeFileDirty = true);
    void clearEncryptionKey(bool makeFileDirty = true);
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void search();
    void replace();
    void gotoTriggered();
    void generatePassphrase();
    void readOnlyToggled(bool flag);
    void wordWrapToggled(bool flag);
    void windowsEolToggled(bool flag);
    void about();
    void documentWasModified();
    void openPreferences();
    void openFileEncryption();
    void cursorPositionChanged();
    void onUrlDrop(QUrl url);
    void findNext(QString text, bool down, bool matchCase, bool wholeWord);
    void replaceAll(QString text, QString replaceWith, bool matchCase, bool wholeWord);
    void replaceOne(QString text, QString replaceWith, bool matchCase, bool wholeWord);
    void clearReplaceContext();
private:
    static const int maxZoomIn;
    static const int minZoomOut;

    QString curFile;
    QString encryptionKeyFile;
    ReplaceContext replaceContext;
    bool persistEncryptionKeyPath;
    QDateTime lastModified;

    QLabel *passphraseSet;
    QLabel *encryptionKeySet;
    QLabel *lineStatus;
    QLabel *charStatus;
    QPlainTextEdit *textEdit;

    FindDialog *findDialog;
    FindAndReplace *replaceDialog;

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *viewMenu;
    QMenu *encMenu;
    QMenu *settingsMenu;
    QMenu *helpMenu;
    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QToolBar *encToolBar;
    QToolBar *zoomToolBar;
    QAction *newAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *setFilePropertiesAct;
    QAction *closeAndResetAct;
    QAction *createNewKeyAct;
    QAction *exitAct;
    QAction *undoAct;
    QAction *redoAct;
    QAction *cutAct;
    QAction *copyAct;
    QAction *pasteAct;
    QAction *selectAllAct;
    QAction *searchAct;
    QAction *replaceAct;
    QAction *zoomInAct;
    QAction *zoomOutAct;
    QAction *resetZoomAct;
    QAction *aboutAct;
    QAction *aboutQtAct;
    QAction *setPassphraseAct;
    QAction *clearPassphraseAct;
    QAction *setEncryptionKeyAct;
    QAction *clearEncryptionKeyAct;
    QAction *openPreferencesAct;
    QAction *readOnlyAct;
    QAction *wordWrapAct;
    QAction *windowsEolAct;
    QAction *openFileEncryptionAct;
    QAction *gotoAct;
    QAction *generatePassphraseAct;

    EncryptPadEncryptor::Encryptor enc;
    EncryptPad::PacketMetadata metadata;
    bool encryptionModified;
    bool isBusy;
    bool saveLastUsedDirectory;
    bool enableBakFiles;
    bool takeBakFile;
    bool windowsEol;

    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    bool maybeSave();
    void makeDirty();
    void updateLineStatus();

    void loadFile(const QString &fileName, bool force_kf_passphrase_request = false);
    bool saveFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    void updateEncryptionKeyStatus();
    void startLoad(const QString &fileName, const QString &encryptionKeyFile,
                std::string &passphrase, EncryptPad::PacketMetadata &metadata, std::string &kf_passphrase);
    void startSave(const QString &fileName, std::string &kf_passphrase);
    QString strippedName(const QString &fullFileName);
    void setWindowsEol(bool flag);

    bool OpenPassphraseDialog(bool confirmationEnabled, std::string *passphrase = nullptr);
    bool OpenSetEncryptionKeyDialogue();
    void EnterWaitState();
    void ExitWaitState();
    void ConvertToWindowsEOL(QString &in, QByteArray &out);
    QString accessRepositoryPath(const QString &fileName);

    int currentZoom;

    AsyncLoad load_state_machine_;
    PlainTextSwitch plain_text_switch_;
    PlainTextFunctor plain_text_functor_;
    RecentFilesService recent_files_service_;
    FileRequestService file_request_service_;

    friend class LoadHandlerAdapter;
    LoadHandlerAdapter loadAdapter;
    EncryptPad::LoadHandler loadHandler;
    bool saveSuccess;
    QStringList passphraseGenerationSettings;
};

#endif
