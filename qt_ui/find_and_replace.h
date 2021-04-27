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
#ifndef FIND_AND_REPLACE_H
#define FIND_AND_REPLACE_H

#include <QDialog>

namespace Ui {
class FindAndReplace;
}

class FindAndReplace : public QDialog
{
    Q_OBJECT

public:
    explicit FindAndReplace(QWidget *parent = 0);
    ~FindAndReplace();

    QString GetFind() const;
    QString GetReplace() const;
    void SetFind(const QString &str);
    void setFindFocus();

private:
    Ui::FindAndReplace *ui;

signals:
    void findNext(QString text, bool down, bool matchCase, bool wholeWord);
    void replaceAll(QString text, QString replaceWith, bool matchCase, bool wholeWord);
    void replaceOne(QString text, QString replaceWith, bool matchCase, bool wholeWord);
private slots:
    void on_FindAndReplace_finished(int result);
    void on_uiFindNext_clicked();
    void on_uiReplaceOne_clicked();
    void on_uiReplaceAll_clicked();
    void on_uiFind_textChanged(const QString &text);
    void on_uiReplace_textChanged(const QString &text);
};

#endif // FIND_AND_REPLACE_H
