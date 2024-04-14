/*
* KDF2
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_KDF2_H_
#define BOTAN_KDF2_H_

#include <botan/hash.h>
#include <botan/kdf.h>

namespace Botan {

/**
* KDF2, from IEEE 1363
*/
class KDF2 final : public KDF {
   public:
      std::string name() const override;

      std::unique_ptr<KDF> new_object() const override;

      void kdf(uint8_t key[],
               size_t key_len,
               const uint8_t secret[],
               size_t secret_len,
               const uint8_t salt[],
               size_t salt_len,
               const uint8_t label[],
               size_t label_len) const override;

      /**
      * @param hash the hash function to use
      */
      explicit KDF2(std::unique_ptr<HashFunction> hash) : m_hash(std::move(hash)) {}

   private:
      std::unique_ptr<HashFunction> m_hash;
};

}  // namespace Botan

#endif
