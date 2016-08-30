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
    int recentFiles;

    void updateFont();
public:
    void setFont(QFont newFont);

    const QFont &getFont() const
    {
        return font;
    }

    void setRecentFiles(int newRecentFiles);
    int getRecentFiles() const;

    void setWordWrap(bool val);
    bool getWordWrap() const;

    bool getSaveLastUsedDirectory() const;
    void setSaveLastUsedDirectory(bool val);

    bool getEnableBakFiles() const;
    void setEnableBakFiles(bool val);

    QString getLibcurlPath() const;
    void setLibcurlPath(const QString &path);

    int getIterations() const;
    void setIterations(int iterations);

public slots:
    void selectFont();
private slots:
    void on_uiLibcurlBrowse_clicked();

private:
    Ui::PreferencesDialog *ui;
};

#endif // PREFERENCES_DIALOG_H
