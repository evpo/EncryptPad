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
#include "set_encryption_key.h"
#include <QFileSystemModel>
#include <QDir>
#include "set_key_dialog.h"
#include "repository.h"

namespace EncryptPad
{
    bool SetEncryptionKey(QWidget *parent, const QString &key_file_path, bool persist_key_path,
                          FileRequestService &file_request_service, EncryptionKeySelectionResult &result_out)
    {
        QFileSystemModel model(parent);
        SetKeyDialog dlg(parent, file_request_service);
        dlg.SetIsKeyPathPersistent(persist_key_path);
        dlg.SetKeyFilePath(key_file_path);
        QString repositoryPath = QString::fromStdString(GetRepositoryPath());
        if(!repositoryPath.isEmpty())
        {
            model.setRootPath(repositoryPath);
            QStringList list;
            list.push_back(QString("*.key"));
            model.setNameFilters(list);
            model.setNameFilterDisables(false);
            auto index = model.index(repositoryPath);
            dlg.SetRepositoryListModel(model, index);
        }

        if(dlg.exec() == QDialog::Rejected)
            return false;

        result_out.key_file_path = dlg.GetKeyFilePath();
        result_out.persist_key_path = dlg.GetIsKeyPathPersistent();
        if(result_out.key_file_path.isNull() || result_out.key_file_path.isEmpty())
        {
            result_out.persist_key_path = false;
        }

        return true;
    }
}
