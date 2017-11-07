#include "emsg_symmetric_key.h"
#include "botan/auto_rng.h"
#include "botan/pbkdf.h"
#include "emsg_constants.h"
#include "openpgp_conversions.h"

namespace LibEncryptMsg
{
    Salt GenerateRandomSalt()
    {
        Botan::AutoSeeded_RNG rng;
        std::vector<uint8_t> salt(kSaltSize);
        rng.randomize(salt.data(), salt.size());
        return salt;
    }

    // Generates an encryption key and update salt in security parameters if it was empty
    std::unique_ptr<EncryptionKey> GenerateEncryptionKey(const Passphrase &passphrase, CipherAlgo cipher_algo, HashAlgo hash_algo, uint8_t iterations, Salt salt)
    {
        using namespace Botan;
        std::unique_ptr<EncryptionKey> key(new EncryptionKey());

        auto algo_spec = GetAlgoSpec(cipher_algo);
        auto hash_spec = GetHashSpec(hash_algo);
        std::unique_ptr<PBKDF> pbkdf(get_pbkdf(hash_spec.botan_name));
        const char *ptr = reinterpret_cast<const char*>(passphrase.data.data());
        //TODO: use unique_ptr to wipe out the string before exiting the function
        std::string str(ptr, ptr + passphrase.data.size());
        *key = pbkdf->derive_key(
                algo_spec.key_size / 8,
                str,
                salt.data(),
                salt.size(),
                DecodeS2KIterations(iterations));
        std::fill(str.begin(), str.end(), 0xFF);
        return key;
    }
}

