
/*
* (C) 2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_PK_OPERATION_IMPL_H_
#define BOTAN_PK_OPERATION_IMPL_H_

#include <botan/hash.h>
#include <botan/kdf.h>
#include <botan/pk_ops.h>
#include <botan/internal/eme.h>

namespace Botan::PK_Ops {

class Encryption_with_EME : public Encryption {
   public:
      size_t max_input_bits() const override;

      secure_vector<uint8_t> encrypt(const uint8_t msg[], size_t msg_len, RandomNumberGenerator& rng) override;

      ~Encryption_with_EME() override = default;

   protected:
      explicit Encryption_with_EME(std::string_view eme);

   private:
      virtual size_t max_ptext_input_bits() const = 0;

      virtual secure_vector<uint8_t> raw_encrypt(const uint8_t msg[], size_t len, RandomNumberGenerator& rng) = 0;
      std::unique_ptr<EME> m_eme;
};

class Decryption_with_EME : public Decryption {
   public:
      secure_vector<uint8_t> decrypt(uint8_t& valid_mask, const uint8_t msg[], size_t msg_len) override;

      ~Decryption_with_EME() override = default;

   protected:
      explicit Decryption_with_EME(std::string_view eme);

   private:
      virtual secure_vector<uint8_t> raw_decrypt(const uint8_t msg[], size_t len) = 0;
      std::unique_ptr<EME> m_eme;
};

class Verification_with_Hash : public Verification {
   public:
      ~Verification_with_Hash() override = default;

      void update(const uint8_t msg[], size_t msg_len) override;
      bool is_valid_signature(const uint8_t sig[], size_t sig_len) override;

      std::string hash_function() const final { return m_hash->name(); }

   protected:
      explicit Verification_with_Hash(std::string_view hash);

      explicit Verification_with_Hash(const AlgorithmIdentifier& alg_id,
                                      std::string_view pk_algo,
                                      bool allow_null_parameters = false);

      /*
      * Perform a signature check operation
      * @param msg the message
      * @param msg_len the length of msg in bytes
      * @param sig the signature
      * @param sig_len the length of sig in bytes
      * @returns if signature is a valid one for message
      */
      virtual bool verify(const uint8_t msg[], size_t msg_len, const uint8_t sig[], size_t sig_len) = 0;

   private:
      std::unique_ptr<HashFunction> m_hash;
};

class Signature_with_Hash : public Signature {
   public:
      void update(const uint8_t msg[], size_t msg_len) override;

      secure_vector<uint8_t> sign(RandomNumberGenerator& rng) override;

      ~Signature_with_Hash() override = default;

   protected:
      explicit Signature_with_Hash(std::string_view hash);

      std::string hash_function() const final { return m_hash->name(); }

#if defined(BOTAN_HAS_RFC6979_GENERATOR)
      std::string rfc6979_hash_function() const;
#endif

   private:
      virtual secure_vector<uint8_t> raw_sign(const uint8_t msg[], size_t msg_len, RandomNumberGenerator& rng) = 0;

      std::unique_ptr<HashFunction> m_hash;
};

class Key_Agreement_with_KDF : public Key_Agreement {
   public:
      secure_vector<uint8_t> agree(size_t key_len,
                                   const uint8_t other_key[],
                                   size_t other_key_len,
                                   const uint8_t salt[],
                                   size_t salt_len) override;

      ~Key_Agreement_with_KDF() override = default;

   protected:
      explicit Key_Agreement_with_KDF(std::string_view kdf);

   private:
      virtual secure_vector<uint8_t> raw_agree(const uint8_t w[], size_t w_len) = 0;
      std::unique_ptr<KDF> m_kdf;
};

class KEM_Encryption_with_KDF : public KEM_Encryption {
   public:
      void kem_encrypt(std::span<uint8_t> out_encapsulated_key,
                       std::span<uint8_t> out_shared_key,
                       RandomNumberGenerator& rng,
                       size_t desired_shared_key_len,
                       std::span<const uint8_t> salt) final;

      size_t shared_key_length(size_t desired_shared_key_len) const final;

      ~KEM_Encryption_with_KDF() override = default;

   protected:
      virtual void raw_kem_encrypt(std::span<uint8_t> out_encapsulated_key,
                                   std::span<uint8_t> out_raw_shared_key,
                                   RandomNumberGenerator& rng) = 0;

      virtual size_t raw_kem_shared_key_length() const = 0;

      explicit KEM_Encryption_with_KDF(std::string_view kdf);

   private:
      std::unique_ptr<KDF> m_kdf;
};

class KEM_Decryption_with_KDF : public KEM_Decryption {
   public:
      void kem_decrypt(std::span<uint8_t> out_shared_key,
                       std::span<const uint8_t> encapsulated_key,
                       size_t desired_shared_key_len,
                       std::span<const uint8_t> salt) final;

      size_t shared_key_length(size_t desired_shared_key_len) const final;

      ~KEM_Decryption_with_KDF() override = default;

   protected:
      virtual void raw_kem_decrypt(std::span<uint8_t> out_raw_shared_key,
                                   std::span<const uint8_t> encapsulated_key) = 0;

      virtual size_t raw_kem_shared_key_length() const = 0;

      explicit KEM_Decryption_with_KDF(std::string_view kdf);

   private:
      std::unique_ptr<KDF> m_kdf;
};

}  // namespace Botan::PK_Ops

#endif
