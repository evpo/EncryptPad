#include "key_service_key_provider.h"
#include "encryptmsg/openpgp_conversions.h"

using namespace EncryptMsg;
namespace EncryptPad
{
    KeyServiceKeyProvider::KeyServiceKeyProvider(KeyService *key_service, const std::string *passphrase):
        key_service_(key_service),
        passphrase_(passphrase)
    {
    }

    std::unique_ptr<EncryptMsg::EncryptionKey> KeyServiceKeyProvider::GetKey(
            EncryptMsg::CipherAlgo cipher_algo,
            EncryptMsg::HashAlgo hash_algo,
            uint8_t iterations,
            EncryptMsg::Salt salt,
            std::string description,
            bool &cancelled)
    {
        (void)description;
        (void)cancelled;

        const KeyRecord *key_record = nullptr;
        AlgoSpec algo_spec = GetAlgoSpec(cipher_algo);
        unsigned key_size = algo_spec.key_size;

        if(passphrase_)
        {
            key_record = &key_service_->ChangePassphrase(
                    *passphrase_,
                    hash_algo,
                    key_size,
                    DecodeS2KIterations(iterations),
                    salt
                    );
        }
        else
        {
            key_record = &key_service_->GetKeyForLoading(
                    salt, DecodeS2KIterations(iterations),
                    hash_algo);
        }
        std::unique_ptr<EncryptionKey> ret_val;
        if(!key_record->IsEmpty())
        {
            //TODO: Work on the key provider API to use pointer or reference
            ret_val.reset(new EncryptionKey(*key_record->key));
        }

        return ret_val;
    }
}

