//**********************************************************************************
//EncryptPad Copyright 2015 Evgeny Pokhilko 
//<evpomail@gmail.com> <http://www.evpo.net/encryptpad>
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
#include <string>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <utility>
#include <tuple>
#include "gtest/gtest.h"
#include "encryptor.h"


using namespace std;
using namespace EncryptPadEncryptor;
using namespace Botan;
using namespace EncryptPad;

class EncryptorFixtureBase
{
protected:
	Encryptor encryptor_; // encryptor should go first because it has Botan library initializer
	SecureVector<byte> initial_string_;
	SecureVector<byte> buffer_;
    PacketMetadata metadata_;
	static const string text_to_encrypt_;
	static const string temp_file_;
	static const string x2_key_file_;

	void SetUpBase()
	{
        initial_string_ = SecureVector<byte>(text_to_encrypt_.begin(),
                text_to_encrypt_.end());
        metadata_= PacketMetadata();
        metadata_.cipher_algo = kDefaultCipherAlgo;
        metadata_.hash_algo = kDefaultHashAlgo;
        metadata_.compression = kDefaultCompression;
        metadata_.iterations = kDefaultIterations;
        metadata_.persist_key_path = false;
        metadata_.is_binary = true;
        metadata_.file_name = "_CONSOLE";
	}

	void TearDownBase()
	{
		encryptor_.SetIsPlainText();
		buffer_.clear();
		remove(temp_file_.c_str());
	}
};


const string EncryptorFixtureBase::text_to_encrypt_ = "Hello World";
const string EncryptorFixtureBase::temp_file_ = "file.epad";
const string EncryptorFixtureBase::x2_key_file_ = "x2.key";

class EncryptorFixture : public EncryptorFixtureBase, public ::testing::Test
{
protected:
	virtual void SetUp()
	{
		SetUpBase();
	}

	virtual void TearDown()
	{
		TearDownBase();
	}

};

namespace {
	string FromSecureVectorToString(const SecureVector<byte> &sec_vec)
	{
		const char *sec_vec_buf = reinterpret_cast<const char *>(sec_vec.data());
		return string(sec_vec_buf, sec_vec_buf + sec_vec.size());
	}

	bool CopyFile(string file_from, string file_to)
	{
		ifstream src_stm(file_from, ifstream::binary);
		if(src_stm.fail())
			return false;
		ofstream dst_stm(file_to, ofstream::binary);
		if(dst_stm.fail())
			return false;
        if(dst_stm << src_stm.rdbuf())
            return true;
        else
            return false;
    }

	void CreateFile(string fileName, string content)
	{
		remove(fileName.c_str());
		ofstream stm(fileName);
		stm << content;
	}
}

TEST_F(EncryptorFixture, When_plain_text_without_X2_File_is_saved_in_plain_text)
{
	// Arrange

	// Act
	EpadResult save_result = encryptor_.Save(temp_file_, initial_string_,
           "", false, &metadata_);

	// Assert
	ifstream stm(temp_file_);
	string str;
	getline(stm, str, '\0');

	ASSERT_EQ(EpadResult::Success, save_result);
	ASSERT_EQ(text_to_encrypt_, str);
}

TEST_F(EncryptorFixture, When_file_is_plain_text_File_is_loaded_without_passphrase_and_X2)
{
	// Arrange
	ofstream stm(temp_file_);
	stm << text_to_encrypt_;
	stm.close();

	// Act
	EpadResult load_result = encryptor_.Load(temp_file_, buffer_);

	// Assert
	ASSERT_EQ(EpadResult::Success, load_result);
	ASSERT_EQ(text_to_encrypt_, FromSecureVectorToString(buffer_));
}


// Exceptions

TEST_F(EncryptorFixture, When_passphrase_is_wrong_and_without_X2_Encryption_error)
{
	// Arrange
	encryptor_.SetPassphrase("123", &metadata_);
	// encryptor_.Save(temp_file_, initial_string_);
	// encryptor_.SetPassphrase("321", &metadata_);
    //
	// // Act
	// EpadResult load_result = encryptor_.Load(temp_file_, buffer_);
    //
	// // Assert
	// ASSERT_EQ(EpadResult::EncryptionError, load_result);
}

TEST_F(EncryptorFixture, When_passphrase_is_correct_and_X2_file_does_not_exist_X2_key_IO_error)
{
	// Arrange
	encryptor_.SetPassphrase("123", &metadata_);
	bool persist_x2_location = false;
	encryptor_.Save(temp_file_, initial_string_, x2_key_file_, persist_x2_location,
            &metadata_);

	// Act
	EpadResult load_result = encryptor_.Load(temp_file_, buffer_, "i_dont_exist.key",
            nullptr, &metadata_);

	// Assert
	ASSERT_EQ(EpadResult::IOErrorKeyFile, load_result);
}

TEST_F(EncryptorFixture, When_passphrase_is_correct_and_X2_file_persisted_and_does_not_exist_X2_IO_error)
{
	// Arrange
	encryptor_.SetPassphrase("123", &metadata_);
	bool persist_x2_location = true;
	remove("x2_copy.key");
	CopyFile(x2_key_file_, "x2_copy.key");
	encryptor_.Save(temp_file_, initial_string_, "x2_copy.key", persist_x2_location, &metadata_);
	remove("x2_copy.key");

	// Act
	EpadResult load_result = encryptor_.Load(temp_file_, buffer_,
            "", nullptr, &metadata_);

	// Assert
	ASSERT_EQ(EpadResult::IOErrorKeyFile, load_result);
}

TEST_F(EncryptorFixture, When_passphrase_is_correct_and_X2_file_is_wrong_Encryption_error)
{
	// Arrange
	encryptor_.SetPassphrase("123", &metadata_);
	CreateFile("x2-invalid.key", "FKeYEq3z3S8krxeWX+gFRnmkTzRTjwyjRxgOFw+eP3s=");


	bool persist_x2_location = true;
	encryptor_.Save(temp_file_, initial_string_, x2_key_file_, persist_x2_location, &metadata_);
	

	// Act
	EpadResult load_result = encryptor_.Load(temp_file_, buffer_, "x2-invalid.key", nullptr,
            &metadata_);
	remove("x2-invalid.key");

	// Assert
    ASSERT_EQ(EpadResult::InvalidKeyFile, load_result);
}

TEST_F(EncryptorFixture, When_invalid_X2_file_Save_result_is_invalid_X2_file)
{
	// Arrange
	encryptor_.SetPassphrase("123", &metadata_);
	// CreateFile("x2-invalid.key", "ICANTBEAKEY     DFSFSDFSDF\t\r\n^*&%*&^*&^*(&^");
	CreateFile("x2-invalid.key", "");
	bool persist_x2_location = false;
	
	// Act
	EpadResult save_result = encryptor_.Save(temp_file_, initial_string_, "x2-invalid.key", persist_x2_location, &metadata_);
	remove("x2-invalid.key");

	// Assert
	ASSERT_EQ(EpadResult::InvalidKeyFile, save_result);
}

TEST_F(EncryptorFixture, When_passphrase_is_correct_and_X2_file_is_invalid_Invalid_X2_file)
{
	// Arrange
	
	encryptor_.SetPassphrase("123", &metadata_);
	CreateFile("x2-invalid.key", "ICANTBEAKEY     DFSFSDFSDF\t\r\n^*&%*&^*&^*(&^");
	bool persist_x2_location = false;
	encryptor_.Save(temp_file_, initial_string_, x2_key_file_, persist_x2_location, &metadata_);

	// Act
	EpadResult load_result = encryptor_.Load(temp_file_, buffer_, "x2-invalid.key", nullptr,
            &metadata_);
	remove("x2-invalid.key");

	// Assert
	ASSERT_EQ(EpadResult::InvalidKeyFile, load_result);
}

TEST_F(EncryptorFixture, When_plain_text_and_invalid_X2_file_Load_result_is_invalid_X2_file)
{
	// Arrange
	encryptor_.SetIsPlainText();
	CreateFile("x2-invalid.key", "ICANTBEAKEY     DFSFSDFSDF\t\r\n^*&%*&^*&^*(&^");
	bool persist_x2_location = false;
	encryptor_.Save(temp_file_, initial_string_, x2_key_file_, persist_x2_location,
            &metadata_);

	// Act
	EpadResult load_result = encryptor_.Load(temp_file_, buffer_, "x2-invalid.key",
            nullptr, &metadata_);
	remove("x2-invalid.key");

	// Assert
	ASSERT_EQ(EpadResult::InvalidKeyFile, load_result);
}

TEST_F(EncryptorFixture, When_passphrase_and_X2_and_X2_not_persisted_and_loaded_without_X2_Result_is_X2_required)
{
	// Arrange
	encryptor_.SetPassphrase("123", &metadata_);
	bool persiste_x2_location = false;
	encryptor_.Save(temp_file_, initial_string_, x2_key_file_, persiste_x2_location,
            &metadata_);

	// Act
	EpadResult load_result = encryptor_.Load(temp_file_, buffer_, "", nullptr,
            &metadata_);

	// Assert
	ASSERT_EQ(EpadResult::KeyFileNotSpecified, load_result);
}

// Disable the test because this test is done in loadFile before coming to Load. However, we leave the test
// because this logic may need to move to Encryptor::Load
// TEST_F(EncryptorFixture, When_plain_text_and_X2_and_X2_not_persisted_and_loaded_without_X2_Result_is_X2_required)
// {
// 	// Arrange
// 	encryptor_.SetIsPlainText();
// 	bool persiste_x2_location = false;
// 	encryptor_.Save(temp_file_, initial_string_, x2_key_file_, persiste_x2_location,
//             &metadata_);
//
// 	// Act
// 	EpadResult load_result = encryptor_.Load(temp_file_, buffer_, "", nullptr,
//             &metadata_);
//
// 	// Assert
// 	ASSERT_EQ(EpadResult::KeyFileNotSpecified, load_result);
// }

TEST_F(EncryptorFixture, When_passphrase_used_and_saved_without_key_The_result_of_load_with_key_and_passphrase_is_OK)
{
	// Arrange
	encryptor_.SetPassphrase("123", &metadata_);
	encryptor_.Save(temp_file_, initial_string_, "", false, &metadata_);

	// Act
	EpadResult load_result = encryptor_.Load(temp_file_, buffer_, x2_key_file_,
            nullptr, &metadata_);

	// Assert
	ASSERT_EQ(EpadResult::Success, load_result);
}

TEST_F(EncryptorFixture, When_passphrase_used_with_persisted_key_Key_information_is_available_after_load)
{
	// Arrange
	encryptor_.SetPassphrase("123", &metadata_);
	bool persistX2KeyLocation = true;
	encryptor_.Save(temp_file_, initial_string_, x2_key_file_, persistX2KeyLocation,
            &metadata_);

	// Act
	EpadResult load_result = encryptor_.Load(temp_file_, buffer_,
            "", nullptr, &metadata_);

	// Assert
    ASSERT_EQ(EpadResult::Success, load_result);
	ASSERT_EQ(x2_key_file_, encryptor_.GetX2KeyLocation());
}

// Test different combinations saving with key

struct TestParam
{
	string passphrase;
	bool usePassphrase;
	bool useX2;
	bool persistX2;
	bool useX2ParameterInLoad;
};

class EncryptorFixtureWithParam : public EncryptorFixtureBase, public ::testing::TestWithParam<TestParam>
{
public:
	static TestParam ParameterCombination[];

protected:
	virtual void SetUp()
	{
		SetUpBase();
	}

	virtual void TearDown()
	{
		TearDownBase();
	}
};

TestParam EncryptorFixtureWithParam::ParameterCombination[] = 
{
	//pwd	usePassphrase	useX2	persistX2	useX2ParameterInLoad
	{"123",	true,		false,	false,		false},
	{"123",	true,		true,	false,		true},
	{"123",	true,		true,	true,		false},
	{"123",	true,		true,	true,		true},
	{"",	false,		false,	false,		false},
	{"",	false,		true,	false,		true},
    // File type identification happens before calling Load
    // see the comment in 
    // When_plain_text_and_X2_and_X2_not_persisted_and_loaded_without_X2_Result_is_X2_required
	// {"",	false,		true,	true,		false},
	{"",	false,		true,	true,		true}
};

TEST_P(EncryptorFixtureWithParam, When_combination_of_pwd_X2_persist_File_is_saved_and_loaded)
{
	// Arrange
	const TestParam &testParam = GetParam();
	if(!testParam.passphrase.empty())
		encryptor_.SetPassphrase(testParam.passphrase.c_str(), &metadata_);
	else
		encryptor_.SetIsPlainText();

	// Act
	EpadResult save_result = encryptor_.Save(
		temp_file_, 
		initial_string_, 
		testParam.useX2 ? x2_key_file_ : "",
		testParam.persistX2,
        &metadata_);

	EpadResult load_result = encryptor_.Load(
		temp_file_, 
		buffer_, 
		testParam.useX2ParameterInLoad ? x2_key_file_ : "",
        nullptr,
        &metadata_);

	string str = FromSecureVectorToString(buffer_);

	// Assert
	ASSERT_EQ(EpadResult::Success, save_result);
	ASSERT_EQ(EpadResult::Success, load_result);
	ASSERT_EQ(text_to_encrypt_, str);
}

INSTANTIATE_TEST_CASE_P(Common, EncryptorFixtureWithParam,
						::testing::ValuesIn(EncryptorFixtureWithParam::ParameterCombination));
