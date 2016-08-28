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
#ifndef FILE_ENCRYPTION_DIALOG_H
#define FILE_ENCRYPTION_DIALOG_H

#include <QDialog>
#include <QString>
#include "packet_composer.h"
#include "file_encryption_dlg_adapter.h"
#include "load_save_handler.h"
#include "file_dlg_async.h"
#include "file_request_service.h"

namespace Ui {
class FileEncryptionDialog;
}

class FileEncryptionDialog : public QDialog
{
private:
    friend class FileEncryptionDlgAdapter;

private:
    Q_OBJECT

public:
    explicit FileEncryptionDialog(QWidget *parent, FileRequestService &fileRequestService_p);
    ~FileEncryptionDialog();

    QString EncryptionFilePath() const;
    void EncryptionFilePath(const QString &path);

    bool PersistKeyPath() const
    {
        return metadata.persist_key_path;
    }

    void PersistKeyPath(bool flag)
    {
        metadata.persist_key_path = flag;
    }

    void StartDecryption(const QString &fileName, const QString &encryptionKeyFile,
        std::string &passphrase, EncryptPad::PacketMetadata &metadata, std::string &kf_passphrase);

    void StartEncryption(const QString &fileName, std::string &kf_passphrase);

    void SetPassphrase(const char *pwd, EncryptPad::PacketMetadata &metadata);
    void ClearPassphrase();

    bool IsPassphraseNotSet() const;

    bool HasKeyFilePassphrase() const;

    FileRequestService &GetFileRequestService();

    void SetDefaultIterations(int defaultIterations);

private slots:
    void WorkDone();

    void on_uiInputBrowse_clicked();

    void on_uiOutputBrowse_clicked();

    void on_uiParameters_clicked();

    void on_uiSelectKeyFile_clicked();

    void on_uiStart_clicked();

    void on_actionSwitchDirection_triggered();

    void on_actionIOChange_triggered();

    void on_uiInputFile_editingFinished();

private:
    Ui::FileEncryptionDialog *ui;
    EncryptPad::PacketMetadata metadata;
    FileEncryptionDlgAdapter loadAdapter;
    EncryptPad::LoadHandler loadHandler;
    FileDlgAsync async;
    EncryptPad::KeyService keyService;
    EncryptPad::KeyService kfKeyService;
    FileRequestService &fileRequestService;

    bool IsEncryption() const;
    void suggestOutput();

};

#endif // FILE_ENCRYPTION_DIALOG_H
