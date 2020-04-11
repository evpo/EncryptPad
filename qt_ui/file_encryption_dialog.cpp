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
#include "file_encryption_dialog.h"
#include <ctime>
#include <QFileDialog>
#include <QFile>
#include "ui_file_encryption_dialog.h"
#include "file_name_helper.h"
#include "encryptmsg/algo_spec.h"
#include "file_properties_dialog.h"
#include "set_encryption_key.h"
#include "file_encryption.h"
#include "common_definitions.h"

using namespace EncryptPad;
namespace
{
    void ClearFileSpecificParameters(PacketMetadata &metadata)
    {
        metadata.file_name.clear();
        metadata.file_date = 0;
        metadata.cannot_use_wad = false;
        metadata.salt.clear();
        metadata.key_only = false;
    }
}

FileEncryptionDialog::FileEncryptionDialog(QWidget *parent, FileRequestService &fileRequestService_p) :
    QDialog(parent, kDefaultWindowFlags),
    ui(new Ui::FileEncryptionDialog),
    loadAdapter(this),
    loadHandler(this, loadAdapter, metadata),
    async(metadata),
    keyService(1),
    fileRequestService(fileRequestService_p),
    isWorkInProgress(false)
{
    ui->setupUi(this);

    QObject::connect(&async, SIGNAL(WorkDone()), this, SLOT(WorkDone()));
    QObject::connect(&async, SIGNAL(Progress(int)), this, SLOT(Progress(int)));

    using namespace EncryptPad;
    metadata = {};
    metadata.cipher_algo = kDefaultCipherAlgo;
    metadata.hash_algo = kDefaultHashAlgo;
    metadata.compression = kDefaultCompression;
    metadata.iterations = kDefaultIterations;
    metadata.persist_key_path = false;
    metadata.is_binary = true;
    metadata.file_name = "_CONSOLE";
}

FileEncryptionDialog::~FileEncryptionDialog()
{
    delete ui;
}

QString FileEncryptionDialog::EncryptionFilePath() const
{
    return ui->uiKeyFilePath->text();
}

void FileEncryptionDialog::EncryptionFilePath(const QString &path)
{
    if(ui->uiKeyFilePath->text() != path)
    {
        kfKeyService.Clear();
    }

    return ui->uiKeyFilePath->setText(path);
}

void FileEncryptionDialog::SetPassphrase(const char *pwd, EncryptPad::PacketMetadata &metadata)
{
    keyService.Clear();
    std::string passphrase(pwd);
    keyService.ChangePassphrase(
            passphrase, 
            metadata.hash_algo, 
            GetAlgoSpec(metadata.cipher_algo).key_size,
            metadata.iterations);

    std::fill(std::begin(passphrase), std::end(passphrase), '0');

    // if there was key_only before, switch it off
    metadata.key_only = false;
}

void FileEncryptionDialog::SetDefaultFileParameters(const EncryptPad::PacketMetadata &default_metadata)
{
    metadata = default_metadata;
    metadata.persist_key_path = false;
    metadata.is_binary = true;
    metadata.file_name = "_CONSOLE";
}

bool FileEncryptionDialog::IsPassphraseNotSet() const
{
    return !keyService.IsPassphraseSet();
}

bool FileEncryptionDialog::HasKeyFilePassphrase() const
{
    return kfKeyService.IsPassphraseSet();
}

FileRequestService &FileEncryptionDialog::GetFileRequestService()
{
    return fileRequestService;
}

void FileEncryptionDialog::ClearPassphrase()
{
    keyService.Clear();
}

// fileName is output file
void FileEncryptionDialog::StartEncryption(const QString &fileName, std::string &kf_passphrase)
{
    using namespace EncryptPad;

    QString inputFile = ui->uiInputFile->text();
    assert(!inputFile.isEmpty());
    QFileInfo file(inputFile);
    metadata.file_name = file.fileName().toStdString();
    metadata.file_date = static_cast<FileDate>(time(NULL));

    metadata.cannot_use_wad = IsGpgFormat(fileName);
    metadata.key_file = EncryptionFilePath().toStdString();
    if(!metadata.key_file.empty() && !keyService.UnusedKeysExist())
        metadata.key_only = true;

    bool encryption = true;
    async.Set(encryption, ui->uiInputFile->text(), fileName, &keyService, std::string(), &kfKeyService, kf_passphrase);
    emit ToggleWorkInProgress(true);
    async.Start();
}

void FileEncryptionDialog::StartDecryption(const QString &fileName, const QString &encryptionKeyFile, std::string &passphrase,
                                           EncryptPad::PacketMetadata &metadata, std::string &kf_passphrase)
{
    using namespace EncryptPad;

    metadata.cannot_use_wad = IsGpgFormat(fileName);
    metadata.file_name = fileName.toStdString();
    metadata.key_file = encryptionKeyFile.toStdString();
    if(!metadata.key_file.empty() && (passphrase.empty() && !keyService.IsPassphraseSet()))
        metadata.key_only = true;

    bool encryption = true;
    async.Set(!encryption, fileName, ui->uiOutputFile->text(), &keyService, passphrase, &kfKeyService, kf_passphrase);
    emit ToggleWorkInProgress(true);
    async.Start();
}

QString FileEncryptionDialog::getSelectedExtension() const
{
    QString ext;
    if(ui->uiEpdRadio->isChecked())
    {
        ext = ".epd";
    }
    else if(ui->uiGpgRadio->isChecked())
    {
        ext = ".gpg";
    }
    else if(ui->uiAscRadio->isChecked())
    {
        ext = ".asc";
    }
    return ext;
}

void FileEncryptionDialog::suggestOutput()
{
    QString inputFile = ui->uiInputFile->text();
    if(inputFile.isEmpty())
        return;

    if(!ui->uiOutputFile->text().isEmpty())
        return;

    if(ui->uiEncryptRadio->isChecked())
    {
        // Encrypt
        ui->uiOutputFile->setText(inputFile + getSelectedExtension());
    }
    else
    {
        //Decrypt
        QFileInfo file(inputFile);
        QString outputFile;
        outputFile = file.path() + "/"; // forward slash works in all cases

        if(IsEncryptPadFormat(inputFile) || IsGpgFormat(inputFile))
        {
            outputFile += file.completeBaseName();
        }
        else
        {
            outputFile += "output";
        }

        ui->uiOutputFile->setText(outputFile);
    }
}

// void FileEncryptionDialog::on_uiEpdRadio_toggled(bool toggled)
void FileEncryptionDialog::on_toggleExtension(bool toggled)
{
    QString file = ui->uiOutputFile->text();
    QString epd(".epd");
    QString gpg(".gpg");
    QString asc(".asc");
    if(file.isEmpty() || file.length() <= epd.length())
        return;

    QString ext = file.right(epd.length()).toLower();
    if(ext != epd && ext != gpg && ext != asc)
        return;

    file = file.left(file.length() - epd.length());
    ui->uiOutputFile->setText(file + getSelectedExtension());
}

void FileEncryptionDialog::on_uiInputBrowse_clicked()
{
    QString inputFile = ui->uiInputFile->text();
    QString allFilesFilter = GetAllFilesFilter();

    FileRequestSelection selection = fileRequestService.RequestExistingFile(
                this,
                tr("Input File"),
                inputFile,
                GetOpenDialogFilter(),
                ui->uiEncryptRadio->isChecked() ? &allFilesFilter : nullptr);

    if(selection.cancelled)
        return;

    ui->uiInputFile->setText(selection.file_name);
    suggestOutput();
}

void FileEncryptionDialog::on_uiOutputBrowse_clicked()
{
    QString outputFile = ui->uiOutputFile->text();
    FileRequestSelection selection = fileRequestService.RequestNewFile(
                this,
                tr("Output File"),
                outputFile,
                ui->uiDecryptRadio->isChecked() ? GetAllFilesFilter() : GetSaveDialogFilter(),
                nullptr);

    if(selection.cancelled)
        return;

    ui->uiOutputFile->setText(selection.file_name);
}

void FileEncryptionDialog::on_uiParameters_clicked()
{
    FilePropertiesDialog dlg(this);
    dlg.SetUiFromMetadata(metadata);
    if(dlg.exec() == QDialog::Rejected || !dlg.GetIsDirty())
        return;

    dlg.UpdateMetadataFromUi(metadata);
}

void FileEncryptionDialog::on_uiSelectKeyFile_clicked()
{
    loadHandler.OpenSetEncryptionKeyDialogue();
}

void FileEncryptionDialog::on_uiCancel_clicked()
{
    async.Cancel();
}

void FileEncryptionDialog::on_uiStart_clicked()
{
    QString inputFile = ui->uiInputFile->text();
    QString outputFile = ui->uiOutputFile->text();
    if(inputFile.isEmpty() || outputFile.isEmpty())
        return;

    if(ui->uiPassphrase->text() != ui->uiConfirmPassphrase->text())
    {
        QMessageBox::warning(
                this,
                tr("EncryptPad"),
                tr("Confirmation passphrase does not match."));
        return;
    }

    if(!QFile::exists(inputFile))
    {
        QMessageBox::warning(
                this,
                tr("EncryptPad"),
                tr("%1 does not exist.").arg(inputFile));
        return;
    }

    if(QFile::exists(outputFile))
    {
        auto ret = QMessageBox::warning(
                this,
                tr("EncryptPad"),
                tr("%1 already exists.\nDo you want to replace it?").arg(outputFile),
                QMessageBox::Yes | QMessageBox::No
                );

        if(ret == QMessageBox::No)
            return;
    }

    ClearFileSpecificParameters(metadata);

    if(IsEncryption())
    {
        if(!ui->uiPassphrase->text().isEmpty())
        {
            QByteArray byte_array = ui->uiPassphrase->text().toUtf8();
            const char *pwd = byte_array.constData();

            std::string passphrase(pwd);
            keyService.ChangePassphrase(passphrase, metadata.hash_algo, 
                GetAlgoSpec(metadata.cipher_algo).key_size,
                metadata.iterations);
        }

        loadHandler.SaveFile(ui->uiOutputFile->text());
    }
    else
    {
        loadHandler.LoadFile(ui->uiInputFile->text());
    }
}

bool FileEncryptionDialog::IsEncryption() const
{
    return ui->uiEncryptRadio->isChecked();
}

void FileEncryptionDialog::WorkDone()
{
    using namespace EncryptPad;

    emit ToggleWorkInProgress(false);
//    this->setEnabled(true);


    bool success = false;
    bool rejected = false;
    bool request_kf_passphrase = false;

    switch(async.get_result())
    {
        case EpadResult::Success:
            success = true;
            break;
        case EpadResult::InvalidSurrogateIV:
            keyService.Clear();
            break;
        case EpadResult::IOErrorKeyFile:
            QMessageBox::warning(
                        this,
                        "EncryptPad",
                        tr("Cannot open the encryption key"));
            rejected = true;
            break;
        case EpadResult::InvalidKeyFile:
            QMessageBox::warning(
                        this,
                        "EncryptPad",
                        tr("The encryption key is invalid"));
            rejected = true;
            break;
        case EpadResult::CurlIsNotFound:
            QMessageBox::warning(
                        this,
                        "EncryptPad",
                        tr("Cannot download the encryption key. CURL tool is not found."));
            rejected = true;
            break;
        case EpadResult::CurlExitNonZero:
            QMessageBox::warning(
                        this,
                        "EncryptPad",
                        tr("Cannot download the key. CURL returned non zero exit code"));
            rejected = true;
            break;
        case EpadResult::KeyFileNotSpecified:
        case EpadResult::KeyIsRequiredForSaving:
            rejected = !loadHandler.OpenSetEncryptionKeyDialogue();
            break;
        case EpadResult::InvalidKeyFilePassphrase:
            kfKeyService.Clear();

            if(IsEncryption())
            {
                QMessageBox::warning(
                        this,
                        "EncryptPad",
                        tr("Invalid passphrase for key file"));
                rejected = true;
            }
            else
            {
                request_kf_passphrase = true;
            }
            break;

        case EpadResult::Cancelled:
            QMessageBox::warning(
                    this,
                    "EncryptPad",
                    tr("The job has been cancelled"));
            rejected = true;
            break;

        default:
            rejected = true;
            break;
    }


    if(!success && !rejected && !IsEncryption())
    {
        loadHandler.LoadFile(ui->uiInputFile->text(), request_kf_passphrase);
    }
    else if(success)
    {
        keyService.Clear();
        QMessageBox::information(
                this,
                "EncryptPad",
                IsEncryption() ? tr("File has been encrypted successfully.") :
                    tr("File has been decrypted successfully."));
        metadata.salt.clear();
    }
    else
    {
        keyService.Clear();
        metadata.salt.clear();
    }
}

void FileEncryptionDialog::Progress(int progress_percent)
{
    ui->uiProgressBar->setValue(progress_percent);
}

void FileEncryptionDialog::on_actionSwitchDirection_triggered()
{
    ui->uiPassphrase->clear();
    ui->uiConfirmPassphrase->clear();
    QString filePath = ui->uiOutputFile->text();
    ui->uiOutputFile->setText(ui->uiInputFile->text());
    ui->uiInputFile->setText(filePath);
}

void FileEncryptionDialog::on_actionIOChange_triggered()
{
    QString inputFile = ui->uiInputFile->text();
    QString outputFile = ui->uiOutputFile->text();
    ui->uiStart->setEnabled(!inputFile.isEmpty() && !outputFile.isEmpty() && inputFile != outputFile);
}

void FileEncryptionDialog::on_uiInputFile_editingFinished()
{
    suggestOutput();
}

void FileEncryptionDialog::closeEvent(QCloseEvent *event)
{
    if(isWorkInProgress)
        event->ignore();
}

void FileEncryptionDialog::on_toggleWorkInProgress(bool value)
{
    isWorkInProgress = value;
    ui->uiProgressBar->setValue(0);
}
