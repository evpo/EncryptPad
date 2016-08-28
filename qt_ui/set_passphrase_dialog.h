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
#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>

class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;

class SetPassphraseDialog : public QDialog
{
	Q_OBJECT
public:
	SetPassphraseDialog(QWidget *parent = 0, const bool enableConfirmation = true);
	const QString GetValue();
private slots:
	void okClicked();
private:
	QLabel *pwdLabel;
	QLabel *confirmPwdLabel;
	QLineEdit *pwdLineEdit;
	QLineEdit *confirmPwdLineEdit;
	QPushButton *okButton;
	QPushButton *cancelButton;
	bool mConfirmationEnabled;

	void HideConfirmation();
};

#endif
