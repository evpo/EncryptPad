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
#ifndef SET_KEY_DIALOG_H
#define SET_KEY_DIALOG_H

#include <QDialog>
#include <QStringList>
#include <QFileSystemModel>
#include "file_request_service.h"

namespace Ui {
class SetKeyDialog;
}

class SetKeyDialog : public QDialog
{
    Q_OBJECT

public:

    explicit SetKeyDialog(QWidget *parent, FileRequestService &file_request_service);
    ~SetKeyDialog();
    QString GetKeyFilePath() const;
    bool GetIsKeyPathPersisted() const;
    void SetIsKeyPathPersisted(bool persisted);
    void SetKeyFilePath(const QString &path);
    void SetRepositoryListModel(QFileSystemModel &repository_list_model, const QModelIndex &index);
public slots:
    void on_uiRepositoryListView_current_changed(QModelIndex index, QModelIndex);
private slots:
    void on_uiFileDialog_clicked();

    void on_uiUseKeyGroupBox_toggled(bool checked);

private:
    Ui::SetKeyDialog *ui;
    QFileSystemModel *model_;
    FileRequestService &file_request_service_;
};

#endif // SET_KEY_DIALOG_H
