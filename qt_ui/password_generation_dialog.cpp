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

PassphraseGenerationDialog::PassphraseGenerationDialog(QWidget *parent) :
    QDialog(parent, kDefaultWindowFlags),
    ui(new Ui::PassphraseGenerationDialog),
    ignoreRegenerate(true),
    allPassphrases(false)
{
    ui->setupUi(this);
    ui->uiPassphraseCount->addItem(tr("7 passphrases"), QVariant(7));
    ui->uiPassphraseCount->addItem(tr("15 passphrases"), QVariant(15));
    ui->uiPassphraseCount->addItem(tr("25 passphrases"), QVariant(25));
    ui->uiPassphraseCount->setCurrentIndex(0);
    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    ui->uiPassphrases->setFont(fixedFont);
    ignoreRegenerate = false;
    ui->actionRegenerate->trigger();
}

QStringList PassphraseGenerationDialog::getPassphrases() const
{
    QStringList list;

    for(int i = 0; i < ui->uiPassphrases->count(); i++)
    {
        list.push_back(ui->uiPassphrases->item(i)->text());
    }
    return list;
}

QString PassphraseGenerationDialog::getCurrentPassphrase() const
{
    return ui->uiPassphrases->currentItem()->text();
}

bool PassphraseGenerationDialog::getAllPassphrases() const
{
    return allPassphrases;
}

int PassphraseGenerationDialog::getLength()
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

void PassphraseGenerationDialog::setLength(int length)
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

CharRange PassphraseGenerationDialog::getCharRange()
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

void PassphraseGenerationDialog::setCharRange(CharRange range)
{
    const Control2CharRange *table = getControl2CharRange();
    while(table->control)
    {
        table->control->setChecked((range & table->range) != CharRange::None);
        table++;
    }
}

const PassphraseGenerationDialog::Control2Length *PassphraseGenerationDialog::getControl2LengthTable()
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

const PassphraseGenerationDialog::SpinBox2CharRange *PassphraseGenerationDialog::getMaxControl2CharRange()
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

const PassphraseGenerationDialog::Control2CharRange *PassphraseGenerationDialog::getControl2CharRange()
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

void PassphraseGenerationDialog::setSettings(const QStringList &list)
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
    ui->uiPassphraseCount->setCurrentIndex((it++)->toInt());
}

QStringList PassphraseGenerationDialog::getSettings()
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
    list.push_back(QString::number(ui->uiPassphraseCount->currentIndex()));
    return list;
}

PassphraseGenerationDialog::~PassphraseGenerationDialog()
{
    delete ui;
}

void PassphraseGenerationDialog::on_actionRegenerate_triggered()
{
    if(ignoreRegenerate)
        return;

    ignoreRegenerate = true;
    ui->uiPassphrases->clear();

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

    std::vector<std::string> passphrases = GeneratePassphrases(sets, length, ui->uiPassphraseCount->currentData().toInt());
    auto pwd_it = passphrases.begin();
    for(; pwd_it != passphrases.end(); pwd_it++)
    {
        QListWidgetItem *item = new QListWidgetItem(QString::fromStdString(*pwd_it), ui->uiPassphrases);
        item->setTextAlignment(Qt::AlignCenter);
        ui->uiPassphrases->addItem(item);
    }

    if(ui->uiPassphrases->count() > 0)
    {
        ui->uiPassphrases->setCurrentRow(0);
    }

    ignoreRegenerate = false;
}

void PassphraseGenerationDialog::on_uiInsertAll_clicked()
{
    allPassphrases = true;
    this->accept();
}
