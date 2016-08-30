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
#include "preferences_dialog.h"
#include "ui_preferences_dialog.h"
#include <QFontDialog>
#include "common_definitions.h"
#include "file_name_helper.h"

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent, kDefaultWindowFlags),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);
    setMinimumSize(sizeHint());
    adjustSize();
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void PreferencesDialog::updateFont()
{
    QString fontText = font.family();
    fontText += tr(" (Size: ");
    fontText += QString::number(font.pointSize());
    fontText += tr(")");
    ui->uiFontFamily->setText(fontText);
}

void PreferencesDialog::setFont(QFont newFont)
{
    font = newFont;
    updateFont();
}

int PreferencesDialog::getIterations() const
{
    return ui->uiIterations->value();
}

void PreferencesDialog::setIterations(int iterations)
{
    ui->uiIterations->setValue(iterations);
}

void PreferencesDialog::setRecentFiles(int newRecentFiles)
{
    ui->uiRecentFiles->setValue(newRecentFiles);
}

int PreferencesDialog::getRecentFiles() const
{
    return ui->uiRecentFiles->value();
}

void PreferencesDialog::setWordWrap(bool val)
{
    ui->uiWordWrap->setChecked(val);
}

bool PreferencesDialog::getWordWrap() const
{
    return ui->uiWordWrap->isChecked();
}

bool PreferencesDialog::getSaveLastUsedDirectory() const
{
    return ui->uiLastUsedDirectory->isChecked();
}

void PreferencesDialog::setSaveLastUsedDirectory(bool val)
{
    return ui->uiLastUsedDirectory->setChecked(val);
}

bool PreferencesDialog::getEnableBakFiles() const
{
    return ui->uiEnableBakFiles->isChecked();
}

void PreferencesDialog::setEnableBakFiles(bool val)
{
    return ui->uiEnableBakFiles->setChecked(val);
}

QString PreferencesDialog::getLibcurlPath() const
{
    return ui->uiLibcurl->text();
}

void PreferencesDialog::setLibcurlPath(const QString &path)
{
    ui->uiLibcurl->setText(path);
}

void PreferencesDialog::selectFont()
{
    bool ok;
    QFont selected_font = QFontDialog::getFont(&ok, font, this, tr("Select Font"));
    if(!ok)
        return;

    font = selected_font;
    updateFont();
}

void PreferencesDialog::on_uiLibcurlBrowse_clicked()
{
    FileRequestSelection selection = fileRequestService.RequestExistingFile(
                this, tr("Path to Libcurl"), ui->uiLibcurl->text(), GetLibcurlFilter());
    if(selection.cancelled)
        return;

    ui->uiLibcurl->setText(selection.file_name);
}
