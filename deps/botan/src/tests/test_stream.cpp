/*
* (C) 2014,2015,2016 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include "tests.h"

#if defined(BOTAN_HAS_STREAM_CIPHER)
   #include <botan/stream_cipher.h>
   #include <botan/internal/fmt.h>
#endif

namespace Botan_Tests {

#if defined(BOTAN_HAS_STREAM_CIPHER)

class Stream_Cipher_Tests final : public Text_Based_Test {
   public:
      Stream_Cipher_Tests() : Text_Based_Test("stream", "Key,Out", "In,Nonce,Seek") {}

      Test::Result run_one_test(const std::string& algo, const VarMap& vars) override {
         const std::vector<uint8_t> key = vars.get_req_bin("Key");
         const std::vector<uint8_t> expected = vars.get_req_bin("Out");
         const std::vector<uint8_t> nonce = vars.get_opt_bin("Nonce");
         const uint64_t seek = vars.get_opt_u64("Seek", 0);
         std::vector<uint8_t> input = vars.get_opt_bin("In");

         if(input.empty()) {
            input.resize(expected.size());
         }

         Test::Result result(algo);

         const std::vector<std::string> providers = provider_filter(Botan::StreamCipher::providers(algo));

         if(providers.empty()) {
            result.note_missing("stream cipher " + algo);
            return result;
         }

         for(const auto& provider_ask : providers) {
            auto cipher = Botan::StreamCipher::create(algo, provider_ask);

            if(!cipher) {
               result.test_failure(Botan::fmt("Stream cipher {} supported by {} but not found", algo, provider_ask));
               continue;
            }

            const std::string provider(cipher->provider());
            result.test_is_nonempty("provider", provider);
            result.test_eq(provider, cipher->name(), algo);

            result.confirm("default iv length is valid", cipher->valid_iv_length(cipher->default_iv_length()));

            result.confirm("advertised buffer size is > 0", cipher->buffer_size() > 0);

            if(cipher->default_iv_length() == 0) {
               result.confirm("if default iv length is zero, no iv supported", nonce.empty());

               // This should still succeed
               cipher->set_iv(nullptr, 0);
            }

            try {
               std::vector<uint8_t> buf(128);
               cipher->cipher1(buf.data(), buf.size());
               result.test_failure("Was able to encrypt without a key being set");
            } catch(Botan::Invalid_State&) {
               result.test_success("Trying to encrypt with no key set fails");
            }

            try {
               cipher->seek(0);
               result.test_failure("Was able to seek without a key being set");
            } catch(Botan::Invalid_State&) {
               result.test_success("Trying to seek with no key set fails");
            } catch(Botan::Not_Implemented&) {
               result.test_success("Trying to seek failed because not implemented");
            }

            if(!cipher->valid_iv_length(nonce.size())) {
               throw Test_Error("Invalid nonce for " + algo);
            }

            bool accepted_nonce_early = false;
            if(!nonce.empty()) {
               try {
                  cipher->set_iv(nonce.data(), nonce.size());
                  accepted_nonce_early = true;
               } catch(Botan::Invalid_State&) {}
            }

            /*
            * Different providers may have additional restrictions on key sizes.
            * Avoid testing the cipher with a key size that it does not natively support.
            */
            if(!cipher->valid_keylength(key.size())) {
               result.test_note("Skipping test with provider " + provider + " as it does not support key length " +
                                std::to_string(key.size()));
               continue;
            }

            result.test_eq("key not set", cipher->has_keying_material(), false);
            cipher->set_key(key);
            result.test_eq("key set", cipher->has_keying_material(), true);

            /*
            Test invalid nonce sizes. this assumes no implemented cipher supports a nonce of 65000
            */
            const size_t large_nonce_size = 65000;
            result.confirm("Stream cipher does not support very large nonce",
                           cipher->valid_iv_length(large_nonce_size) == false);

            result.test_throws("Throws if invalid nonce size given",
                               [&]() { cipher->set_iv(nullptr, large_nonce_size); });

            /*
            If the set_nonce call earlier succeded, then we require that it also
            worked (ie saved the nonce for later use) even though the key was
            not set. So, don't set the nonce now, to ensure the previous call
            had an effect.
            */
            if(!nonce.empty() && accepted_nonce_early == false) {
               cipher->set_iv(nonce.data(), nonce.size());
            }

            if(seek != 0) {
               cipher->seek(seek);
            }

            // Test that clone works and does not affect parent object
            auto clone = cipher->new_object();
            result.confirm("Clone has different pointer", cipher.get() != clone.get());
            result.test_eq("Clone has same name", cipher->name(), clone->name());
            clone->set_key(this->rng().random_vec(cipher->maximum_keylength()));

            {
               std::vector<uint8_t> buf = input;
               cipher->encrypt(buf);
               result.test_eq(provider, "encrypt", buf, expected);
            }

            {
               if(nonce.empty()) {
                  cipher->set_key(key);
               } else {
                  cipher->set_iv(nonce.data(), nonce.size());
               }
               if(seek != 0) {
                  cipher->seek(seek);
               }
               std::vector<uint8_t> buf = input;
               cipher->encrypt(buf);
               result.test_eq(provider, "encrypt 2", buf, expected);
            }

            if(!nonce.empty()) {
               cipher->set_iv(nonce.data(), nonce.size());
               if(seek != 0) {
                  cipher->seek(seek);
               }
               std::vector<uint8_t> buf = input;
               cipher->encrypt(buf);
               result.test_eq(provider, "second encrypt", buf, expected);
            }

            {
               cipher->set_key(key);

               cipher->set_iv(nonce.data(), nonce.size());

               if(seek != 0) {
                  cipher->seek(seek);
               }

               std::vector<uint8_t> buf(input.size(), 0xAB);

               uint8_t* buf_ptr = buf.data();
               size_t buf_len = buf.size();

               while(buf_len > 0) {
                  size_t next = std::min<size_t>(buf_len, this->rng().next_byte());
                  cipher->write_keystream(buf_ptr, next);
                  buf_ptr += next;
                  buf_len -= next;
               }

               for(size_t i = 0; i != input.size(); ++i) {
                  buf[i] ^= input[i];
               }
               result.test_eq(provider, "write_keystream", buf, expected);
            }

            result.test_eq("key set", cipher->has_keying_material(), true);
            cipher->clear();
            result.test_eq("key not set", cipher->has_keying_material(), false);

            try {
               std::vector<uint8_t> buf(128);
               cipher->cipher1(buf.data(), buf.size());
               result.test_failure("Was able to encrypt without a key being set (after clear)");
            } catch(Botan::Invalid_State&) {
               result.test_success("Trying to encrypt with no key set (after clear) fails");
            }
         }

         return result;
      }
};

BOTAN_REGISTER_SERIALIZED_SMOKE_TEST("stream", "stream_ciphers", Stream_Cipher_Tests);

#endif

}  // namespace Botan_Tests
