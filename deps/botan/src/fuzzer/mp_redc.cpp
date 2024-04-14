/*
* (C) 2023 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include "mp_fuzzers.h"

namespace {

template <size_t N>
void fuzz_mp_redc(const uint8_t in[], size_t in_len) {
   FUZZER_ASSERT_EQUAL(in_len, (N * 3 + 1) * sizeof(word));

   word z[2 * N] = {0};

   word z_script[2 * N] = {0};
   word z_ref[2 * N] = {0};
   word p[N] = {0};
   word p_dash = 0;

   word ws[2 * (N + 1)] = {0};

   std::memcpy(z, in, sizeof(z));
   std::memcpy(p, in + sizeof(z), sizeof(p));
   std::memcpy(&p_dash, in + sizeof(z) + sizeof(p), sizeof(p_dash));

   for(size_t i = 0; i != 2 * N; ++i) {
      z_script[i] = z_ref[i] = z[i];
   }

   if(N == 4) {
      Botan::bigint_monty_redc_4(z_script, p, p_dash, ws);
   } else if(N == 6) {
      Botan::bigint_monty_redc_6(z_script, p, p_dash, ws);
   } else if(N == 8) {
      Botan::bigint_monty_redc_8(z_script, p, p_dash, ws);
   } else if(N == 16) {
      Botan::bigint_monty_redc_16(z_script, p, p_dash, ws);
   } else if(N == 24) {
      Botan::bigint_monty_redc_24(z_script, p, p_dash, ws);
   } else if(N == 32) {
      Botan::bigint_monty_redc_32(z_script, p, p_dash, ws);
   } else {
      std::abort();
   }

   Botan::bigint_monty_redc_generic(z_ref, 2 * N, p, N, p_dash, ws);

   for(size_t i = 0; i != 2 * N; ++i) {
      if(z_script[i] != z_ref[i]) {
         dump_word_vec("input", z, 2 * N);
         dump_word_vec("z_script", z_script, 2 * N);
         dump_word_vec("z_ref", z_ref, 2 * N);
         dump_word_vec("p", p, N);
         dump_word_vec("p_dash", &p_dash, 1);
         std::abort();
      }
   }
   compare_word_vec(z_script, 2 * N, z_ref, 2 * N, "redc generic vs specialized");
}

}  // namespace

void fuzz(const uint8_t in[], size_t len) {
   if(len == 0 || len % sizeof(word) != 0) {
      return;
   }

   const size_t words = len / sizeof(word);

   switch(words) {
      case 4 * 3 + 1:
         return fuzz_mp_redc<4>(in, len);
      case 6 * 3 + 1:
         return fuzz_mp_redc<6>(in, len);
      case 8 * 3 + 1:
         return fuzz_mp_redc<8>(in, len);
      case 16 * 3 + 1:
         return fuzz_mp_redc<16>(in, len);
      case 24 * 3 + 1:
         return fuzz_mp_redc<24>(in, len);
      case 32 * 3 + 1:
         return fuzz_mp_redc<32>(in, len);
      default:
         return;
   }
}
