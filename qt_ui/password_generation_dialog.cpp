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
#include "password_generation_dialog.h"
#include "ui_password_generation_dialog.h"
#include <QStringList>
#include <QFontDatabase>
#include <QListWidgetItem>
#include <vector>
#include <algorithm>
#include "password_generator.h"
#include "assert.h"
#include "common_definitions.h"

using namespace EncryptPad;

PasswordGenerationDialog::PasswordGenerationDialog(QWidget *parent) :
    QDialog(parent, kDefaultWindowFlags),
    ui(new Ui::PasswordGenerationDialog),
    ignoreRegenerate(true),
    allPasswords(false)
{
    ui->setupUi(this);
    ui->uiPasswordCount->addItem(tr("7 passwords"), QVariant(7));
    ui->uiPasswordCount->addItem(tr("15 passwords"), QVariant(15));
    ui->uiPasswordCount->addItem(tr("25 passwords"), QVariant(25));
    ui->uiPasswordCount->setCurrentIndex(0);
    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    ui->uiPasswords->setFont(fixedFont);
    ignoreRegenerate = false;
    ui->actionRegenerate->trigger();
}

QStringList PasswordGenerationDialog::getPasswords() const
{
    QStringList list;

    for(int i = 0; i < ui->uiPasswords->count(); i++)
    {
        list.push_back(ui->uiPasswords->item(i)->text());
    }
    return list;
}

QString PasswordGenerationDialog::getCurrentPassword() const
{
    return ui->uiPasswords->currentItem()->text();
}

bool PasswordGenerationDialog::getAllPasswords() const
{
    return allPasswords;
}

int PasswordGenerationDialog::getLength()
{
    int length = 0;
    const Control2Length *table = getControl2LengthTable();
    while(table->control)
    {
        if(table->control->isChecked())
        {
            length = table->length;
            break;
        }

        table++;
    }

    if(ui->uiCustomLength->isChecked())
    {
        length = ui->uiCustomLengthValue->value();
    }

    return length;
}

void PasswordGenerationDialog::setLength(int length)
{
    const Control2Length *table = getControl2LengthTable();
    while(table->control)
    {
        if(table->length == length)
        {
            table->control->setChecked(true);
            break;
        }
        table ++;
    }

    // not found
    if(!table->control)
    {
        ui->uiCustomLength->setChecked(true);
        ui->uiCustomLengthValue->setValue(length);
    }
}

CharRange PasswordGenerationDialog::getCharRange()
{
    CharRange range = CharRange::None;
    const Control2CharRange *table = getControl2CharRange();
    while(table->control)
    {
        if(table->control->isChecked())
            range = range | table->range;

        table++;
    }

    return range;
}

void PasswordGenerationDialog::setCharRange(CharRange range)
{
    const Control2CharRange *table = getControl2CharRange();
    while(table->control)
    {
        table->control->setChecked((range & table->range) != CharRange::None);
        table++;
    }
}

const PasswordGenerationDialog::Control2Length *PasswordGenerationDialog::getControl2LengthTable()
{
    typedef Control2Length C;
    if(control2LengthTable.empty())
    {
        auto &v = control2LengthTable;
        v.push_back(C(ui->uiLength8, 8));
        v.push_back(C(ui->uiLength12, 12));
        v.push_back(C(ui->uiLength16, 16));
        v.push_back(C(nullptr, 0));
    }

    return control2LengthTable.data();
}

const PasswordGenerationDialog::SpinBox2CharRange *PasswordGenerationDialog::getMaxControl2CharRange()
{
    typedef SpinBox2CharRange C;
    if(maxControl2CharRange.empty())
    {
        auto &v = maxControl2CharRange;
        v.push_back(C(ui->uiLowerAtoZMax, CharRange::LowerAtoZ));
        v.push_back(C(ui->uiUpperAtoZMax, CharRange::UpperAtoZ));
        v.push_back(C(ui->uiNumbersMax, CharRange::Numbers));
        v.push_back(C(ui->uiSymbolsMax, CharRange::Symbols));
        v.push_back(C(ui->uiCustomSymbolsMax, CharRange::Other));
        v.push_back(C(nullptr, CharRange::None));
    }

    return maxControl2CharRange.data();
}

const PasswordGenerationDialog::Control2CharRange *PasswordGenerationDialog::getControl2CharRange()
{
    typedef Control2CharRange C;
    if(control2CharRange.empty())
    {
        auto &v = control2CharRange;
        v.push_back(C(ui->uiLowerAtoZ, CharRange::LowerAtoZ));
        v.push_back(C(ui->uiUpperAtoZ, CharRange::UpperAtoZ));
        v.push_back(C(ui->uiNumbers, CharRange::Numbers));
        v.push_back(C(ui->uiSymbols, CharRange::Symbols));
        v.push_back(C(ui->uiCustomSymbols, CharRange::Other));
        v.push_back(C(nullptr, CharRange::None));
    }

    return control2CharRange.data();
}

void PasswordGenerationDialog::setSettings(const QStringList &list)
{
    if(list.empty())
        return;

    assert(list.count() == 9);

    QStringList::const_iterator it = list.begin();
    setLength((it++)->toInt());
    setCharRange(static_cast<CharRange>((it++)->toInt()));
    ui->uiCustomSymbolsValue->setText(*(it++));
    ui->uiLowerAtoZMax->setValue((it++)->toInt());
    ui->uiUpperAtoZMax->setValue((it++)->toInt());
    ui->uiNumbersMax->setValue((it++)->toInt());
    ui->uiSymbolsMax->setValue((it++)->toInt());
    ui->uiCustomSymbolsMax->setValue((it++)->toInt());
    ui->uiPasswordCount->setCurrentIndex((it++)->toInt());
}

QStringList PasswordGenerationDialog::getSettings()
{
    QStringList list;
    list.push_back(QString::number(getLength()));
    list.push_back(QString::number(static_cast<int>(getCharRange())));
    list.push_back(ui->uiCustomSymbolsValue->text());
    list.push_back(QString::number(ui->uiLowerAtoZMax->value()));
    list.push_back(QString::number(ui->uiUpperAtoZMax->value()));
    list.push_back(QString::number(ui->uiNumbersMax->value()));
    list.push_back(QString::number(ui->uiSymbolsMax->value()));
    list.push_back(QString::number(ui->uiCustomSymbolsMax->value()));
    list.push_back(QString::number(ui->uiPasswordCount->currentIndex()));
    return list;
}

PasswordGenerationDialog::~PasswordGenerationDialog()
{
    delete ui;
}

void PasswordGenerationDialog::on_actionRegenerate_triggered()
{
    if(ignoreRegenerate)
        return;

    ignoreRegenerate = true;
    ui->uiPasswords->clear();

    int length = getLength();
    CharRange range = getCharRange();

    std::vector<CharSet> sets = GetCharSets(range);

    if((range & CharRange::Other) != CharRange::None && !ui->uiCustomSymbolsValue->text().isEmpty())
    {
        sets.push_back(CharSet(ui->uiCustomSymbolsValue->text().toStdString(), CharRange::Other));
    }

    const SpinBox2CharRange *table = getMaxControl2CharRange();
    while(table->control)
    {
        std::vector<CharSet>::iterator set_it = std::find_if(sets.begin(), sets.end(),
                                   [table](CharSet &set){ return set.range == table->range; });
        if(set_it != sets.end())
        {
            set_it->max = table->control->value();
        }

        table++;
    }

    std::vector<std::string> passwords = GeneratePasswords(sets, length, ui->uiPasswordCount->currentData().toInt());
    auto pwd_it = passwords.begin();
    for(; pwd_it != passwords.end(); pwd_it++)
    {
        QListWidgetItem *item = new QListWidgetItem(QString::fromStdString(*pwd_it), ui->uiPasswords);
        item->setTextAlignment(Qt::AlignCenter);
        ui->uiPasswords->addItem(item);
    }

    if(ui->uiPasswords->count() > 0)
    {
        ui->uiPasswords->setCurrentRow(0);
    }

    ignoreRegenerate = false;
}

void PasswordGenerationDialog::on_uiInsertAll_clicked()
{
    allPasswords = true;
    this->accept();
}
