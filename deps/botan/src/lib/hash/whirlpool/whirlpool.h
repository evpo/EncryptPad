/*
* Whirlpool
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_WHIRLPOOL_H_
#define BOTAN_WHIRLPOOL_H_

#include <botan/internal/mdx_hash.h>

namespace Botan {

/**
* Whirlpool
*/
class Whirlpool final : public HashFunction {
   public:
      using digest_type = secure_vector<uint64_t>;

      static constexpr MD_Endian byte_endianness = MD_Endian::Big;
      static constexpr MD_Endian bit_endianness = MD_Endian::Big;
      static constexpr size_t block_bytes = 64;
      static constexpr size_t output_bytes = 64;
      static constexpr size_t ctr_bytes = 32;

      static void compress_n(digest_type& digest, std::span<const uint8_t> input, size_t blocks);
      static void init(digest_type& digest);

   public:
      std::string name() const override { return "Whirlpool"; }

      size_t output_length() const override { return output_bytes; }

      size_t hash_block_size() const override { return block_bytes; }

      std::unique_ptr<HashFunction> new_object() const override;

      std::unique_ptr<HashFunction> copy_state() const override;

      void clear() override { m_md.clear(); }

   private:
      void add_data(std::span<const uint8_t> input) override;

      void final_result(std::span<uint8_t> output) override;

   private:
      MerkleDamgard_Hash<Whirlpool> m_md;
};

}  // namespace Botan

#endif
