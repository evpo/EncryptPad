/*
* (C) 2014,2015 Jack Lloyd
* (C) 2017 René Korthaus, Rohde & Schwarz Cybersecurity
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include "tests.h"

#include "test_rng.h"

#if defined(BOTAN_HAS_ECDSA)
   #include "test_pubkey.h"
   #include <botan/ecdsa.h>
   #include <botan/pk_algs.h>
#endif

namespace Botan_Tests {

namespace {

#if defined(BOTAN_HAS_ECDSA)

class ECDSA_Verification_Tests final : public PK_Signature_Verification_Test {
   public:
      ECDSA_Verification_Tests() :
            PK_Signature_Verification_Test("ECDSA", "pubkey/ecdsa_verify.vec", "Group,Px,Py,Msg,Signature", "Valid") {}

      bool clear_between_callbacks() const override { return false; }

      std::unique_ptr<Botan::Public_Key> load_public_key(const VarMap& vars) override {
         const std::string group_id = vars.get_req_str("Group");
         const BigInt px = vars.get_req_bn("Px");
         const BigInt py = vars.get_req_bn("Py");
         Botan::EC_Group group(Botan::OID::from_string(group_id));

         const Botan::EC_Point public_point = group.point(px, py);

         return std::make_unique<Botan::ECDSA_PublicKey>(group, public_point);
      }

      std::string default_padding(const VarMap& /*unused*/) const override { return "Raw"; }
};

class ECDSA_Wycheproof_Verification_Tests final : public PK_Signature_Verification_Test {
   public:
      ECDSA_Wycheproof_Verification_Tests() :
            PK_Signature_Verification_Test(
               "ECDSA", "pubkey/ecdsa_wycheproof.vec", "Group,Px,Py,Hash,Msg,Signature,Valid") {}

      bool clear_between_callbacks() const override { return false; }

      Botan::Signature_Format sig_format() const override { return Botan::Signature_Format::DerSequence; }

      bool test_random_invalid_sigs() const override { return false; }

      std::unique_ptr<Botan::Public_Key> load_public_key(const VarMap& vars) override {
         const std::string group_id = vars.get_req_str("Group");
         const BigInt px = vars.get_req_bn("Px");
         const BigInt py = vars.get_req_bn("Py");
         Botan::EC_Group group(Botan::OID::from_string(group_id));

         const Botan::EC_Point public_point = group.point(px, py);

         return std::make_unique<Botan::ECDSA_PublicKey>(group, public_point);
      }

      std::string default_padding(const VarMap& vars) const override { return vars.get_req_str("Hash"); }
};

class ECDSA_Signature_KAT_Tests final : public PK_Signature_Generation_Test {
   public:
      ECDSA_Signature_KAT_Tests() :
            PK_Signature_Generation_Test("ECDSA",
   #if defined(BOTAN_HAS_RFC6979_GENERATOR)
                                         "pubkey/ecdsa_rfc6979.vec",
                                         "Group,X,Hash,Msg,Signature") {
      }
   #else
                                         "pubkey/ecdsa_prob.vec",
                                         "Group,X,Hash,Msg,Nonce,Signature") {
      }
   #endif

      bool clear_between_callbacks() const override { return false; }

      std::unique_ptr<Botan::Private_Key> load_private_key(const VarMap& vars) override {
         const std::string group_id = vars.get_req_str("Group");
         const BigInt x = vars.get_req_bn("X");
         Botan::EC_Group group(Botan::OID::from_string(group_id));

         return std::make_unique<Botan::ECDSA_PrivateKey>(this->rng(), group, x);
      }

      std::string default_padding(const VarMap& vars) const override { return vars.get_req_str("Hash"); }

   #if !defined(BOTAN_HAS_RFC6979_GENERATOR)
      std::unique_ptr<Botan::RandomNumberGenerator> test_rng(const std::vector<uint8_t>& nonce) const override {
         // probabilistic ecdsa signature generation extracts more random than just the nonce,
         // but the nonce is extracted first
         return std::make_unique<Fixed_Output_Position_RNG>(nonce, 1, this->rng());
      }
   #endif
};

class ECDSA_KAT_Verification_Tests final : public PK_Signature_Verification_Test {
   public:
      ECDSA_KAT_Verification_Tests() :
            PK_Signature_Verification_Test("ECDSA",
   #if !defined(BOTAN_HAS_RFC6979_GENERATOR)
                                           "pubkey/ecdsa_rfc6979.vec",
                                           "Group,X,Hash,Msg,Signature") {
      }
   #else
                                           "pubkey/ecdsa_prob.vec",
                                           "Group,X,Hash,Msg,Nonce,Signature") {
      }
   #endif

      bool clear_between_callbacks() const override { return false; }

      std::unique_ptr<Botan::Public_Key> load_public_key(const VarMap& vars) override {
         const std::string group_id = vars.get_req_str("Group");
         const BigInt x = vars.get_req_bn("X");
         Botan::EC_Group group(Botan::OID::from_string(group_id));

         Botan::ECDSA_PrivateKey priv_key(this->rng(), group, x);

         return priv_key.public_key();
      }

      std::string default_padding(const VarMap& vars) const override { return vars.get_req_str("Hash"); }
};

class ECDSA_Sign_Verify_DER_Test final : public PK_Sign_Verify_DER_Test {
   public:
      ECDSA_Sign_Verify_DER_Test() : PK_Sign_Verify_DER_Test("ECDSA", "SHA-512") {}

      std::unique_ptr<Botan::Private_Key> key() override {
         return Botan::create_private_key("ECDSA", this->rng(), "secp256r1");
      }
};

class ECDSA_Keygen_Tests final : public PK_Key_Generation_Test {
   public:
      std::vector<std::string> keygen_params() const override {
         return {"secp256r1", "secp384r1", "secp521r1", "frp256v1"};
      }

      std::string algo_name() const override { return "ECDSA"; }
};

class ECDSA_Keygen_Stability_Tests final : public PK_Key_Generation_Stability_Test {
   public:
      ECDSA_Keygen_Stability_Tests() : PK_Key_Generation_Stability_Test("ECDSA", "pubkey/ecdsa_keygen.vec") {}
};

   #if defined(BOTAN_HAS_EMSA_RAW)

class ECDSA_Key_Recovery_Tests final : public Text_Based_Test {
   public:
      ECDSA_Key_Recovery_Tests() :
            Text_Based_Test("pubkey/ecdsa_key_recovery.vec", "Group,Msg,R,S,V,PubkeyX,PubkeyY") {}

      Test::Result run_one_test(const std::string& /*header*/, const VarMap& vars) override {
         Test::Result result("ECDSA key recovery");

         const std::string group_id = vars.get_req_str("Group");
         Botan::EC_Group group(group_id);

         const BigInt R = vars.get_req_bn("R");
         const BigInt S = vars.get_req_bn("S");
         const uint8_t V = vars.get_req_u8("V");
         const std::vector<uint8_t> msg = vars.get_req_bin("Msg");
         const BigInt pubkey_x = vars.get_req_bn("PubkeyX");
         const BigInt pubkey_y = vars.get_req_bn("PubkeyY");

         try {
            Botan::ECDSA_PublicKey pubkey(group, msg, R, S, V);
            result.test_eq("Pubkey X coordinate", pubkey.public_point().get_affine_x(), pubkey_x);
            result.test_eq("Pubkey Y coordinate", pubkey.public_point().get_affine_y(), pubkey_y);

            const uint8_t computed_V = pubkey.recovery_param(msg, R, S);
            result.test_eq("Recovery param is correct", static_cast<size_t>(computed_V), static_cast<size_t>(V));

            Botan::PK_Verifier verifier(pubkey, "Raw");

            auto sig = Botan::BigInt::encode_fixed_length_int_pair(R, S, group.get_order_bytes());

            result.confirm("Signature verifies", verifier.verify_message(msg, sig));
         } catch(Botan::Exception& e) {
            result.test_failure("Failed to recover ECDSA public key", e.what());
         }

         return result;
      }
};

BOTAN_REGISTER_TEST("pubkey", "ecdsa_key_recovery", ECDSA_Key_Recovery_Tests);

   #endif

class ECDSA_Invalid_Key_Tests final : public Text_Based_Test {
   public:
      ECDSA_Invalid_Key_Tests() : Text_Based_Test("pubkey/ecdsa_invalid.vec", "Group,InvalidKeyX,InvalidKeyY") {}

      bool clear_between_callbacks() const override { return false; }

      Test::Result run_one_test(const std::string& /*header*/, const VarMap& vars) override {
         Test::Result result("ECDSA invalid keys");

         const std::string group_id = vars.get_req_str("Group");
         Botan::EC_Group group(Botan::OID::from_string(group_id));
         const Botan::BigInt x = vars.get_req_bn("InvalidKeyX");
         const Botan::BigInt y = vars.get_req_bn("InvalidKeyY");

         std::unique_ptr<Botan::EC_Point> public_point;

         try {
            public_point = std::make_unique<Botan::EC_Point>(group.point(x, y));
         } catch(Botan::Invalid_Argument&) {
            // EC_Point() performs a range check on x, y in [0, p−1],
            // which is also part of the EC public key checks, e.g.,
            // in NIST SP800-56A rev2, sec. 5.6.2.3.2
            result.test_success("public key fails check");
            return result;
         }

         auto key = std::make_unique<Botan::ECDSA_PublicKey>(group, *public_point);
         result.test_eq("public key fails check", key->check_key(this->rng(), false), false);
         return result;
      }
};

BOTAN_REGISTER_TEST("pubkey", "ecdsa_verify", ECDSA_Verification_Tests);
BOTAN_REGISTER_TEST("pubkey", "ecdsa_verify_wycheproof", ECDSA_Wycheproof_Verification_Tests);
BOTAN_REGISTER_TEST("pubkey", "ecdsa_sign", ECDSA_Signature_KAT_Tests);
BOTAN_REGISTER_TEST("pubkey", "ecdsa_verify_kat", ECDSA_KAT_Verification_Tests);
BOTAN_REGISTER_TEST("pubkey", "ecdsa_sign_verify_der", ECDSA_Sign_Verify_DER_Test);
BOTAN_REGISTER_TEST("pubkey", "ecdsa_keygen", ECDSA_Keygen_Tests);
BOTAN_REGISTER_TEST("pubkey", "ecdsa_keygen_stability", ECDSA_Keygen_Stability_Tests);
BOTAN_REGISTER_TEST("pubkey", "ecdsa_invalid", ECDSA_Invalid_Key_Tests);

#endif

}  // namespace

}  // namespace Botan_Tests
