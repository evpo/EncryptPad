#include "commands.h"
#include <iostream>
#include <fstream>
#include "emsg_types.h"
#include "message_encryption.h"
#include "message_decryption.h"
#include "emsg_symmetric_key.h"

using namespace std;
using namespace LibEncryptMsg;
using namespace Botan;

namespace
{
    fstream::char_type *ToChar(uint8_t *p)
    {
        return reinterpret_cast<fstream::char_type*>(p);
    }

    SafeVector ReadFile(const std::string &file_name)
    {
        SafeVector buf;
        ifstream pwd_stm(file_name, ios_base::binary);
        if(!pwd_stm)
            return buf;
        pwd_stm.seekg(0, ios_base::end);
        auto length = pwd_stm.tellg();
        buf.resize(length);
        pwd_stm.seekg(0, ios_base::beg);
        pwd_stm.read(ToChar(buf.data()), buf.size());
        return buf;
    }

    class FilePassphraseProvider : public PassphraseProvider
    {
        private:
            std::string pwd_file_;
        public:
            FilePassphraseProvider(const std::string &pwd_file)
                :pwd_file_(pwd_file)
            {
            }

            std::unique_ptr<SafeVector> GetPassphrase(std::string description, bool &cancelled) override
            {
                (void)description;
                cancelled = false;
                return std::unique_ptr<SafeVector>(new SafeVector(ReadFile(pwd_file_)));
            }
    };
}

namespace LibEncryptMsg
{
    namespace Cli
    {
        void Encrypt(const std::string &input, const std::string &output, const std::string &pwd_file)
        {
            FilePassphraseProvider pwd_provider(pwd_file);

            MessageConfig config;
            config.SetCipherAlgo(CipherAlgo::AES256);
            config.SetCompression(Compression::ZIP);
            config.SetFileName("test.txt");
            config.SetFileDate(0); //TODO: see how to set the date in EncryptPad
            config.SetBinary(true);
            config.SetPartialLengthPower(4); // This will create partial packets of 2^4 = 16 bytes

            MessageWriter writer;


            const bool existing_key = true;
            if(existing_key)
            {
                Salt salt = GenerateRandomSalt();
                std::unique_ptr<EncryptionKey> encryption_key = GenerateEncryptionKey(
                        ReadFile(pwd_file), config.GetCipherAlgo(),
                        config.GetHashAlgo(), config.GetIterations(), salt);
                writer.Start(std::move(encryption_key), config, salt);
            }
            else
            {
                writer.Start(ReadFile(pwd_file)); // use default parameters
            }

            ifstream in_file(input, ios::binary);
            if(!in_file)
                return;

            ofstream out_file(output, ios::binary);
            if(!out_file)
                return;

            const size_t kBufSize = 16;
            SafeVector buf(kBufSize);
            while(in_file && !in_file.eof())
            {
                buf.resize(kBufSize);
                in_file.read(ToChar(buf.data()), buf.size());
                buf.resize(in_file.gcount());
                writer.Update(buf);
                out_file.write(ToChar(buf.data()), buf.size());
            }
            buf.clear();
            writer.Finish(buf);
            out_file.write(ToChar(buf.data()), buf.size());
        }

        void Decrypt(const std::string &input, const std::string &output, const std::string &pwd_file)
        {
            cout << "input: " << input << " output: " << output << " pwd: " << pwd_file << endl;

            FilePassphraseProvider pwd_provider(pwd_file);
            MessageReader reader;
            reader.Start(pwd_provider);

            ifstream in_file(input, ios::binary);
            if(!in_file)
                return;

            ofstream out_file(output, ios::binary);
            if(!out_file)
                return;

            const size_t kBufSize = 16;
            SafeVector buf(kBufSize);
            while(in_file && !in_file.eof())
            {
                buf.resize(kBufSize);
                in_file.read(ToChar(buf.data()), buf.size());
                buf.resize(in_file.gcount());
                reader.Update(buf);
                out_file.write(ToChar(buf.data()), buf.size());
            }
            buf.clear();
            reader.Finish(buf);
            out_file.write(ToChar(buf.data()), buf.size());
        }
    }
}
