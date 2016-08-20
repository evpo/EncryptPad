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
#ifndef PASSWORD_GENERATION_DIALOG_H
#define PASSWORD_GENERATION_DIALOG_H

#include <QDialog>
#include <QAbstractButton>
#include <QSpinBox>
#include <vector>
#include "password_generator.h"

namespace Ui {
class PassphraseGenerationDialog;
}

class PassphraseGenerationDialog : public QDialog
{
    Q_OBJECT
private:
    Ui::PassphraseGenerationDialog *ui;
    bool ignoreRegenerate;
    bool allPassphrases;
    int getLength();
    void setLength(int length);
    EncryptPad::CharRange getCharRange();
    void setCharRange(EncryptPad::CharRange range);

    struct Control2Length
    {
        Control2Length(QAbstractButton *control_p, int length_p)
            :control(control_p), length(length_p){}
        QAbstractButton *control;
        int length;
    };

    struct Control2CharRange
    {
        Control2CharRange(QAbstractButton *control_p, EncryptPad::CharRange range_p)
            :control(control_p), range(range_p){}
        QAbstractButton *control;
        EncryptPad::CharRange range;
    };

    struct SpinBox2CharRange
    {
        SpinBox2CharRange(QSpinBox *control_p, EncryptPad::CharRange range_p)
            :control(control_p), range(range_p){}
        QSpinBox *control;
        EncryptPad::CharRange range;
    };

    std::vector<Control2Length> control2LengthTable;
    std::vector<Control2CharRange> control2CharRange;
    std::vector<SpinBox2CharRange> maxControl2CharRange;

    const Control2Length *getControl2LengthTable();
    const Control2CharRange *getControl2CharRange();
    const SpinBox2CharRange *getMaxControl2CharRange();

public:
    explicit PassphraseGenerationDialog(QWidget *parent = 0);
    ~PassphraseGenerationDialog();

    QStringList getSettings();
    void setSettings(const QStringList &list);
    QStringList getPassphrases() const;
    QString getCurrentPassphrase() const;
    bool getAllPassphrases() const;

private slots:
    void on_actionRegenerate_triggered();
    void on_uiInsertAll_clicked();
};

#endif // PASSWORD_GENERATION_DIALOG_H
