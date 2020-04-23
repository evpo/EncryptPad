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
#include "file_properties_dialog.h"
#include "ui_file_properties_dialog.h"
#include <QDebug>
#include <QtGlobal>
#include <sstream>
#include "common_definitions.h"
#include "encryptmsg/openpgp_conversions.h"

namespace
{
    template<class T>
    int ToInt(T v)
    {
        return static_cast<int>(v);
    }

    struct Name2Id
    {
        const char *name;
        int id;
    };

    void PopulateCombo(QComboBox *c, Name2Id *p)
    {
        c->clear();
        while(p->name)
        {
            c->addItem(qApp->translate("FilePropertiesDialog", p->name), ToInt(p->id));
            p++;
        }
    }

    template<class T>
    void SetCurrent(QComboBox *c, T v)
    {
        c->setCurrentIndex(c->findData(ToInt(v)));
    }
}

FilePropertiesDialog::FilePropertiesDialog(QWidget *parent) :
    QDialog(parent, kDefaultWindowFlags),
    ui(new Ui::FilePropertiesDialog), isDirty(false)
{
    ui->setupUi(this);
    PopulateItems();
    connect(ui->uiCipherAlgo, SIGNAL(currentIndexChanged(int)), this, SLOT(PropertyChanged()));
    connect(ui->uiHashAlgo, SIGNAL(currentIndexChanged(int)), this, SLOT(PropertyChanged()));
    connect(ui->uiCompressionAlgo, SIGNAL(currentIndexChanged(int)), this, SLOT(PropertyChanged()));
    connect(ui->uiIterations, SIGNAL(currentIndexChanged(int)), this, SLOT(PropertyChanged()));
}

FilePropertiesDialog::~FilePropertiesDialog()
{
    delete ui;
}

void FilePropertiesDialog::PopulateItems()
{
    using namespace EncryptMsg;
    static Name2Id ciphers[] =
    {
        {"Triple DES", ToInt(CipherAlgo::TripleDES)},
        {"CAST5", ToInt(CipherAlgo::CAST5)},
        {"AES-128", ToInt(CipherAlgo::AES128)},
        {"AES-192", ToInt(CipherAlgo::AES192)},
        {QT_TRANSLATE_NOOP("FilePropertiesDialog", "AES-256 (recommended)"), ToInt(CipherAlgo::AES256)},
        {"Twofish", ToInt(CipherAlgo::Twofish)},
        {"Camellia-128", ToInt(CipherAlgo::Camellia128)},
        {"Camellia-192", ToInt(CipherAlgo::Camellia192)},
        {"Camellia-256", ToInt(CipherAlgo::Camellia256)},
        {nullptr, 0}
    };

    static Name2Id hashes[] =
    {
        {"SHA-1", ToInt(HashAlgo::SHA160)},
        {"SHA-256", ToInt(HashAlgo::SHA256)},
        {"SHA-384", ToInt(HashAlgo::SHA384)},
        {"SHA-512", ToInt(HashAlgo::SHA512)},
        {"SHA-224", ToInt(HashAlgo::SHA224)},
        {nullptr, 0}
    };

    static Name2Id compressions[] =
    {
        {QT_TRANSLATE_NOOP("FilePropertiesDialog", "Uncompressed"), ToInt(Compression::Uncompressed)},
        {"ZIP", ToInt(Compression::ZIP)},
        {"ZLIB", ToInt(Compression::ZLIB)},
        {"BZip2", ToInt(Compression::BZip2)},
        {nullptr, 0}
    };

    PopulateCombo(ui->uiCipherAlgo, ciphers);
    PopulateCombo(ui->uiHashAlgo, hashes);
    PopulateCombo(ui->uiCompressionAlgo, compressions);

    for(unsigned int i = 0; i < 256; i++)
    {
        unsigned int iterations = DecodeS2KIterations(static_cast<unsigned char>(i));
        ui->uiIterations->addItem(
            QString::number(iterations), QVariant(iterations));
    }
}


void FilePropertiesDialog::SetUiFromMetadata(const EncryptPad::PacketMetadata &metadata)
{
    SetCurrent(ui->uiCipherAlgo, metadata.cipher_algo);
    SetCurrent(ui->uiHashAlgo, metadata.hash_algo);
    SetCurrent(ui->uiCompressionAlgo, metadata.compression);
    SetCurrent(ui->uiIterations, metadata.iterations);

    std::ostringstream stm;
    auto it = metadata.salt.begin();
    for(;it != metadata.salt.end(); it++)
    {
        if(it != metadata.salt.begin())
            stm << " ";
        stm << std::uppercase << std::hex << static_cast<int>(*it);
    }
    std::string s = stm.str();
    QString qs = QString::fromStdString(s);
    ui->uiSalt->setText(qs);
    isDirty = false;
}

bool FilePropertiesDialog::GetIsDirty() const
{
    return isDirty;
}

void FilePropertiesDialog::UpdateMetadataFromUi(EncryptPad::PacketMetadata &metadata) const
{
    using namespace EncryptMsg;
    metadata.cipher_algo = static_cast<CipherAlgo>(ui->uiCipherAlgo->currentData().toInt());
    metadata.hash_algo = static_cast<HashAlgo>(ui->uiHashAlgo->currentData().toInt());
    metadata.compression = static_cast<Compression>(ui->uiCompressionAlgo->currentData().toInt());
    metadata.iterations = DecodeS2KIterations(EncodeS2KIterations(ui->uiIterations->currentData().toUInt()));
}

void FilePropertiesDialog::PropertyChanged()
{
   isDirty = true;
}
