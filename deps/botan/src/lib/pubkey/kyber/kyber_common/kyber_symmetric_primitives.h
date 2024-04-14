/*
 * Symmetric primitives for Kyber (modern)
 * (C) 2022 Jack Lloyd
 * (C) 2022 Hannes Rantzsch, René Meusel, neXenio GmbH
 *
 * Botan is released under the Simplified BSD License (see license.txt)
 */

#ifndef BOTAN_KYBER_SYMMETRIC_PRIMITIVES_H_
#define BOTAN_KYBER_SYMMETRIC_PRIMITIVES_H_

#include <botan/hash.h>
#include <botan/secmem.h>

#include <memory>
#include <span>
#include <tuple>
#include <vector>

namespace Botan {

class XOF;

/**
 * Adapter class that uses polymorphy to distinguish
 * Kyber "modern" from Kyber "90s" modes.
 */
class Kyber_Symmetric_Primitives {
   public:
      virtual ~Kyber_Symmetric_Primitives() = default;

      virtual std::unique_ptr<HashFunction> G() const = 0;
      virtual std::unique_ptr<HashFunction> H() const = 0;
      virtual std::unique_ptr<HashFunction> KDF() const = 0;

      virtual Botan::XOF& XOF(std::span<const uint8_t> seed, std::tuple<uint8_t, uint8_t> matrix_position) const = 0;

      virtual secure_vector<uint8_t> PRF(std::span<const uint8_t> seed, uint8_t nonce, size_t outlen) const = 0;
};

}  // namespace Botan

#endif
