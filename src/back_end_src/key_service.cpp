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
#include "key_service.h"
#include <algorithm>
#include "assert.h"
#include "encryptmsg/algo_spec.h"
#include "encryptmsg/openpgp_conversions.h"
#include "encryptmsg/emsg_symmetric_key.h"

using namespace EncryptMsg;

namespace EncryptPad
{
    KeyService::KeyService(int key_count)
        :key_count_(key_count)
        {}


    void KeyService::set_key_count(int key_count)
    {
        key_records_.clear();
        key_count_ = key_count;
    }

    const KeyRecord &KeyService::EmptyRecord() const
    {
        return empty_record_;
    }

    bool KeyService::UnusedKeysExist() const
    {
        auto it = std::find_if(key_records_.begin(), key_records_.end(), 
                [](const KeyRecordPtr &p){return !p->used;});

        return it != key_records_.end();
    }

    const KeyRecord &KeyService::GetKeyForSaving()
    {
        auto it = std::find_if(key_records_.begin(), key_records_.end(), 
                [](KeyRecordPtr &p){return !p->used;});

        if(it == key_records_.end())
        {
            key_records_.clear();
            return EmptyRecord();
        }

        (*it)->used = true;
        return **it;
    }

    const KeyRecord &KeyService::GetKeyForLoading(const std::vector<uint8_t> &salt, int iterations, EncryptMsg::HashAlgo hash_algo) const
    {
        if(hash_algo != hash_algo_)
            return EmptyRecord();

        auto it = std::find_if(key_records_.begin(), key_records_.end(), 
                [&salt, &iterations](const KeyRecordPtr &p){return p->salt == salt && p->iterations == iterations;});

        if(it == key_records_.end())
            return EmptyRecord();

        return **it;
    }

    const KeyRecord &KeyService::ChangePassphrase(const std::string &passphrase_str, EncryptMsg::HashAlgo hash_algo, unsigned key_size,
            int iterations, EncryptMsg::Salt salt)
    {
        using namespace Botan;

        const KeyRecord *ret_val = &EmptyRecord();

        key_size_ = key_size;
        hash_algo_ = hash_algo;

        key_records_.clear();

        Passphrase passphrase(SafeVector(passphrase_str.begin(), passphrase_str.end()));
        uint8_t encoded_iterations = EncodeS2KIterations(iterations);

        if(salt.size() > 0)
        {
            KeyRecordPtr key_record(new KeyRecord());
            key_record->used = true;
            key_record->iterations = iterations;
            key_record->salt = salt;
            key_record->key = EncryptMsg::GenerateEncryptionKey(
                    passphrase, key_size, hash_algo, encoded_iterations, key_record->salt);
            key_records_.push_back(key_record);
            ret_val = key_record.get();
        }

        //generate more keys for saving
        int size_before = key_records_.size();
        assert(size_before <= key_count_);
        for(int i = 0; i < key_count_ - size_before; i++)
        {
            Salt newSalt = GenerateRandomSalt();

            KeyRecordPtr key_record(new KeyRecord());
            key_record->used = false;
            key_record->iterations = iterations;
            key_record->salt = newSalt;
            key_record->key = EncryptMsg::GenerateEncryptionKey(
                    passphrase, key_size, hash_algo, encoded_iterations, key_record->salt);
            key_records_.push_back(key_record);
        }

        return *ret_val;
    }
}
