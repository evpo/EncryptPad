/*
* EMSA-Raw
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <botan/internal/emsa_raw.h>

#include <botan/exceptn.h>
#include <botan/mem_ops.h>
#include <botan/internal/ct_utils.h>

namespace Botan {

std::string EMSA_Raw::name() const {
   if(m_expected_size > 0) {
      return "Raw(" + std::to_string(m_expected_size) + ")";
   }
   return "Raw";
}

/*
* EMSA-Raw Encode Operation
*/
void EMSA_Raw::update(const uint8_t input[], size_t length) {
   m_message += std::make_pair(input, length);
}

/*
* Return the raw (unencoded) data
*/
std::vector<uint8_t> EMSA_Raw::raw_data() {
   if(m_expected_size && m_message.size() != m_expected_size) {
      throw Invalid_Argument("EMSA_Raw was configured to use a " + std::to_string(m_expected_size) +
                             " byte hash but instead was used for a " + std::to_string(m_message.size()) + " hash");
   }

   std::vector<uint8_t> output;
   std::swap(m_message, output);
   return output;
}

/*
* EMSA-Raw Encode Operation
*/
std::vector<uint8_t> EMSA_Raw::encoding_of(const std::vector<uint8_t>& msg,
                                           size_t /*output_bits*/,
                                           RandomNumberGenerator& /*rng*/) {
   if(m_expected_size && msg.size() != m_expected_size) {
      throw Invalid_Argument("EMSA_Raw was configured to use a " + std::to_string(m_expected_size) +
                             " byte hash but instead was used for a " + std::to_string(msg.size()) + " hash");
   }

   return msg;
}

/*
* EMSA-Raw Verify Operation
*/
bool EMSA_Raw::verify(const std::vector<uint8_t>& coded, const std::vector<uint8_t>& raw, size_t /*key_bits*/) {
   if(m_expected_size && raw.size() != m_expected_size) {
      return false;
   }

   if(coded.size() == raw.size()) {
      return (coded == raw);
   }

   if(coded.size() > raw.size()) {
      return false;
   }

   // handle zero padding differences
   const size_t leading_zeros_expected = raw.size() - coded.size();

   bool same_modulo_leading_zeros = true;

   for(size_t i = 0; i != leading_zeros_expected; ++i) {
      if(raw[i]) {
         same_modulo_leading_zeros = false;
      }
   }

   if(!CT::is_equal(coded.data(), raw.data() + leading_zeros_expected, coded.size()).as_bool()) {
      same_modulo_leading_zeros = false;
   }

   return same_modulo_leading_zeros;
}

}  // namespace Botan
