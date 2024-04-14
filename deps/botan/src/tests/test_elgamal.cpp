/*
* (C) 2014,2015,2020 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include "tests.h"

#if defined(BOTAN_HAS_ELGAMAL)
   #include "test_pubkey.h"
   #include <botan/dl_group.h>
   #include <botan/elgamal.h>
#endif

namespace Botan_Tests {

namespace {

#if defined(BOTAN_HAS_ELGAMAL)

class ElGamal_Encrypt_Tests final : public PK_Encryption_Decryption_Test {
   public:
      ElGamal_Encrypt_Tests() :
            PK_Encryption_Decryption_Test(
               "ElGamal", "pubkey/elgamal_encrypt.vec", "Group,Secret,Nonce,Msg,Ciphertext", "Padding") {}

      std::unique_ptr<Botan::Private_Key> load_private_key(const VarMap& vars) override {
         const Botan::BigInt x = vars.get_req_bn("Secret");
         const Botan::DL_Group group(vars.get_req_str("Group"));

         return std::make_unique<Botan::ElGamal_PrivateKey>(group, x);
      }
};

class ElGamal_Decrypt_Tests final : public PK_Decryption_Test {
   public:
      ElGamal_Decrypt_Tests() : PK_Decryption_Test("ElGamal", "pubkey/elgamal_decrypt.vec", "P,G,X,Msg,Ciphertext") {}

      std::unique_ptr<Botan::Private_Key> load_private_key(const VarMap& vars) override {
         const Botan::BigInt p = vars.get_req_bn("P");
         const Botan::BigInt g = vars.get_req_bn("G");
         const Botan::BigInt x = vars.get_req_bn("X");

         const Botan::DL_Group group(p, g);

         return std::make_unique<Botan::ElGamal_PrivateKey>(group, x);
      }
};

class ElGamal_Keygen_Tests final : public PK_Key_Generation_Test {
   public:
      std::vector<std::string> keygen_params() const override { return {"modp/ietf/1024"}; }

      std::string algo_name() const override { return "ElGamal"; }
};

BOTAN_REGISTER_TEST("pubkey", "elgamal_encrypt", ElGamal_Encrypt_Tests);
BOTAN_REGISTER_TEST("pubkey", "elgamal_decrypt", ElGamal_Decrypt_Tests);
BOTAN_REGISTER_TEST("pubkey", "elgamal_keygen", ElGamal_Keygen_Tests);

#endif

}  // namespace

}  // namespace Botan_Tests
