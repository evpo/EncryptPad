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
#include "load_save_handler.h"
#include <QtGui>
#include <QtGlobal>
#include <QString>
#include <QObject>
#include <QMessageBox>
#include <QDialog>
#include <QFile>
#include "file_encryption.h"
#include "file_name_helper.h"
#include "get_password_dialog.h"
#include "confirm_password_dialog.h"
#include "get_password_dialog.h"
#include "set_encryption_key.h"
#include "get_password_or_key_dialog.h"

namespace EncryptPad
{
    const char *kKeyFilePassphraseWindowTitle = QT_TRANSLATE_NOOP("LoadSaveHandler", "Password for Key File");

    bool LoadHandler::LoadFile(const QString &fileName, bool force_kf_passphrase_request)
    {
        bool can_be_passphrase_protected = true;

        bool is_wad_file = false;
        std::string key_file;
        bool isGpg = IsGpgFormat(fileName);
        bool isCpad = IsCryptPadFormat(fileName);

        if(!QFile::exists(fileName))
        {
            QMessageBox::warning(
                    parent,
                    "EncryptPad",
                    qApp->translate("LoadSaveHandler", "Cannot open the file because it does not exist"));

            return false;
        }

        // Only check if it's an encrypt pad format
        if(!isCpad && !isGpg)
        {
            can_be_passphrase_protected = EncryptPad::CheckIfPassphraseProtected(
                        fileName.toStdString(), is_wad_file, key_file);
        }

        assert(!can_be_passphrase_protected || !is_wad_file);

        bool isPwd = !client.IsPassphraseNotSet();
        bool isKey = !client.EncryptionKeyFile().isEmpty();

        if(!isCpad && !can_be_passphrase_protected && !is_wad_file && (isPwd || isKey))
        {
            // this is an unencrypted file or a corrupted encrypted file. Reset encryption parameters.
            client.SetIsPlainText();
            client.PersistEncryptionKeyPath(false);
            client.EncryptionKeyFile("");
            client.UpdateEncryptionKeyStatus();
        }

        std::string passphrase;
        if(isGpg)
        {
            // Password and key file are both or none. We need to display the key or password dialog.
            if(!(isPwd ^ isKey))
            {
                GetPasswordOrKeyDialog dlg(parent, client.GetFileRequestService());
                if(dlg.exec() == QDialog::Rejected)
                    return false;

                client.EncryptionKeyFile("");
                client.PersistEncryptionKeyPath(false);
                client.SetIsPlainText();

                if(dlg.IsPasswordSelected())
                {
                    QString pwdString = dlg.GetPassword();
                    QByteArray byte_array = pwdString.toUtf8();
                    const char *pwd = byte_array.constData();
                    passphrase = pwd;
                    if(passphrase.size() > 0)
                        client.SetPassword(pwd, metadata);
                }
                else
                {
                    client.EncryptionKeyFile(dlg.GetKeyFilePath());
                    client.PersistEncryptionKeyPath(dlg.GetPersistKeyPath());
                }

                client.UpdateEncryptionKeyStatus();
            }
        }
        else if(client.IsPassphraseNotSet() && can_be_passphrase_protected && !OpenPasswordDialog(false, &passphrase))
        {
            return false;
        }

        if(is_wad_file) 
        {
            if(!key_file.empty())
            {
                client.EncryptionKeyFile(QString::fromStdString(key_file));
                client.PersistEncryptionKeyPath(true);
                client.UpdateEncryptionKeyStatus();
            }
            else if(client.EncryptionKeyFile().isEmpty() && !OpenSetEncryptionKeyDialogue())
            {
                return false;
            }
        }

        isPwd = !client.IsPassphraseNotSet();
        isKey = !client.EncryptionKeyFile().isEmpty();

        std::string kf_passphrase;
        if((isKey && !client.HasKeyFilePassphrase()) || force_kf_passphrase_request)
        {
            if(!OpenPasswordDialog(false, &kf_passphrase, false, qApp->translate("LoadSaveHandler", kKeyFilePassphraseWindowTitle)))
                return false;
        }

        if(!isPwd && isKey)
        {
            metadata.key_only = true;
        }

        client.StartLoad(fileName, client.EncryptionKeyFile(), passphrase, metadata, kf_passphrase);
        std::fill(std::begin(passphrase), std::end(passphrase), '0');
        std::fill(std::begin(kf_passphrase), std::end(kf_passphrase), '0');
        return true;
    }

    bool LoadHandler::SaveFile(const QString &fileName)
    {
        bool passwordSet = !client.IsPassphraseNotSet();
        bool isGpg = IsGpgFormat(fileName);
        bool isEncryptedFormat = IsCryptPadFormat(fileName) || IsEncryptPadFormat(fileName) || isGpg;

        if(isGpg)
        {
            if(client.PersistEncryptionKeyPath())
            {
                auto ret = QMessageBox::warning(
                        parent,
                        "EncryptPad",
                        qApp->translate("LoadSaveHandler", "GPG format does not support persisted key path.") 
                            + QString("\n") +
                            qApp->translate("LoadSaveHandler", "Do you want to disable it?"),
                        QMessageBox::Ok | QMessageBox::Cancel
                        );

                if(ret == QMessageBox::Cancel)
                    return false;

                client.PersistEncryptionKeyPath(false);
                client.UpdateEncryptionKeyStatus();
            }

            if(passwordSet && !client.EncryptionKeyFile().isEmpty())
            {
                QMessageBox::warning(
                        parent,
                        "EncryptPad",
                        qApp->translate("LoadSaveHandler", "GPG format does not support the password and key file double protection.")
                            + QString("\n") +
                        qApp->translate( "LoadSaveHandler", "Use EPD format or disable either password or key protection."));

                return false;
            }
            else if(!passwordSet && client.EncryptionKeyFile().isEmpty() && !OpenPasswordDialog(true))
            {
                return false;
            }

            assert(client.IsPassphraseNotSet() || client.EncryptionKeyFile().isEmpty());
            assert(!client.PersistEncryptionKeyPath());
        }
        else if(!passwordSet && isEncryptedFormat && !OpenPasswordDialog(true))
        {
            return false;
        }

        passwordSet = !client.IsPassphraseNotSet();

        if(isEncryptedFormat && !passwordSet && client.EncryptionKeyFile().isEmpty())
        {
            auto ret = QMessageBox::warning(
                    parent,
                    "EncryptPad",
                    qApp->translate("LoadSaveHandler", "Neither a key file nor password is set. The file is going to be saved UNENCRYPTED."),
                    QMessageBox::Ok | QMessageBox::Cancel
                    );

            if(ret == QMessageBox::Cancel)
                return false;
        }

        std::string kf_passphrase;

        if(!client.EncryptionKeyFile().isEmpty() && !client.HasKeyFilePassphrase() &&
                !OpenPasswordDialog(false, &kf_passphrase, false, kKeyFilePassphraseWindowTitle))
        {
            return false;
        }

        metadata.cannot_use_wad = isGpg;
        client.StartSave(fileName, kf_passphrase);
        std::fill(std::begin(kf_passphrase), std::end(kf_passphrase), '0');
        return true;
    }

    bool LoadHandler::OpenPasswordDialog(bool confirmationEnabled, std::string *passphrase, bool set_client_password, const QString &title)
    {
        if(passphrase)
            passphrase->clear();

        QString pwdString;
        if(confirmationEnabled)
        {
            ConfirmPasswordDialog dlg(parent);
            dlg.setWindowTitle(title);
            if(dlg.exec() == QDialog::Rejected)
                return false;
            pwdString = dlg.GetPassword();
        }
        else
        {
            GetPasswordDialog dlg(parent);
            dlg.setWindowTitle(title);
            if(dlg.exec() == QDialog::Rejected)
                return false;
            pwdString = dlg.GetPassword();
        }

        if(pwdString.isEmpty())
        {
            if(set_client_password)
                client.SetIsPlainText();
            return true;
        }

        QByteArray byte_array = pwdString.toUtf8();
        const char *pwd = byte_array.constData();

        if(set_client_password)
            client.SetPassword(pwd, metadata);

        if(passphrase)
            *passphrase = pwd;
        return true;
    }

    bool LoadHandler::OpenSetEncryptionKeyDialogue()
    {
        EncryptionKeySelectionResult selection;
        if(!SetEncryptionKey(parent, client.EncryptionKeyFile(), client.PersistEncryptionKeyPath(),
                             client.GetFileRequestService(), selection))
        {
            return false;
        }

        client.EncryptionKeyFile(selection.key_file_path);
        client.PersistEncryptionKeyPath(selection.persist_key_path);

        client.UpdateEncryptionKeyStatus();

        return true;
    }
}
