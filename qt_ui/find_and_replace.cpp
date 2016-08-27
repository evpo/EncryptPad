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
#include "find_and_replace.h"
#include "ui_find_and_replace.h"
#include "common_definitions.h"

FindAndReplace::FindAndReplace(QWidget *parent) :
    QDialog(parent, kDefaultWindowFlags),
    ui(new Ui::FindAndReplace)
{
    ui->setupUi(this);
    setMinimumSize(sizeHint());
    adjustSize();
}

FindAndReplace::~FindAndReplace()
{
    delete ui;
}

QString FindAndReplace::GetFind() const
{
    return ui->uiFind->text();
}

QString FindAndReplace::GetReplace() const
{
    return ui->uiReplace->text();
}

void FindAndReplace::SetFind(const QString &str)
{
    ui->uiFind->setText(str);
}

void FindAndReplace::on_FindAndReplace_finished(int result)
{
    (void)result;
    ui->uiFind->setFocus();
}

void FindAndReplace::on_uiFindNext_clicked()
{
    findNext(ui->uiFind->text(), true, ui->uiMatchCase->isChecked(), ui->uiWholeWord->isChecked());
}

void FindAndReplace::on_uiReplaceOne_clicked()
{
    replaceOne(ui->uiFind->text(), ui->uiReplace->text(), ui->uiMatchCase->isChecked(), ui->uiWholeWord->isChecked());
}

void FindAndReplace::on_uiReplaceAll_clicked()
{
    replaceAll(ui->uiFind->text(), ui->uiReplace->text(), ui->uiMatchCase->isChecked(), ui->uiWholeWord->isChecked());
}

void FindAndReplace::on_uiFind_textChanged(const QString &text)
{
    ui->uiFindNext->setEnabled(!text.isEmpty());

    bool replaceEnabled = !text.isEmpty() && !ui->uiReplace->text().isEmpty();
    ui->uiReplaceOne->setEnabled(replaceEnabled);
    ui->uiReplaceAll->setEnabled(replaceEnabled);
}

void FindAndReplace::on_uiReplace_textChanged(const QString &text)
{
    bool enabled = !ui->uiFind->text().isEmpty() && !text.isEmpty();
    ui->uiReplaceOne->setEnabled(enabled);
    ui->uiReplaceAll->setEnabled(enabled);
}
