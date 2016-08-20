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
#include <QtGui>
#include <QtWidgets>
#include <QFontDatabase>

#include <iostream>
#include <string>
#include <algorithm>
#include "get_password_dialog.h"
#include "confirm_password_dialog.h"
#include "mainwindow.h"
#include "key_generation.h"
#include "file_encryption.h"
#include "algo_spec.h"
#include "file_properties_dialog.h"
#include "os_api.h"
#include "file_name_helper.h"
#include "new_key_dialog.h"
#include "preferences_dialog.h"
#include "find_and_replace.h"
#include "set_encryption_key.h"
#include "get_password_or_key_dialog.h"
#include "file_encryption_dialog.h"
#include "password_generation_dialog.h"
#include "version.h"
#include "plain_text_edit.h"
#include "common_definitions.h"

typedef unsigned char byte;

namespace 
{
#if defined(__MINGW__) || defined(__MINGW32__)
    const char *kRepositoryDirName = "_encryptpad";
#else
    const char *kRepositoryDirName = ".encryptpad";
#endif

    const char *kConfigFileName = "encryptpad.ini";
    const bool kDefaultWindowsEol = false;

    void SetDefaultMetadataValues(EncryptPad::PacketMetadata &metadata)
    {
        using namespace EncryptPad;
        metadata = {};
        metadata.cipher_algo = kDefaultCipherAlgo;
        metadata.hash_algo = kDefaultHashAlgo;
        metadata.compression = kDefaultCompression;
        metadata.iterations = kDefaultIterations;
        metadata.file_name = "CONSOLE";
    }

    // TakeBakFile
    // returns true if ok and false if we need to cancel the whole operation
    bool TakeBakFile(const QString &fileName)
    {
        QFileInfo fileInfo(fileName);
        QFile file(fileName);

        if(!file.exists())
            return true;

        QString bakFileName = fileInfo.dir().path() + "/" + fileInfo.completeBaseName() + QString(".bak");

        if(bakFileName.toUpper() == fileName.toUpper())
        {
            // we are editing the bak file
            return true;
        }

        if(QFile::exists(bakFileName))
        {
            if(!QFile::remove(bakFileName))
                return false;
        }

        if(!file.rename(bakFileName))
            return false;

        return true;
    }
}

const int MainWindow::maxZoomIn = 75;
const int MainWindow::minZoomOut = 3;

MainWindow::MainWindow(): encryptionKeyFile(tr("")), persistEncryptionKeyPath(false), 
    enc(), metadata(), encryptionModified(false), isBusy(false), saveLastUsedDirectory(false),
    enableBakFiles(false), takeBakFile(false), windowsEol(kDefaultWindowsEol), currentZoom(0), load_state_machine_(enc),
    plain_text_switch_(this), plain_text_functor_(plain_text_switch_), recent_files_service_(this),
    loadAdapter(this),
    loadHandler(this, loadAdapter, metadata),
    saveSuccess(false)
{
    SetDefaultMetadataValues(metadata);
    setWindowIcon(QIcon(":/images/application_icon.png"));

    textEdit = new PlainTextEdit(this);
    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    textEdit->setFont(fixedFont);
    setCentralWidget(textEdit);
    QPalette palette = textEdit->palette();
    QColor color = palette.color(QPalette::Active, QPalette::Highlight);
    palette.setColor(QPalette::Inactive, QPalette::Highlight, color);
    color = palette.color(QPalette::Active, QPalette::HighlightedText);
    palette.setColor(QPalette::Inactive, QPalette::HighlightedText, color);
    textEdit->setPalette(palette);

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();

    readSettings();

    connect(textEdit->document(), SIGNAL(contentsChanged()), this, SLOT(documentWasModified()));
    connect(textEdit, SIGNAL(cursorPositionChanged()), this, SLOT(cursorPositionChanged()));
    connect(textEdit, SIGNAL(urlDropped(QUrl)), this, SLOT(onUrlDrop(QUrl)));
    connect(&load_state_machine_, SIGNAL(AsyncOperationCompleted()), this, SLOT(AsyncOperationCompleted()));
    connect(&load_state_machine_, SIGNAL(UpdateStatus(const QString&)), this, SLOT(UpdateStatus(const QString &)));
    connect(&plain_text_switch_, SIGNAL(UpdateSwitch(bool)), this, SLOT(UpdateEncryptedPlainSwitch(bool)));
    connect(&recent_files_service_, SIGNAL(FileSelected(const QString &)), this, SLOT(open(QString)));

    plain_text_functor_.EncryptedPlainSwitchChange(!enc.GetIsPlainText());
    enc.SetEncryptedPlainSwitchFunctor(&plain_text_functor_);
    setCurrentFile("");

    update();

    setUnifiedTitleAndToolBarOnMac(true);
    updateEncryptionKeyStatus();
}

void MainWindow::updateLineStatus()
{
    QTextCursor cursor = textEdit->textCursor();
    QTextDocument *doc = textEdit->document();
    int charCount = doc->characterCount();
    int count = doc->blockCount();

    int lineNumber = cursor.blockNumber();

    lineStatus->setText(tr("ln: %1 of %2").arg(QString::number(lineNumber + 1)).arg(QString::number(count)));
    charStatus->setText(tr("chars: %1").arg(QString::number(charCount - 1)));
}

void MainWindow::cursorPositionChanged()
{
    updateLineStatus();
}

void MainWindow::onUrlDrop(QUrl url)
{
    activateWindow();
    QCoreApplication::processEvents();
    open(url.toLocalFile());
}

void MainWindow::UpdateEncryptedPlainSwitch(bool encrypted)
{
    if(!encrypted)
    {
        passphraseSet->setText(QString("<span style=\"color:#FF0000;\">") + tr("Passphrase not set") + QString("</span>"));
        clearPassphraseAct->setEnabled(false);
    }
    else
    {
        passphraseSet->setText(QString("<b>") + tr("Passphrase protected") + QString("</b>"));
        clearPassphraseAct->setEnabled(true);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

namespace
{
    QDateTime getLastModified(const QString &fileName, QDateTime defaultDateTime)
    {
        QFileInfo fileInfo(fileName);
        if(!fileInfo.exists())
            return defaultDateTime;

        return fileInfo.lastModified();
    }

    // Check if EOL is Windows or Unix. Returns true if EOL is for Windows.
    bool inferWindowsEol(const byte *begin, const byte *end)
    {
        const byte *it = std::find(begin, end, 0x0D);
        return (it != end);
    }
}

void MainWindow::AsyncOperationCompleted()
{
    ExitWaitState();

    bool success = false;
    bool rejected = false;
    bool request_kf_passphrase = false;

    switch(load_state_machine_.get_load_result())
    {
    case EncryptPadEncryptor::Result::OK:
        success = true;
        break;
    case EncryptPadEncryptor::Result::CpadFileIOError:
    case EncryptPadEncryptor::Result::InvalidCpadFile:
        QMessageBox::warning(
                    this,
                    "EncryptPad",
                    tr("Cannot open '%1'").arg(load_state_machine_.get_file_name()));
        rejected = true;
        break;
    case EncryptPadEncryptor::Result::EncryptionError:
        if(enc.GetIsPlainText())
        {
            // This is not normal. We decrypted without a passphrase.
            // There must have been a key so we should have got InvalidX2File instead
            // Process it just in case
            QMessageBox::warning(
                this,
                "EncryptPad",
                tr("Cannot open '%1'").arg(load_state_machine_.get_file_name()));
            rejected = true;
        }
        else
        {
            enc.SetIsPlainText();
        }

        break;
    case EncryptPadEncryptor::Result::InvalidKeyFilePassphrase:
        // There will be another request for the key file passphrase
        enc.ClearKFPassphrase();
        request_kf_passphrase = true;
        break;
    case EncryptPadEncryptor::Result::X2KeyIOError:
        QMessageBox::warning(
                    this,
                    "EncryptPad",
                    tr("Cannot open the encryption key"));
        rejected = true;
        break;
    case EncryptPadEncryptor::Result::InvalidX2File:
        QMessageBox::warning(
                    this,
                    "EncryptPad",
                    tr("The encryption key is invalid"));
        rejected = true;
        break;
    case EncryptPadEncryptor::Result::X2CurlIsNotFound:
        QMessageBox::warning(
                    this,
                    "EncryptPad",
                    tr("Cannot download the encryption key. CURL tool is not found."));
        rejected = true;
        break;
    case EncryptPadEncryptor::Result::X2CurlExitNonZero:
        QMessageBox::warning(
                    this,
                    "EncryptPad",
                    tr("Cannot download the key. CURL returned non zero exit code"));
        rejected = true;
        break;
    case EncryptPadEncryptor::Result::X2FileIsRequired:
        rejected = !OpenSetEncryptionKeyDialogue();
        break;
    default:
        rejected = true;
        break;
    }

    Q_ASSERT(!success || !rejected);

    if(!success)
    {
        statusBar()->showMessage(tr("Cannot load the file"), 2000);
    }

    if(!success && !rejected)
    {
        loadFile(load_state_machine_.get_file_name(), request_kf_passphrase);
    }
    else if(success)
    {
        // need to destroy the string quickly
        {
            setWindowsEol(inferWindowsEol(load_state_machine_.get_file_data().begin(),
                            load_state_machine_.get_file_data().end()));

            QString str = QString::fromUtf8(
                        reinterpret_cast<const char *>(load_state_machine_.get_file_data().begin()),
                        load_state_machine_.get_file_data().size());

            // clear the unencrypted data in the buffer
            load_state_machine_.ClearBuffer();
            textEdit->setPlainText(str);
            if(encryptionKeyFile.length() == 0 && enc.GetX2KeyLocation().length() > 0)
            {
                persistEncryptionKeyPath = true;
                setEncryptionKeyFile(QString::fromStdString(enc.GetX2KeyLocation()));
                updateEncryptionKeyStatus();
            }
            else if(encryptionKeyFile.length() > 0 && enc.GetX2KeyLocation().length() == 0)
            {
                persistEncryptionKeyPath = false;
                setEncryptionKeyFile(QString());
                updateEncryptionKeyStatus();
            }
        }
        // at this point only textEdit contains unencrypted data
        setCurrentFile(load_state_machine_.get_file_name());

        if(enableBakFiles)
        {
            takeBakFile = true;
        }

        statusBar()->showMessage(tr("File loaded"), 2000);

        recent_files_service_.PushFile(load_state_machine_.get_file_name());
        textEdit->setFocus();
    }
}

void MainWindow::UpdateStatus(const QString &text)
{
    statusBar()->showMessage(text);
}

bool MainWindow::newFile()
{
    using namespace EncryptPad;
    if (maybeSave()) {
        textEdit->clear();
        setCurrentFile("");

        CipherAlgo algo_before = metadata.cipher_algo;
        HashAlgo hash_algo_before = metadata.hash_algo;
        unsigned int iterations_before = metadata.iterations;
        
        SetDefaultMetadataValues(metadata);

        if(algo_before != metadata.cipher_algo || hash_algo_before != metadata.hash_algo 
                || iterations_before != metadata.iterations)
        {
            clearPassphrase();
        }

        setWindowsEol(kDefaultWindowsEol);

        return true;
    }
    else
    {
        return false;
    }
}

void MainWindow::closeAndReset()
{
    if(newFile())
    {
        bool makeFileDirty = false;
        clearPassphrase(makeFileDirty);
        clearEncryptionKey(makeFileDirty);
    }
}

void MainWindow::onApplicationActive()
{
    if(isBusy)
        return;

    FlagResetter isBusyResetter(isBusy);

    if(curFile.isEmpty())
        return;

    QDateTime modified = getLastModified(curFile, lastModified);
    if(lastModified == modified)
        return;

    QString message = tr("The file has been modified by another program. Do you want to reload it?");
    if(textEdit->document()->isModified())
    {
        message = tr("The file has been modified by another program. Do you want to reload it"
            " and lose the changes made in this application?");
    }

    auto reply = QMessageBox::question(this, "EncryptPad",
        message,
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes);

    lastModified = modified;

    if(reply == QMessageBox::No)
    {
        return;
    }

    isBusyResetter.Disconnect();

    loadFile(curFile);
}

bool MainWindow::getIsBusy() const
{
    return isBusy;
}

void MainWindow::open(QString fileName)
{
    if(isBusy)
        return;

    FlagResetter isBusyResetter(isBusy);

    if (maybeSave()) {
        QString selectedFilter;
        if(fileName.isNull())
        {
            FileRequestSelection selection = file_request_service_.RequestExistingFile(
                        this,
                        tr("Open File"),
                        QString(),
                        GetOpenDialogFilter());

            if(!selection.cancelled)
            {
                fileName = selection.file_name;
                selectedFilter = selection.filter;
            }
        }

        if (!fileName.isEmpty())
        {
            //leave flag in set state
            isBusyResetter.Disconnect();
            loadFile(fileName);
        }
    }
}

void MainWindow::open()
{
    open(QString());
}

bool MainWindow::save()
{
    if (curFile.isEmpty()) {
        return saveAs();
    } else {
        return saveFile(curFile);
    }
}

bool MainWindow::saveAs()
{
    QString selectedFilter;
    if(!curFile.isEmpty())
    {
        selectedFilter = GetFileFilterFromFileName(curFile);
    }

    FileRequestSelection selection = file_request_service_.RequestNewFile(
                this,
                tr("Save File As"),
                curFile,
                GetSaveDialogFilter(),
                &selectedFilter);

    if(selection.cancelled)
        return false;

    // Set the bak file flag if the file name has changed
    if(curFile != selection.file_name && enableBakFiles)
    {
        takeBakFile = true;
    }

    return saveFile(selection.file_name);
}

QString MainWindow::accessRepositoryPath(const QString &fileName)
{
    QDir dir(QDir::home());

    if(!(dir.cd(tr(kRepositoryDirName)) ||
         (dir.mkdir(kRepositoryDirName) && dir.cd(kRepositoryDirName))))
    {
        QMessageBox::warning(
                    this,
                    "EncryptPad",
                    tr("Cannot create the repository directory in HOME"));
        return QString();
    }

    return dir.filePath(fileName);
}

void MainWindow::createNewKey()
{
    using namespace EncryptPad;

    QString fileName;
    NewKeyDialog dlg(this, file_request_service_);
    if(dlg.exec() == QDialog::Rejected)
        return;

    bool isRepo = dlg.isKeyInRepository();
    fileName = dlg.getKey();

    if (fileName.isEmpty())
        return;

    QString filePath = fileName;
    if(isRepo)
    {
        if(!fileName.endsWith(".key", Qt::CaseInsensitive))
            fileName += ".key";

        filePath = accessRepositoryPath(fileName);
        if(filePath.isEmpty())
            return;
    }

    std::string kf_passphrase;
    if(!loadHandler.OpenPassphraseDialog(true, &kf_passphrase, false, tr("Passphrase for Key File")))
        return;

    if(kf_passphrase.empty())
    {
        auto ret = QMessageBox::warning(
                    this,
                    "EncryptPad",
                    tr("You left the passphrase blank. The key file is going to be UNENCRYPTED. Do you want to continue?"),
                    QMessageBox::Ok | QMessageBox::Cancel
                    );

        if(ret == QMessageBox::Cancel)
            return;
    }

    try
    {
        EncryptParams kf_encrypt_params;
        if(!kf_passphrase.empty())
        {
            kf_encrypt_params.key_service = &enc.GetKFKeyService();
            enc.ClearKFPassphrase();
            PacketMetadata metadata = GetDefaultKFMetadata();
            kf_encrypt_params.key_service->ChangePassphrase(
                    kf_passphrase, metadata.hash_algo, GetAlgoSpec(metadata.cipher_algo).key_size);
            std::fill(kf_passphrase.begin(), kf_passphrase.end(), '\0');
            GenerateNewKey(filePath.toStdString(), &kf_encrypt_params, &metadata);
        }
        else
        {
            GenerateNewKey(filePath.toStdString());
        }
    }
    catch(EncryptPad::IoException)
    {
        QMessageBox::warning(
            this,
            "EncryptPad",
            tr("Cannot generate the key '%1' Check the path and permissions.").arg(fileName));

        statusBar()->showMessage(tr("Cannot generate key"));
        return;
    }
    auto reply = QMessageBox::question(this, "EncryptPad",
        tr("Do you want to use the generated key for this file?"),
        QMessageBox::Yes | QMessageBox::No);

    if(reply == QMessageBox::Yes)
    {
        bool clearKFKeyService = false;
        setEncryptionKeyFile(isRepo ? fileName : filePath, clearKFKeyService);
        this->updateEncryptionKeyStatus();
    }
}

void MainWindow::about()
{
   QMessageBox::about(this, tr("About EncryptPad"),
            tr("<b>EncryptPad %1 Beta</b><br/><br/>"
               "Minimalist secure text editor and binary encryptor that implements <br/>"
               "RFC 4880 Open PGP format: "
               "symmetrically encrypted, compressed and integrity protected. "
               "The editor can ptotect files with passphrases, key files or both.<br/><br/>"
               "%2<br/>"
               "GNU General Public License v2<br/><br/>"
               ).arg(VER_PRODUCTVERSION_STR).arg(VER_LEGALCOPYRIGHT_STR) +
               QString("<a href=\"http://www.evpo.net/encryptpad\">http://www.evpo.net/encryptpad</a>"));
}

void MainWindow::documentWasModified()
{
    if(encryptionModified)
        return;
    setWindowModified(textEdit->document()->isModified());
    updateLineStatus();
}

void MainWindow::openPreferences()
{
    bool lastEnableBakFiles = enableBakFiles;

    PreferencesDialog dlg(this);
    dlg.setFont(textEdit->font());
    dlg.setRecentFiles(recent_files_service_.GetMaxFiles());
    dlg.setWordWrap(wordWrapAct->isChecked());
    dlg.setSaveLastUsedDirectory(saveLastUsedDirectory);
    dlg.setEnableBakFiles(enableBakFiles);
    dlg.setLibcurlPath(QString::fromStdString(enc.GetLibcurlPath()));
    if(dlg.exec() == QDialog::Rejected)
        return;

    recent_files_service_.SetMaxFiles(dlg.getRecentFiles());
    resetZoom();
    textEdit->setFont(dlg.getFont());
    wordWrapAct->setChecked(dlg.getWordWrap());
    saveLastUsedDirectory = dlg.getSaveLastUsedDirectory();
    enableBakFiles = dlg.getEnableBakFiles();
    enc.SetLibcurlPath(dlg.getLibcurlPath().toStdString());

    if(enableBakFiles && !lastEnableBakFiles)
    {
        takeBakFile = true;
    }
}

void MainWindow::createActions()
{
    newAct = new QAction(QIcon(":/images/famfamfam/page_white.png"), tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));

    openAct = new QAction(QIcon(":/images/famfamfam/folder_page_white.png"), tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    saveAct = new QAction(QIcon(":/images/famfamfam/disk.png"), tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    saveAsAct = new QAction(QIcon(":/images/famfamfam/disk_multiple.png"), tr("Save &As..."), this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Save the document under a new name"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    setFilePropertiesAct = new QAction(QIcon(":/images/famfamfam/page_white_gear.png"), tr("File &Properties..."), this);
    setFilePropertiesAct->setStatusTip(tr("Set file properties"));
    connect(setFilePropertiesAct, SIGNAL(triggered()), this, SLOT(setFileProperties()));

    closeAndResetAct = new QAction(QIcon(":/images/famfamfam/cross.png"), tr("&Close and Reset"), this);
    closeAndResetAct->setShortcuts(QKeySequence::Close);
    closeAndResetAct->setStatusTip(tr("Close and reset security settings"));
    connect(closeAndResetAct, SIGNAL(triggered()), this, SLOT(closeAndReset()));

    createNewKeyAct = new QAction(QIcon(":/images/famfamfam/key_add.png"), tr("Generate Key..."), this);
    createNewKeyAct->setStatusTip(tr("Generate a new key file"));
    connect(createNewKeyAct, SIGNAL(triggered()), this, SLOT(createNewKey()));

    setPassphraseAct = new QAction(QIcon(":/images/famfamfam/lock.png"), tr("Set &Passphrase..."), this);
    setPassphraseAct->setStatusTip(tr("Set passphrase for encryption and decryption"));
    connect(setPassphraseAct, SIGNAL(triggered()), this, SLOT(setPassphrase()));

    setEncryptionKeyAct = new QAction(QIcon(":/images/famfamfam/key.png"), tr("Set &Encryption Key..."), this);
    setEncryptionKeyAct->setStatusTip(tr("Set encryption key file"));
    connect(setEncryptionKeyAct, SIGNAL(triggered()), this, SLOT(setEncryptionKey()));

    clearEncryptionKeyAct = new QAction(QIcon(":/images/famfamfam/key_delete.png"), tr("Clear &Encryption Key"), this);
    clearEncryptionKeyAct->setStatusTip(tr("Clear encryption key file. Passphrase only (if set)."));
    connect(clearEncryptionKeyAct, SIGNAL(triggered()), this, SLOT(clearEncryptionKey()));

    clearPassphraseAct = new QAction(QIcon(":/images/famfamfam/lock_delete.png"), tr("&Clear Passphrase"), this);
    clearPassphraseAct->setStatusTip(tr("Save without passphrase encryption. File key encryption only (if enabled)."));
    connect(clearPassphraseAct, SIGNAL(triggered()), this, SLOT(clearPassphrase()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);

    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    undoAct = new QAction(QIcon(":/images/famfamfam/arrow_undo.png"), tr("Undo"), this);
    undoAct->setShortcuts(QKeySequence::Undo);
    undoAct->setStatusTip(tr("Undo"));
    connect(undoAct, SIGNAL(triggered()), textEdit, SLOT(undo()));

    redoAct = new QAction(QIcon(":/images/famfamfam/arrow_redo.png"), tr("Redo"), this);
    redoAct->setShortcuts(QKeySequence::Redo);
    redoAct->setStatusTip(tr("Redo"));
    connect(redoAct, SIGNAL(triggered()), textEdit, SLOT(redo()));

    cutAct = new QAction(QIcon(":/images/famfamfam/cut.png"), tr("Cu&t"), this);
    cutAct->setShortcuts(QKeySequence::Cut);
    cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                            "clipboard"));
    connect(cutAct, SIGNAL(triggered()), textEdit, SLOT(cut()));

    copyAct = new QAction(QIcon(":/images/famfamfam/page_copy.png"), tr("&Copy"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
    connect(copyAct, SIGNAL(triggered()), textEdit, SLOT(copy()));

    pasteAct = new QAction(QIcon(":/images/famfamfam/page_paste.png"), tr("&Paste"), this);
    pasteAct->setShortcuts(QKeySequence::Paste);
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                              "selection"));
    connect(pasteAct, SIGNAL(triggered()), textEdit, SLOT(paste()));

    selectAllAct = new QAction(tr("&Select All"), this);
    selectAllAct->setShortcuts(QKeySequence::SelectAll);
    selectAllAct->setStatusTip(tr("Select all text"));
    connect(selectAllAct, SIGNAL(triggered()), textEdit, SLOT(selectAll()));

    searchAct = new QAction(QIcon(":/images/famfamfam/find.png"), tr("&Find..."), this);
    searchAct->setShortcuts(QKeySequence::Find);
    searchAct->setStatusTip(tr("Find in text"));
    connect(searchAct, SIGNAL(triggered()), this, SLOT(search()));

    gotoAct = new QAction(tr("&Go to..."), this);
    gotoAct->setStatusTip(tr("Go to"));
    gotoAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_G));
    connect(gotoAct, SIGNAL(triggered()), this, SLOT(gotoTriggered()));

    generatePassphraseAct = new QAction(QIcon(":/images/famfamfam/user_suit.png"), tr("&Generate Passphrase..."), this);
    generatePassphraseAct->setStatusTip(tr("Generate passphrase"));
    connect(generatePassphraseAct, SIGNAL(triggered()), this, SLOT(generatePassphrase()));

    replaceAct = new QAction(tr("&Replace..."), this);
    replaceAct->setShortcut(QKeySequence::Replace);
    replaceAct->setStatusTip(tr("Find and replace text"));
    connect(replaceAct, SIGNAL(triggered()), this, SLOT(replace()));

    readOnlyAct = new QAction(QIcon(":/images/famfamfam/read_only.png"), tr("Read Only"), this);
    readOnlyAct->setCheckable(true);
    readOnlyAct->setStatusTip(tr("Switch Read Only mode"));
    connect(readOnlyAct, SIGNAL(toggled(bool)), this, SLOT(readOnlyToggled(bool)));
    connect(readOnlyAct, SIGNAL(toggled(bool)), replaceAct, SLOT(setDisabled(bool)));
    connect(readOnlyAct, SIGNAL(toggled(bool)), generatePassphraseAct, SLOT(setDisabled(bool)));

    wordWrapAct = new QAction(QIcon(":/images/famfamfam/wrap.png"), tr("Word Wrap"), this);
    wordWrapAct->setCheckable(true);
    wordWrapAct->setStatusTip(tr("Switch Word Wrap"));
    connect(wordWrapAct, SIGNAL(toggled(bool)), this, SLOT(wordWrapToggled(bool)));

    zoomInAct = new QAction(QIcon(":/images/famfamfam/magnifier_zoom_in.png"), tr("&Zoom In"), this);
    zoomInAct->setShortcuts(QKeySequence::ZoomIn);
    zoomInAct->setStatusTip(tr("Zoom In"));
    connect(zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));

    zoomOutAct = new QAction(QIcon(":/images/famfamfam/magnifier_zoom_out.png"), tr("&Zoom Out"), this);
    zoomOutAct->setShortcuts(QKeySequence::ZoomOut);
    zoomOutAct->setStatusTip(tr("Zoom Out"));
    connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));

    resetZoomAct = new QAction(QIcon(":/images/famfamfam/magnifier.png"), tr("&Reset Zoom"), this);
    resetZoomAct->setShortcut(QKeySequence("Ctrl+0"));
    resetZoomAct->setStatusTip(tr("Reset Zoom"));
    connect(resetZoomAct, SIGNAL(triggered()), this, SLOT(resetZoom()));

    aboutAct = new QAction(tr("&About..."), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));


    aboutQtAct = new QAction(tr("About &Qt"), this);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    openPreferencesAct = new QAction(tr("&Preferences..."), this);
    openPreferencesAct->setStatusTip(tr("Application preferences"));
    connect(openPreferencesAct, SIGNAL(triggered()), this, SLOT(openPreferences()));

    openFileEncryptionAct = new QAction(QIcon(":/images/famfamfam/arrow_switch.png"), tr("File Encryption..."), this);
    openFileEncryptionAct->setStatusTip(tr("File encryption"));
    connect(openFileEncryptionAct, SIGNAL(triggered()), this, SLOT(openFileEncryption()));

    windowsEolAct = new QAction(tr("Windows EOL"), this);
    windowsEolAct->setStatusTip(tr("Windows end of line: CR LN"));
    windowsEolAct->setCheckable(true);
    connect(windowsEolAct, SIGNAL(toggled(bool)), this, SLOT(windowsEolToggled(bool)));
    setWindowsEol(false);

    cutAct->setEnabled(false);
    copyAct->setEnabled(false);
    undoAct->setEnabled(false);
    redoAct->setEnabled(false);
    connect(textEdit, SIGNAL(copyAvailable(bool)),
            cutAct, SLOT(setEnabled(bool)));
    connect(textEdit, SIGNAL(copyAvailable(bool)),
            copyAct, SLOT(setEnabled(bool)));

    connect(textEdit, SIGNAL(undoAvailable(bool)),
            undoAct, SLOT(setEnabled(bool)));
    connect(textEdit, SIGNAL(redoAvailable(bool)),
            redoAct, SLOT(setEnabled(bool)));

    findDialog = new FindDialog(this);
    connect(findDialog, SIGNAL(findNext(QString,bool,bool,bool)),
            this, SLOT(findNext(QString,bool,bool,bool)));

    replaceDialog = new FindAndReplace(this);
    connect(replaceDialog, SIGNAL(findNext(QString,bool,bool,bool)),
            this, SLOT(findNext(QString,bool,bool,bool)));
    connect(replaceDialog, SIGNAL(replaceAll(QString,QString,bool,bool)),
            this, SLOT(replaceAll(QString,QString,bool,bool)));
    connect(replaceDialog, SIGNAL(replaceOne(QString,QString,bool,bool)),
            this, SLOT(replaceOne(QString,QString,bool,bool)));
    connect(replaceDialog, SIGNAL(finished(int)),
            this, SLOT(clearReplaceContext()));
}

void MainWindow::replaceAll(QString text, QString replaceWith, bool matchCase, bool wholeWord)
{
    QTextDocument::FindFlags flags = 0;
    if(matchCase)
        flags |= QTextDocument::FindCaseSensitively;

    if(wholeWord)
        flags |= QTextDocument::FindWholeWords;

    QTextCursor cursor = textEdit->textCursor();
    cursor.beginEditBlock();
    textEdit->moveCursor(QTextCursor::MoveOperation::Start);
    while(textEdit->find(text, flags))
    {
        QTextCursor foundTextCur = textEdit->textCursor();
        if(foundTextCur.hasSelection())
            foundTextCur.insertText(replaceWith);
    }
    cursor.endEditBlock();
}

void MainWindow::replaceOne(QString text, QString replaceWith, bool matchCase, bool wholeWord)
{
    QTextCursor cursor = textEdit->textCursor();
    if(!cursor.hasSelection() ||
            cursor.selectedText() != replaceContext.foundText ||
            text != replaceContext.enteredText ||
            replaceContext.matchCase != matchCase ||
            replaceContext.wholeWord != wholeWord)
    {
        findNext(text, true, matchCase, wholeWord);
        return;
    }

    cursor.beginEditBlock();
    cursor.insertText(replaceWith);
    cursor.endEditBlock();

    findNext(text, true, matchCase, wholeWord);
}

void MainWindow::clearReplaceContext()
{
    replaceContext.enteredText.clear();
    replaceContext.foundText.clear();
    replaceContext.matchCase = false;
    replaceContext.wholeWord = false;
}

void MainWindow::findNext(QString text, bool down, bool matchCase, bool wholeWord)
{
    QTextDocument::FindFlags flags = 0;
    if(!down)
        flags |= QTextDocument::FindBackward;

    if(matchCase)
        flags |= QTextDocument::FindCaseSensitively;

    if(wholeWord)
        flags |= QTextDocument::FindWholeWords;

    QString msgForWrapping = tr("End of file reached. Started from the top.");
    bool found = false;
    bool wrapped = false;
    clearReplaceContext();

    QTextCursor cursor = textEdit->textCursor();
    found = textEdit->find(text, flags);
    if(!found)
    {
        statusBar()->showMessage(msgForWrapping);
        textEdit->moveCursor(down ? QTextCursor::MoveOperation::Start : QTextCursor::MoveOperation::End);
        wrapped = true;
        found = textEdit->find(text, flags);
        if(!found)
        {
            textEdit->setTextCursor(cursor);
            statusBar()->showMessage(tr("Text not found."));
        }
    }

    if(found)
    {
        replaceContext.foundText = textEdit->textCursor().selectedText();
        replaceContext.enteredText = text;
        replaceContext.matchCase = matchCase;
        replaceContext.wholeWord = wholeWord;
        QString message;
        if(wrapped)
            message = msgForWrapping + " ";

        message += tr("Text found.");
        statusBar()->showMessage(message);
    }
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAct);

    fileMenu->addAction(openAct);

    fileMenu->addAction(saveAct);

    fileMenu->addAction(saveAsAct);
    fileMenu->addAction(setFilePropertiesAct);
    fileMenu->addAction(closeAndResetAct);
    QAction *separator = fileMenu->addSeparator();
    fileMenu->addAction(openFileEncryptionAct);

    separator = fileMenu->addSeparator();
    fileMenu->addAction(exitAct);
    recent_files_service_.Init(fileMenu, separator, exitAct);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(undoAct);
    editMenu->addAction(redoAct);
    editMenu->addSeparator();
    editMenu->addAction(cutAct);
    editMenu->addAction(copyAct);
    editMenu->addAction(pasteAct);
    editMenu->addAction(selectAllAct);
    editMenu->addSeparator();
    editMenu->addAction(searchAct);
    editMenu->addAction(replaceAct);
    editMenu->addAction(gotoAct);
    editMenu->addSeparator();
    editMenu->addAction(windowsEolAct);
    editMenu->addAction(generatePassphraseAct);
    editMenu->addSeparator();
    editMenu->addAction(readOnlyAct);

    viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(wordWrapAct);
    viewMenu->addSeparator();
    viewMenu->addAction(zoomInAct);
    viewMenu->addAction(zoomOutAct);
    viewMenu->addAction(resetZoomAct);

    menuBar()->addSeparator();

    encMenu = menuBar()->addMenu(tr("En&cryption"));
    encMenu->addAction(setPassphraseAct);
    encMenu->addAction(clearPassphraseAct);
    encMenu->addSeparator();
    encMenu->addAction(createNewKeyAct);
    encMenu->addAction(setEncryptionKeyAct);
    encMenu->addAction(clearEncryptionKeyAct);

    settingsMenu = menuBar()->addMenu(tr("&Settings"));
    settingsMenu->addAction(openPreferencesAct);

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
}

void MainWindow::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(newAct);

    fileToolBar->addAction(openAct);

    fileToolBar->addAction(saveAct);
    fileToolBar->addAction(saveAsAct);
    fileToolBar->addAction(openFileEncryptionAct);
    fileToolBar->addAction(setFilePropertiesAct);
    fileToolBar->addAction(closeAndResetAct);

    editToolBar = addToolBar(tr("Edit"));
    editToolBar->addAction(cutAct);
    editToolBar->addAction(copyAct);
    editToolBar->addAction(pasteAct);
    editToolBar->addAction(searchAct);
    editToolBar->addAction(generatePassphraseAct);
    editToolBar->addAction(readOnlyAct);

    encToolBar = addToolBar(tr("Encryption"));
    encToolBar->addAction(setPassphraseAct);
    encToolBar->addAction(clearPassphraseAct);
    encToolBar->addAction(createNewKeyAct);
    encToolBar->addAction(setEncryptionKeyAct);
    encToolBar->addAction(clearEncryptionKeyAct);

    zoomToolBar = addToolBar(tr("View"));
    zoomToolBar->addAction(wordWrapAct);
    zoomToolBar->addAction(zoomInAct);
    zoomToolBar->addAction(zoomOutAct);
    zoomToolBar->addAction(resetZoomAct);

}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));

    lineStatus = new QLabel("", this);
    statusBar()->addPermanentWidget(lineStatus);

    charStatus = new QLabel("", this);
    statusBar()->addPermanentWidget(charStatus);

    passphraseSet = new QLabel("", this);
    statusBar()->addPermanentWidget(passphraseSet);

    encryptionKeySet = new QLabel("", this);
    statusBar()->addPermanentWidget(encryptionKeySet);
}

void MainWindow::readSettings()
{
    QString configFile = accessRepositoryPath(kConfigFileName);
    if(configFile.isEmpty())
        return;

    QSettings settings(configFile, QSettings::IniFormat);
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(400, 400)).toSize();
    QString fontString = settings.value("font", QVariant(textEdit->font().toString())).toString();
    int max_files = settings.value("recent_files", QVariant(5)).toInt();
    bool readOnly = settings.value("read_only", QVariant(false)).toBool();
    readOnlyAct->setChecked(readOnly);
    readOnlyToggled(readOnly);
    bool wordWrap = settings.value("word_wrap", QVariant(false)).toBool();
    wordWrapAct->setChecked(wordWrap);
    wordWrapToggled(wordWrap);
    recent_files_service_.Deserialize(
            settings.value("recent_file_list", QVariant().toStringList()).toStringList(),
            max_files);
    passphraseGenerationSettings = settings.value(
                "passphrase_generation", QVariant().toStringList()).toStringList();
    saveLastUsedDirectory = settings.value("save_last_used_directory", QVariant(true)).toBool();
    enableBakFiles = settings.value("enable_bak_files", QVariant(true)).toBool();
    file_request_service_.set_current_directory(
                saveLastUsedDirectory ? settings.value("last_used_directory", QVariant(QString())).toString()
                                      : QString());
    enc.SetLibcurlPath(settings.value("libcurl_path", QVariant(QString())).toString().toStdString());

    QFont font;
    font.fromString(fontString);
    textEdit->setFont(font);

    resize(size);
    move(pos);
}

void MainWindow::writeSettings()
{
    QString configFile = accessRepositoryPath(kConfigFileName);
    if(configFile.isEmpty())
        return;

    QSettings settings(configFile, QSettings::IniFormat);
    settings.setValue("pos", pos());
    settings.setValue("size", size());
    settings.setValue("recent_files", QVariant(recent_files_service_.GetMaxFiles()));
    QStringList list;
    recent_files_service_.Serialize(list);
    settings.setValue("recent_file_list", QVariant(list));
    settings.setValue("font", QVariant(textEdit->font().toString()));
    settings.setValue("read_only", QVariant(readOnlyAct->isChecked()));
    settings.setValue("word_wrap", QVariant(wordWrapAct->isChecked()));
    settings.setValue("passphrase_generation", QVariant(passphraseGenerationSettings));
    settings.setValue("save_last_used_directory", QVariant(saveLastUsedDirectory));
    settings.setValue("enable_bak_files", QVariant(enableBakFiles));
    settings.setValue("last_used_directory", QVariant(saveLastUsedDirectory ? file_request_service_.get_current_directory() : QString()));
    settings.setValue("libcurl_path", QVariant(QString::fromStdString(enc.GetLibcurlPath())));
}

bool MainWindow::maybeSave()
{
    if (isWindowModified()) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, "EncryptPad",
                tr("The document has been modified.") + QString("\n") +
                tr("Do you want to save your changes?"),
                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
            return save();
        else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}

void MainWindow::makeDirty()
{
    if(curFile.isEmpty())
        return;
    setWindowModified(true);
    encryptionModified = true;
}

bool MainWindow::OpenPassphraseDialog(bool confirmationEnabled, std::string *passphrase)
{
    return loadHandler.OpenPassphraseDialog(confirmationEnabled, passphrase);
}

void MainWindow::EnterWaitState()
{
    isBusy = true;
    #ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QApplication::processEvents();
    #endif
    setAcceptDrops(false);
    this->setEnabled(false);
}

void MainWindow::ExitWaitState()
{
    #ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
    QApplication::processEvents();
    #endif
    this->setEnabled(true);
    isBusy = false;
    setAcceptDrops(true);
}

void MainWindow::ConvertToWindowsEOL(QString &in, QByteArray &out)
{
    QTextDocument *doc = textEdit->document();
    int count = doc->blockCount();
    out.reserve(in.size() + count - 1);
    QTextStream stm(&in);

    QString line = stm.readLine();
    out.append(line.toUtf8());
    while(!stm.atEnd())
    {
        out.push_back(0x0D);
        out.push_back(0x0A);

        QString line = stm.readLine();
        out.append(line.toUtf8());
    }

    // QTextStream behavior: if the last line is empty, the stream is atEnd
    // and we have no way of knowing if there was an empty line. So we add a line manually
    if(in.endsWith('\n'))
    {
        out.push_back(0x0D);
        out.push_back(0x0A);
    }
}

void MainWindow::startLoad(const QString &fileName, const QString &encryptionKeyFile, 
            std::string &passphrase, EncryptPad::PacketMetadata &metadata, std::string &kf_passphrase)
{
    EnterWaitState();
    load_state_machine_.Set(fileName, encryptionKeyFile, passphrase, metadata, kf_passphrase);
    load_state_machine_.BeginLoad();
    std::fill(std::begin(passphrase), std::end(passphrase), '0');
    std::fill(std::begin(kf_passphrase), std::end(kf_passphrase), '0');
}

void MainWindow::startSave(const QString &fileName, std::string &kf_passphrase)
{
    EncryptPadEncryptor::Result result = EncryptPadEncryptor::OK;
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    {
        QString str = textEdit->toPlainText();
        QByteArray byteArr;

        if(!windowsEol)
        {
            byteArr = str.toUtf8();
        }
        else
        {
            ConvertToWindowsEOL(str, byteArr);
        }

        Botan::SecureVector<byte> secureVect(reinterpret_cast<const byte*>(byteArr.constData()), byteArr.size());
        if(takeBakFile)
        {
            if(!TakeBakFile(fileName))
            {
                result = EncryptPadEncryptor::Result::BakFileMoveFailed;
            }
            takeBakFile = false;
        }

        if(result != EncryptPadEncryptor::Result::BakFileMoveFailed)
        {
            result = enc.Save(fileName.toUtf8().constData(), secureVect, 
                    encryptionKeyFile.toStdString(), persistEncryptionKeyPath,
                    &metadata, !kf_passphrase.empty() ? &kf_passphrase : nullptr);
        }
    }
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
    QString warningMessage;
    switch(result)
    {
    case EncryptPadEncryptor::Result::OK:
        break;
    case EncryptPadEncryptor::Result::CpadFileIOError:
    case EncryptPadEncryptor::Result::InvalidCpadFile:
        warningMessage = tr("Cannot save '%1'").arg(fileName);
        break;
    case EncryptPadEncryptor::Result::X2KeyIOError:
        warningMessage = tr("Cannot open the specified encryption key");
        break;
    case EncryptPadEncryptor::Result::InvalidX2File:
        warningMessage = tr("The specified encryption key is invalid");
        break;
    case EncryptPadEncryptor::Result::X2CurlIsNotFound:
        warningMessage = tr("Cannot download the encryption key. CURL tool is not found");
        break;
    case EncryptPadEncryptor::Result::X2CurlExitNonZero:
        warningMessage = tr("Cannot download the encryption key. CURL returned non zero exit code");
        break;
    case EncryptPadEncryptor::Result::EncryptionError:
        warningMessage = tr("Unknown encryption error");
        break;
    case EncryptPadEncryptor::Result::BakFileMoveFailed:
        warningMessage = tr("Cannot create bak file");
        break;
    case EncryptPadEncryptor::Result::InvalidKeyFilePassphrase:
        // Ask for the passphrase again
        enc.ClearKFPassphrase();
        saveFile(fileName);
        return;
    default:
        warningMessage = tr("Unknown error");
    }

    if(result != EncryptPadEncryptor::Result::OK)
    {
        QMessageBox::warning(
                    this,
                    "EncryptPad",
                    warningMessage
                    );

        statusBar()->showMessage(tr("Cannot save file"));
        saveSuccess = false;
        return;
    }

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File saved"), 2000);
    recent_files_service_.PushFile(fileName);
    saveSuccess = true;
    return;
}

void MainWindow::loadFile(const QString &fileName, bool force_kf_passphrase_request)
{
    isBusy = true;
    if(!loadHandler.LoadFile(fileName, force_kf_passphrase_request))
        isBusy = false;
}

void MainWindow::clearPassphrase(bool makeFileDirty)
{
    enc.SetIsPlainText();
    if(makeFileDirty)
        makeDirty();
}

void MainWindow::clearEncryptionKey(bool makeFileDirty)
{
    persistEncryptionKeyPath = false;
    setEncryptionKeyFile("");
    updateEncryptionKeyStatus();
    if(makeFileDirty)
        makeDirty();
}

void MainWindow::setPassphrase()
{
    bool result = OpenPassphraseDialog(true);
    if(result)
        makeDirty();
}

bool MainWindow::OpenSetEncryptionKeyDialogue()
{
    return loadHandler.OpenSetEncryptionKeyDialogue();
}

void MainWindow::setEncryptionKeyFile(const QString &file, bool clearKFKeyService)
{
    if(file != encryptionKeyFile && clearKFKeyService)
        enc.ClearKFPassphrase();

    encryptionKeyFile = file;
}

const QString &MainWindow::getEncryptionKeyFile() const
{
    return encryptionKeyFile;
}

void MainWindow::setFileProperties()
{
    FilePropertiesDialog dlg(this);
    dlg.SetUiFromMetadata(metadata);
    if(dlg.exec() == QDialog::Rejected || !dlg.GetIsDirty())
        return;

    dlg.UpdateMetadataFromUi(metadata);
    // if an algorithm has changed, the keys need to be regenerated.
    clearPassphrase();
}

void MainWindow::setEncryptionKey()
{
    bool result = OpenSetEncryptionKeyDialogue();
    if(result)
        makeDirty();
}

void MainWindow::replace()
{
    if(findDialog->isVisible())
    {
        findDialog->activateWindow();
        return;
    }

    if(replaceDialog->isVisible())
    {
        replaceDialog->activateWindow();
        return;
    }

    clearReplaceContext();
    QString selectedText = textEdit->textCursor().selectedText();
    if(!selectedText.isEmpty())
        replaceDialog->SetFind(selectedText);
    replaceDialog->show();
}

void MainWindow::readOnlyToggled(bool flag)
{
    textEdit->setReadOnly(flag);
}

void MainWindow::wordWrapToggled(bool flag)
{
    textEdit->setWordWrapMode(flag ? QTextOption::WordWrap : QTextOption::NoWrap);
}

void MainWindow::windowsEolToggled(bool flag)
{
    windowsEol = flag;
}

void MainWindow::search()
{
    if(findDialog->isVisible())
    {
        findDialog->activateWindow();
        return;
    }

    if(replaceDialog->isVisible())
    {
        replaceDialog->activateWindow();
        return;
    }

    QString selectedText = textEdit->textCursor().selectedText();
    if(!selectedText.isEmpty())
        findDialog->setFindWhat(selectedText);

    findDialog->show();
}

void MainWindow::generatePassphrase()
{
    PassphraseGenerationDialog dlg(this);
    dlg.setSettings(passphraseGenerationSettings);
    auto result = dlg.exec();
    if(result == QDialog::Rejected)
        return;

    passphraseGenerationSettings = dlg.getSettings();

    QTextCursor cursor = textEdit->textCursor();
    if(!dlg.getAllPassphrases())
    {
        cursor.insertText(dlg.getCurrentPassphrase());
    }
    else
    {
        QStringList list = dlg.getPassphrases();
        QString text;
        foreach(QString pwd, list)
        {
            if(!text.isEmpty())
                text.append(QChar::CarriageReturn);
            text.append(pwd);
        }
        cursor.insertText(text);
    }
}

void MainWindow::gotoTriggered()
{
    QTextCursor cursor = textEdit->textCursor();
    QTextDocument *doc = textEdit->document();
    int count = doc->blockCount();

    int lineNumber = cursor.blockNumber();


    int newLineNumber = QInputDialog::getInt(this, tr("Go to"), tr("Line number:"),
        lineNumber + 1, 1, count, 1, 0, kDefaultWindowFlags);

    newLineNumber = newLineNumber - 1;

    if(newLineNumber == lineNumber)
        return;

    QTextBlock block = doc->findBlockByNumber(newLineNumber);
    cursor = QTextCursor(block);
    textEdit->setTextCursor(cursor);
    textEdit->centerCursor();
    textEdit->setFocus();
}

void MainWindow::zoomIn()
{
    int pointSize = textEdit->font().pointSize();
    if(pointSize >= maxZoomIn)
    {
        statusBar()->showMessage(tr("Maximum zoom"));
        return;
    }

    textEdit->zoomIn();
    currentZoom++;
    statusBar()->showMessage(tr("Font size: %1").arg(QString::number(++pointSize)));
}

void MainWindow::zoomOut()
{
    int pointSize = textEdit->font().pointSize();
    if(pointSize <= minZoomOut)
    {
        statusBar()->showMessage(tr("Minimum zoom"));
        return;
    }

    textEdit->zoomOut();
    currentZoom--;
    statusBar()->showMessage(tr("Font size: ").arg(QString::number(--pointSize)));
}

void MainWindow::resetZoom()
{
    if(currentZoom == 0)
        return;

    else if(currentZoom > 0)
        textEdit->zoomOut(currentZoom);
    else if(currentZoom < 0)
        textEdit->zoomIn(-currentZoom);

    currentZoom = 0;
    int pointSize = textEdit->font().pointSize();
    statusBar()->showMessage(tr("Font size: ").arg(QString::number(pointSize)));
}

bool MainWindow::saveFile(const QString &fileName)
{
    saveSuccess = false;
    // SaveFile will call startSave that sets the saveSuccess flag
    if(!loadHandler.SaveFile(fileName))
        return false;

    return saveSuccess;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    curFile = fileName;
    if(fileName.length() > 0)
    {
        QFileInfo fileInfo(fileName);
        lastModified = fileInfo.lastModified();
        encryptionModified = false;
    }

    textEdit->document()->setModified(false);
    setWindowModified(false);

    QString shownName = curFile;
    if (curFile.isEmpty())
        shownName = "untitled.epd";
    setWindowFilePath(shownName);
    updateLineStatus();
}

void MainWindow::updateEncryptionKeyStatus()
{
    QString key_protected = tr("Key protected");
    QString key_not_set = tr("Key not set");
    QString persisted = tr("persisted");

    QString str = encryptionKeyFile.length() > 0 ?
        (QString("<b>") + key_protected) :
        (QString("<span style=\"color:#FF0000;\">") + key_not_set + QString("</span>"));

    str += persistEncryptionKeyPath ? (QString(" (") + persisted + QString(")</b>"))
        : QString("</b>");

    if(encryptionKeyFile.length() == 0)
        clearEncryptionKeyAct->setEnabled(false);
    else
        clearEncryptionKeyAct->setEnabled(true);
    encryptionKeySet->setText(str);
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MainWindow::setWindowsEol(bool flag)
{
    windowsEol = flag;
    windowsEolAct->setChecked(flag);
}

void MainWindow::openFileEncryption()
{
    FileEncryptionDialog dlg(this, file_request_service_);
    dlg.exec();
}
