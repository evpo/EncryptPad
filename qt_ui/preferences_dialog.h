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
#ifndef PREFERENCES_DIALOG_H
#define PREFERENCES_DIALOG_H

#include <QDialog>
#include <QFont>
#include "file_request_service.h"
#include "packet_composer.h"
#include "preferences.h"

namespace Ui {
class PreferencesDialog;
}

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(QWidget *parent = 0);
    ~PreferencesDialog();
private:
    QFont font;
    FileRequestService fileRequestService;
    EncryptPad::PacketMetadata defaultFileProperties;
    EncryptPad::PacketMetadata keyFileProperties;
    bool defaultFilePropertiesChanged;

    void updateFont();
public:
    void set(PersistentPreferences &value);
    void get(PersistentPreferences &value);
    bool getDefaultFilePropertiesChanged() const;

public slots:
    void selectFont();
    void editDefaultFileProperties();
    void editKeyFileProperties();
private slots:
    void on_uiLibcurlBrowse_clicked();

private:
    Ui::PreferencesDialog *ui;
};

#endif // PREFERENCES_DIALOG_H
