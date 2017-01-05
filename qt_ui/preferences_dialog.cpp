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
#include "file_properties_dialog.h"

namespace
{
    enum class BinderDirection
    {
        Set,
        Get
    };

    void BindControl(QSpinBox *cnt, int &value, BinderDirection direction)
    {
        if(direction == BinderDirection::Get)
        {
            value = cnt->value();
        }
        else if(direction == BinderDirection::Set)
        {
            cnt->setValue(value);
        }
    }

    void BindControl(QCheckBox *cnt, bool &value, BinderDirection direction)
    {
        if(direction == BinderDirection::Get)
        {
            value = cnt->isChecked();
        }
        else if(direction == BinderDirection::Set)
        {
            cnt->setChecked(value);
        }
    }

    void BindControl(QLineEdit *cnt, QString &value, BinderDirection direction)
    {
        if(direction == BinderDirection::Get)
        {
            value = cnt->text();
        }
        else if(direction == BinderDirection::Set)
        {
            cnt->setText(value);
        }
    }

    void BindControl(QComboBox *cnt, int &value, BinderDirection direction)
    {
        if(direction == BinderDirection::Get)
        {
            value = cnt->currentData().toInt();
        }
        else if(direction == BinderDirection::Set)
        {
            int index = cnt->findData(value);
            cnt->setCurrentIndex(index);
        }
    }

    struct ControlBinder
    {
        BinderDirection direction;
        template<class L, class R>
            void Bind(L *control, R &value)
            {
                BindControl(control, value, direction);
            }
        ControlBinder(BinderDirection d):direction(d){}
    };

    void Bind(Ui::PreferencesDialog &ui, ControlBinder &binder, PersistentPreferences &value)
    {
        binder.Bind(ui.uiRecentFiles, value.recentFiles);
        binder.Bind(ui.uiWordWrap, value.wordWrap);
        binder.Bind(ui.uiLastUsedDirectory, value.saveLastUsedDirectory);
        binder.Bind(ui.uiEnableBakFiles, value.enableBakFiles);
        binder.Bind(ui.uiLibcurl, value.libCurlPath);
        binder.Bind(ui.uiLibcurlParameters, value.libCurlParameters);
        binder.Bind(ui.uiWindowsEol, value.windowsEol);
        binder.Bind(ui.uiS2KResultsPoolSize, value.s2kResultsPoolSize);
        binder.Bind(ui.uiKeyFileLength, value.kfKeyLength);
    }

    void ParseComboBoxData(QComboBox *combo)
    {
        for(int i = 0; i < combo->count(); i++)
        {
            bool ok = false;
            int data = combo->itemText(i).toInt(&ok);
            assert(ok);
            combo->setItemData(i, data);
        }
    }
}

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent, kDefaultWindowFlags),
    defaultFilePropertiesChanged(false),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);
    setMinimumSize(sizeHint());
    adjustSize();
    ParseComboBoxData(ui->uiS2KResultsPoolSize);
    ParseComboBoxData(ui->uiKeyFileLength);
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

bool PreferencesDialog::getDefaultFilePropertiesChanged() const
{
    return defaultFilePropertiesChanged;
}

void PreferencesDialog::updateFont()
{
    QString fontText = font.family();
    fontText += tr(" (Size: ");
    fontText += QString::number(font.pointSize());
    fontText += tr(")");
    ui->uiFontFamily->setText(fontText);
}

void PreferencesDialog::get(PersistentPreferences &value)
{
    value.defaultFileProperties = defaultFileProperties;
    value.keyFileProperties = keyFileProperties;
    value.font = font;
    ControlBinder binder(BinderDirection::Get);
    Bind(*ui, binder, value);
}

void PreferencesDialog::set(PersistentPreferences &value)
{
    defaultFileProperties = value.defaultFileProperties;
    keyFileProperties = value.keyFileProperties;
    font = value.font;
    updateFont();
    ControlBinder binder(BinderDirection::Set);
    Bind(*ui, binder, value);
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

void PreferencesDialog::editDefaultFileProperties()
{
    FilePropertiesDialog dlg(this);
    dlg.SetUiFromMetadata(defaultFileProperties);
    if(dlg.exec() == QDialog::Rejected || !dlg.GetIsDirty())
        return;

    dlg.UpdateMetadataFromUi(defaultFileProperties);
    if(dlg.GetIsDirty())
        defaultFilePropertiesChanged = true;
}

void PreferencesDialog::editKeyFileProperties()
{
    FilePropertiesDialog dlg(this);
    dlg.SetUiFromMetadata(keyFileProperties);
    if(dlg.exec() == QDialog::Rejected || !dlg.GetIsDirty())
        return;

    dlg.UpdateMetadataFromUi(keyFileProperties);
}
