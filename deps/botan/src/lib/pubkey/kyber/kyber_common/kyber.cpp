/*
 * Crystals Kyber key encapsulation mechanism
 * Based on the public domain reference implementation by the
 * designers (https://github.com/pq-crystals/kyber)
 *
 * Further changes
 * (C) 2021-2022 Jack Lloyd
 * (C) 2021-2022 Manuel Glaser and Michael Boric, Rohde & Schwarz Cybersecurity
 * (C) 2021-2022 René Meusel and Hannes Rantzsch, neXenio GmbH
 *
 * Botan is released under the Simplified BSD License (see license.txt)
 */

#include <botan/kyber.h>

#include <botan/assert.h>
#include <botan/ber_dec.h>
#include <botan/hash.h>
#include <botan/mem_ops.h>
#include <botan/pubkey.h>
#include <botan/rng.h>
#include <botan/secmem.h>

#include <botan/internal/ct_utils.h>
#include <botan/internal/fmt.h>
#include <botan/internal/kyber_symmetric_primitives.h>
#include <botan/internal/loadstor.h>
#include <botan/internal/pk_ops_impl.h>
#include <botan/internal/stl_util.h>

#if defined(BOTAN_HAS_KYBER)
   #include <botan/internal/kyber_modern.h>
#endif

#if defined(BOTAN_HAS_KYBER_90S)
   #include <botan/internal/kyber_90s.h>
#endif

#include <algorithm>
#include <array>
#include <iterator>
#include <limits>
#include <memory>
#include <optional>
#include <vector>

namespace Botan {

namespace {

KyberMode::Mode kyber_mode_from_string(std::string_view str) {
   if(str == "Kyber-512-90s-r3") {
      return KyberMode::Kyber512_90s;
   }
   if(str == "Kyber-768-90s-r3") {
      return KyberMode::Kyber768_90s;
   }
   if(str == "Kyber-1024-90s-r3") {
      return KyberMode::Kyber1024_90s;
   }
   if(str == "Kyber-512-r3") {
      return KyberMode::Kyber512_R3;
   }
   if(str == "Kyber-768-r3") {
      return KyberMode::Kyber768_R3;
   }
   if(str == "Kyber-1024-r3") {
      return KyberMode::Kyber1024_R3;
   }

   throw Invalid_Argument(fmt("'{}' is not a valid Kyber mode name", str));
}

/**
 * Constant time implementation for computing an unsigned integer division
 * with KyberConstants::Q = 3329.
 *
 * It enforces the optimization of various compilers,
 * replacing the division operation with multiplication and shifts.
 *
 * This implementation is only valid for integers <= 2**20
 *
 * @returns (a / KyberConstants::Q)
 */
uint16_t ct_int_div_kyber_q(uint32_t a) {
   BOTAN_DEBUG_ASSERT(a < (1 << 18));

   /*
   Constants based on "Hacker's Delight" (Second Edition) by Henry
   S. Warren, Jr. Chapter 10-9 "Unsigned Division by Divisors >= 1"
   */
   const uint64_t m = 161271;
   const size_t p = 29;
   return static_cast<uint16_t>((a * m) >> p);
}

}  // namespace

KyberMode::KyberMode(Mode mode) : m_mode(mode) {}

KyberMode::KyberMode(const OID& oid) : m_mode(kyber_mode_from_string(oid.to_formatted_string())) {}

KyberMode::KyberMode(std::string_view str) : m_mode(kyber_mode_from_string(str)) {}

OID KyberMode::object_identifier() const {
   return OID::from_string(to_string());
}

std::string KyberMode::to_string() const {
   switch(m_mode) {
      case Kyber512_90s:
         return "Kyber-512-90s-r3";
      case Kyber768_90s:
         return "Kyber-768-90s-r3";
      case Kyber1024_90s:
         return "Kyber-1024-90s-r3";
      case Kyber512_R3:
         return "Kyber-512-r3";
      case Kyber768_R3:
         return "Kyber-768-r3";
      case Kyber1024_R3:
         return "Kyber-1024-r3";
   }

   BOTAN_ASSERT_UNREACHABLE();
}

bool KyberMode::is_90s() const {
   return m_mode == Kyber512_90s || m_mode == Kyber768_90s || m_mode == Kyber1024_90s;
}

bool KyberMode::is_modern() const {
   return !is_90s();
}

bool KyberMode::is_available() const {
#if defined(BOTAN_HAS_KYBER)
   if(is_modern()) {
      return true;
   }
#endif

#if defined(BOTAN_HAS_KYBER_90S)
   if(is_90s()) {
      return true;
   }
#endif

   return false;
}

namespace {

class KyberConstants {
   public:
      static constexpr size_t N = 256;
      static constexpr size_t Q = 3329;
      static constexpr size_t Q_Inv = 62209;

      static constexpr int16_t zetas[128] = {
         2285, 2571, 2970, 1812, 1493, 1422, 287,  202,  3158, 622,  1577, 182,  962,  2127, 1855, 1468,
         573,  2004, 264,  383,  2500, 1458, 1727, 3199, 2648, 1017, 732,  608,  1787, 411,  3124, 1758,
         1223, 652,  2777, 1015, 2036, 1491, 3047, 1785, 516,  3321, 3009, 2663, 1711, 2167, 126,  1469,
         2476, 3239, 3058, 830,  107,  1908, 3082, 2378, 2931, 961,  1821, 2604, 448,  2264, 677,  2054,
         2226, 430,  555,  843,  2078, 871,  1550, 105,  422,  587,  177,  3094, 3038, 2869, 1574, 1653,
         3083, 778,  1159, 3182, 2552, 1483, 2727, 1119, 1739, 644,  2457, 349,  418,  329,  3173, 3254,
         817,  1097, 603,  610,  1322, 2044, 1864, 384,  2114, 3193, 1218, 1994, 2455, 220,  2142, 1670,
         2144, 1799, 2051, 794,  1819, 2475, 2459, 478,  3221, 3021, 996,  991,  958,  1869, 1522, 1628};

      static constexpr int16_t zetas_inv[128] = {
         1701, 1807, 1460, 2371, 2338, 2333, 308,  108,  2851, 870,  854,  1510, 2535, 1278, 1530, 1185,
         1659, 1187, 3109, 874,  1335, 2111, 136,  1215, 2945, 1465, 1285, 2007, 2719, 2726, 2232, 2512,
         75,   156,  3000, 2911, 2980, 872,  2685, 1590, 2210, 602,  1846, 777,  147,  2170, 2551, 246,
         1676, 1755, 460,  291,  235,  3152, 2742, 2907, 3224, 1779, 2458, 1251, 2486, 2774, 2899, 1103,
         1275, 2652, 1065, 2881, 725,  1508, 2368, 398,  951,  247,  1421, 3222, 2499, 271,  90,   853,
         1860, 3203, 1162, 1618, 666,  320,  8,    2813, 1544, 282,  1838, 1293, 2314, 552,  2677, 2106,
         1571, 205,  2918, 1542, 2721, 2597, 2312, 681,  130,  1602, 1871, 829,  2946, 3065, 1325, 2756,
         1861, 1474, 1202, 2367, 3147, 1752, 2707, 171,  3127, 3042, 1907, 1836, 1517, 359,  758,  1441};

      static constexpr size_t kSymBytes = 32;
      static constexpr size_t kSeedLength = kSymBytes;
      static constexpr size_t kSerializedPolynomialByteLength = N / 2 * 3;
      static constexpr size_t kPublicKeyHashLength = 32;
      static constexpr size_t kZLength = kSymBytes;

   public:
      KyberConstants(const KyberMode mode) : m_mode(mode) {
         switch(mode.mode()) {
            case KyberMode::Kyber512_R3:
            case KyberMode::Kyber512_90s:
               m_nist_strength = 128;
               m_k = 2;
               m_eta1 = 3;
               break;

            case KyberMode::Kyber768_R3:
            case KyberMode::Kyber768_90s:
               m_nist_strength = 192;
               m_k = 3;
               m_eta1 = 2;
               break;

            case KyberMode::Kyber1024_R3:
            case KyberMode::Kyber1024_90s:
               m_nist_strength = 256;
               m_k = 4;
               m_eta1 = 2;
               break;

            default:
               BOTAN_ASSERT_UNREACHABLE();
         }

#ifdef BOTAN_HAS_KYBER_90S
         if(mode.is_90s()) {
            m_symmetric_primitives = std::make_unique<Kyber_90s_Symmetric_Primitives>();
         }
#endif
#ifdef BOTAN_HAS_KYBER
         if(!mode.is_90s()) {
            m_symmetric_primitives = std::make_unique<Kyber_Modern_Symmetric_Primitives>();
         }
#endif

         if(!m_symmetric_primitives) {
            throw Not_Implemented("requested Kyber mode is not enabled in this build");
         }
      }

      ~KyberConstants() = default;

      KyberConstants(const KyberConstants& other) : KyberConstants(other.m_mode) {}

      KyberConstants(KyberConstants&& other) = default;
      KyberConstants& operator=(const KyberConstants& other) = delete;
      KyberConstants& operator=(KyberConstants&& other) = default;

      KyberMode mode() const { return m_mode; }

      size_t estimated_strength() const { return m_nist_strength; }

      uint8_t k() const { return m_k; }

      uint8_t eta1() const { return m_eta1; }

      uint8_t eta2() const { return 2; }

      size_t polynomial_vector_byte_length() const { return kSerializedPolynomialByteLength * k(); }

      size_t public_key_byte_length() const { return polynomial_vector_byte_length() + kSeedLength; }

      size_t private_key_byte_length() const {
         return polynomial_vector_byte_length() + public_key_byte_length() + kPublicKeyHashLength + kZLength;
      }

      std::unique_ptr<HashFunction> G() const { return m_symmetric_primitives->G(); }

      std::unique_ptr<HashFunction> H() const { return m_symmetric_primitives->H(); }

      std::unique_ptr<HashFunction> KDF() const { return m_symmetric_primitives->KDF(); }

      Botan::XOF& XOF(std::span<const uint8_t> seed, std::tuple<uint8_t, uint8_t> matrix_position) const {
         return m_symmetric_primitives->XOF(seed, matrix_position);
      }

      secure_vector<uint8_t> PRF(std::span<const uint8_t> seed, const uint8_t nonce, const size_t outlen) const {
         return m_symmetric_primitives->PRF(seed, nonce, outlen);
      }

   private:
      KyberMode m_mode;
      std::unique_ptr<Kyber_Symmetric_Primitives> m_symmetric_primitives;
      size_t m_nist_strength;
      uint8_t m_k;
      uint8_t m_eta1;
};

class Polynomial {
   public:
      /**
       * Applies conditional subtraction of q to each coefficient of the polynomial.
       */
      void csubq() {
         for(auto& coeff : m_coeffs) {
            coeff -= KyberConstants::Q;
            coeff += (coeff >> 15) & KyberConstants::Q;
         }
      }

      /**
       * Applies Barrett reduction to all coefficients of the polynomial
       */
      void reduce() {
         for(auto& c : m_coeffs) {
            c = barrett_reduce(c);
         }
      }

      template <typename T = std::vector<uint8_t>>
      T to_bytes() {
         this->csubq();

         T r(KyberConstants::kSerializedPolynomialByteLength);

         for(size_t i = 0; i < size() / 2; ++i) {
            const uint16_t t0 = m_coeffs[2 * i];
            const uint16_t t1 = m_coeffs[2 * i + 1];
            r[3 * i + 0] = static_cast<uint8_t>(t0 >> 0);
            r[3 * i + 1] = static_cast<uint8_t>((t0 >> 8) | (t1 << 4));
            r[3 * i + 2] = static_cast<uint8_t>(t1 >> 4);
         }

         return r;
      }

      /**
       * Given an array of uniformly random bytes, compute polynomial with coefficients
       * distributed according to a centered binomial distribution with parameter eta=2
       */
      static Polynomial cbd2(std::span<const uint8_t> buf) {
         Polynomial r;

         BOTAN_ASSERT(buf.size() == (2 * r.size() / 4), "wrong input buffer size for cbd2");

         for(size_t i = 0; i < r.size() / 8; ++i) {
            uint32_t t = load_le<uint32_t>(buf.data(), i);
            uint32_t d = t & 0x55555555;
            d += (t >> 1) & 0x55555555;

            for(size_t j = 0; j < 8; ++j) {
               int16_t a = (d >> (4 * j + 0)) & 0x3;
               int16_t b = (d >> (4 * j + 2)) & 0x3;
               r.m_coeffs[8 * i + j] = a - b;
            }
         }

         return r;
      }

      /**
       * Given an array of uniformly random bytes, compute polynomial with coefficients
       * distributed according to a centered binomial distribution with parameter eta=3
       *
       * This function is only needed for Kyber-512
       */
      static Polynomial cbd3(std::span<const uint8_t> buf) {
         Polynomial r;

         BOTAN_ASSERT(buf.size() == (3 * r.size() / 4), "wrong input buffer size for cbd3");

         // Note: load_le<> does not support loading a 3-byte value
         const auto load_le24 = [](const uint8_t in[], const size_t off) {
            const auto off3 = off * 3;
            return make_uint32(0, in[off3 + 2], in[off3 + 1], in[off3]);
         };

         for(size_t i = 0; i < r.size() / 4; ++i) {
            uint32_t t = load_le24(buf.data(), i);
            uint32_t d = t & 0x00249249;
            d += (t >> 1) & 0x00249249;
            d += (t >> 2) & 0x00249249;

            for(size_t j = 0; j < 4; ++j) {
               int16_t a = (d >> (6 * j + 0)) & 0x7;
               int16_t b = (d >> (6 * j + 3)) & 0x7;
               r.m_coeffs[4 * i + j] = a - b;
            }
         }
         return r;
      }

      /**
       * Sample a polynomial deterministically from a seed and a nonce, with output
       * polynomial close to centered binomial distribution with parameter eta=2.
       */
      static Polynomial getnoise_eta2(std::span<const uint8_t> seed, uint8_t nonce, const KyberConstants& mode) {
         const auto eta2 = mode.eta2();
         BOTAN_ASSERT(eta2 == 2, "Invalid eta2 value");

         const auto outlen = eta2 * KyberConstants::N / 4;
         return Polynomial::cbd2(mode.PRF(seed, nonce, outlen));
      }

      /**
       * Sample a polynomial deterministically from a seed and a nonce, with output
       * polynomial close to centered binomial distribution with parameter mode.eta1()
       */
      static Polynomial getnoise_eta1(std::span<const uint8_t> seed, uint8_t nonce, const KyberConstants& mode) {
         const auto eta1 = mode.eta1();
         BOTAN_ASSERT(eta1 == 2 || eta1 == 3, "Invalid eta1 value");

         const auto outlen = eta1 * KyberConstants::N / 4;
         return (eta1 == 2) ? Polynomial::cbd2(mode.PRF(seed, nonce, outlen))
                            : Polynomial::cbd3(mode.PRF(seed, nonce, outlen));
      }

      static Polynomial from_bytes(std::span<const uint8_t> a) {
         Polynomial r;
         for(size_t i = 0; i < r.size() / 2; ++i) {
            r.m_coeffs[2 * i] = ((a[3 * i + 0] >> 0) | (static_cast<uint16_t>(a[3 * i + 1]) << 8)) & 0xFFF;
            r.m_coeffs[2 * i + 1] = ((a[3 * i + 1] >> 4) | (static_cast<uint16_t>(a[3 * i + 2]) << 4)) & 0xFFF;
         }
         return r;
      }

      static Polynomial from_message(std::span<const uint8_t> msg) {
         BOTAN_ASSERT(msg.size() == KyberConstants::N / 8, "message length must be Kyber_N/8 bytes");

         Polynomial r;
         for(size_t i = 0; i < r.size() / 8; ++i) {
            for(size_t j = 0; j < 8; ++j) {
               const auto mask = -static_cast<int16_t>((msg[i] >> j) & 1);
               r.m_coeffs[8 * i + j] = mask & ((KyberConstants::Q + 1) / 2);
            }
         }
         return r;
      }

      template <typename T = secure_vector<uint8_t>>
      T to_message() {
         T result(size() / 8);

         this->csubq();

         for(size_t i = 0; i < size() / 8; ++i) {
            result[i] = 0;
            for(size_t j = 0; j < 8; ++j) {
               const uint16_t t =
                  ct_int_div_kyber_q((static_cast<uint16_t>(this->m_coeffs[8 * i + j]) << 1) + KyberConstants::Q / 2);
               result[i] |= (t & 1) << j;
            }
         }

         return result;
      }

      /**
       * Adds two polynomials element-wise. Does not perform a reduction after the addition.
       * Therefore this operation might cause an integer overflow.
       */
      Polynomial& operator+=(const Polynomial& other) {
         for(size_t i = 0; i < this->size(); ++i) {
            BOTAN_DEBUG_ASSERT(static_cast<int32_t>(this->m_coeffs[i]) + other.m_coeffs[i] <=
                               std::numeric_limits<int16_t>::max());
            this->m_coeffs[i] = this->m_coeffs[i] + other.m_coeffs[i];
         }
         return *this;
      }

      /**
       * Subtracts two polynomials element-wise. Does not perform a reduction after the subtraction.
       * Therefore this operation might cause an integer underflow.
       */
      Polynomial& operator-=(const Polynomial& other) {
         for(size_t i = 0; i < this->size(); ++i) {
            BOTAN_DEBUG_ASSERT(static_cast<int32_t>(other.m_coeffs[i]) - this->m_coeffs[i] >=
                               std::numeric_limits<int16_t>::min());
            this->m_coeffs[i] = other.m_coeffs[i] - this->m_coeffs[i];
         }
         return *this;
      }

      /**
       * Multiplication of two polynomials in NTT domain
       */
      static Polynomial basemul_montgomery(const Polynomial& a, const Polynomial& b) {
         /**
          * Multiplication of polynomials in Zq[X]/(X^2-zeta) used for
          * multiplication of elements in Rq in NTT domain.
          */
         auto basemul = [](int16_t r[2], const int16_t s[2], const int16_t t[2], const int16_t zeta) {
            r[0] = fqmul(s[1], t[1]);
            r[0] = fqmul(r[0], zeta);
            r[0] += fqmul(s[0], t[0]);

            r[1] = fqmul(s[0], t[1]);
            r[1] += fqmul(s[1], t[0]);
         };

         Polynomial r;

         for(size_t i = 0; i < r.size() / 4; ++i) {
            basemul(&r.m_coeffs[4 * i], &a.m_coeffs[4 * i], &b.m_coeffs[4 * i], KyberConstants::zetas[64 + i]);
            basemul(
               &r.m_coeffs[4 * i + 2], &a.m_coeffs[4 * i + 2], &b.m_coeffs[4 * i + 2], -KyberConstants::zetas[64 + i]);
         }

         return r;
      }

      /**
       * Run rejection sampling on uniform random bytes to generate uniform
       * random integers mod q.
       */
      static Polynomial sample_rej_uniform(XOF& xof) {
         Polynomial p;

         size_t count = 0;
         while(count < p.size()) {
            std::array<uint8_t, 3> buf;
            xof.output(buf);

            const uint16_t val0 = ((buf[0] >> 0) | (static_cast<uint16_t>(buf[1]) << 8)) & 0xFFF;
            const uint16_t val1 = ((buf[1] >> 4) | (static_cast<uint16_t>(buf[2]) << 4)) & 0xFFF;

            if(val0 < KyberConstants::Q) {
               p.m_coeffs[count++] = val0;
            }
            if(count < p.size() && val1 < KyberConstants::Q) {
               p.m_coeffs[count++] = val1;
            }
         }

         return p;
      }

      /**
       * Inplace conversion of all coefficients of a polynomial from normal
       * domain to Montgomery domain.
       */
      void tomont() {
         constexpr int16_t f = (1ULL << 32) % KyberConstants::Q;
         for(auto& c : m_coeffs) {
            c = montgomery_reduce(static_cast<int32_t>(c) * f);
         }
      }

      /**
       * Computes negacyclic number-theoretic transform (NTT) of a polynomial in place;
       * inputs assumed to be in normal order, output in bitreversed order.
       */
      void ntt() {
         for(size_t len = size() / 2, k = 0; len >= 2; len /= 2) {
            for(size_t start = 0, j = 0; start < size(); start = j + len) {
               const auto zeta = KyberConstants::zetas[++k];
               for(j = start; j < start + len; ++j) {
                  const auto t = fqmul(zeta, m_coeffs[j + len]);
                  m_coeffs[j + len] = m_coeffs[j] - t;
                  m_coeffs[j] = m_coeffs[j] + t;
               }
            }
         }

         reduce();
      }

      /**
       * Computes inverse of negacyclic number-theoretic transform (NTT) of a polynomial
       * in place; inputs assumed to be in bitreversed order, output in normal order.
       */
      void invntt_tomont() {
         for(size_t len = 2, k = 0; len <= size() / 2; len *= 2) {
            for(size_t start = 0, j = 0; start < size(); start = j + len) {
               const auto zeta = KyberConstants::zetas_inv[k++];
               for(j = start; j < start + len; ++j) {
                  const auto t = m_coeffs[j];
                  m_coeffs[j] = barrett_reduce(t + m_coeffs[j + len]);
                  m_coeffs[j + len] = fqmul(zeta, t - m_coeffs[j + len]);
               }
            }
         }

         for(auto& c : m_coeffs) {
            c = fqmul(c, KyberConstants::zetas_inv[127]);
         }
      }

      size_t size() const { return m_coeffs.size(); }

      int16_t operator[](size_t idx) const { return m_coeffs[idx]; }

      int16_t& operator[](size_t idx) { return m_coeffs[idx]; }

   private:
      /**
       * Barrett reduction; given a 16-bit integer a, computes 16-bit integer congruent
       * to a mod q in {0,...,q}.
       */
      static int16_t barrett_reduce(int16_t a) {
         int16_t t;
         constexpr int16_t v = ((1U << 26) + KyberConstants::Q / 2) / KyberConstants::Q;

         t = static_cast<int32_t>(v) * a >> 26;
         t *= KyberConstants::Q;
         return a - t;
      }

      /**
       * Multiplication followed by Montgomery reduction.
       */
      static int16_t fqmul(int16_t a, int16_t b) { return montgomery_reduce(static_cast<int32_t>(a) * b); }

      /**
       * Montgomery reduction; given a 32-bit integer a, computes 16-bit integer
       * congruent to a * R^-1 mod q, where R=2^16
       */
      static int16_t montgomery_reduce(int32_t a) {
         const int16_t u = static_cast<int16_t>(a * KyberConstants::Q_Inv);
         int32_t t = static_cast<int32_t>(u) * KyberConstants::Q;
         t = a - t;
         t >>= 16;
         return static_cast<int16_t>(t);
      }

      std::array<int16_t, KyberConstants::N> m_coeffs;
};

class PolynomialVector {
   public:
      PolynomialVector() = delete;

      explicit PolynomialVector(const size_t k) : m_vec(k) {}

      static PolynomialVector from_bytes(std::span<const uint8_t> a, const KyberConstants& mode) {
         BOTAN_ASSERT(a.size() == mode.polynomial_vector_byte_length(), "wrong byte length for frombytes");

         PolynomialVector r(mode.k());
         for(size_t i = 0; i < mode.k(); ++i) {
            r.m_vec[i] = Polynomial::from_bytes(a.subspan(0, KyberConstants::kSerializedPolynomialByteLength));
            a = a.subspan(KyberConstants::kSerializedPolynomialByteLength);
         }
         return r;
      }

      /**
       * Pointwise multiply elements of a and b, accumulate into r, and multiply by 2^-16.
       */
      static Polynomial pointwise_acc_montgomery(const PolynomialVector& a, const PolynomialVector& b) {
         BOTAN_ASSERT(a.m_vec.size() == b.m_vec.size(),
                      "pointwise_acc_montgomery works on equally sized "
                      "PolynomialVectors only");

         auto r = Polynomial::basemul_montgomery(a.m_vec[0], b.m_vec[0]);
         for(size_t i = 1; i < a.m_vec.size(); ++i) {
            r += Polynomial::basemul_montgomery(a.m_vec[i], b.m_vec[i]);
         }

         r.reduce();
         return r;
      }

      static PolynomialVector getnoise_eta2(std::span<const uint8_t> seed, uint8_t nonce, const KyberConstants& mode) {
         PolynomialVector r(mode.k());

         for(auto& p : r.m_vec) {
            p = Polynomial::getnoise_eta2(seed, nonce++, mode);
         }

         return r;
      }

      static PolynomialVector getnoise_eta1(std::span<const uint8_t> seed, uint8_t nonce, const KyberConstants& mode) {
         PolynomialVector r(mode.k());

         for(auto& p : r.m_vec) {
            p = Polynomial::getnoise_eta1(seed, nonce++, mode);
         }

         return r;
      }

      template <typename T = std::vector<uint8_t>>
      T to_bytes() {
         T r;

         r.reserve(m_vec.size() * KyberConstants::kSerializedPolynomialByteLength);
         for(auto& v : m_vec) {
            const auto poly = v.to_bytes<T>();
            r.insert(r.end(), poly.begin(), poly.end());
         }

         return r;
      }

      /**
       * Applies conditional subtraction of q to each coefficient of each element
       * of the vector of polynomials.
       */
      void csubq() {
         for(auto& p : m_vec) {
            p.csubq();
         }
      }

      PolynomialVector& operator+=(const PolynomialVector& other) {
         BOTAN_ASSERT(m_vec.size() == other.m_vec.size(), "cannot add polynomial vectors of differing lengths");

         for(size_t i = 0; i < m_vec.size(); ++i) {
            m_vec[i] += other.m_vec[i];
         }
         return *this;
      }

      Polynomial& operator[](size_t idx) { return m_vec[idx]; }

      /**
       * Applies Barrett reduction to each coefficient of each element of a vector of polynomials.
       */
      void reduce() {
         for(auto& v : m_vec) {
            v.reduce();
         }
      }

      /**
       * Apply inverse NTT to all elements of a vector of polynomials and multiply by Montgomery factor 2^16.
       */
      void invntt_tomont() {
         for(auto& v : m_vec) {
            v.invntt_tomont();
         }
      }

      /**
       * Apply forward NTT to all elements of a vector of polynomials.
       */
      void ntt() {
         for(auto& v : m_vec) {
            v.ntt();
         }
      }

   private:
      std::vector<Polynomial> m_vec;
};

class PolynomialMatrix {
   public:
      PolynomialMatrix() = delete;

      static PolynomialMatrix generate(std::span<const uint8_t> seed,
                                       const bool transposed,
                                       const KyberConstants& mode) {
         BOTAN_ASSERT(seed.size() == KyberConstants::kSymBytes, "unexpected seed size");

         PolynomialMatrix matrix(mode);

         for(uint8_t i = 0; i < mode.k(); ++i) {
            for(uint8_t j = 0; j < mode.k(); ++j) {
               const auto pos = (transposed) ? std::tuple(i, j) : std::tuple(j, i);
               matrix.m_mat[i][j] = Polynomial::sample_rej_uniform(mode.XOF(seed, pos));
            }
         }

         return matrix;
      }

      PolynomialVector pointwise_acc_montgomery(const PolynomialVector& vec, const bool with_mont = false) const {
         PolynomialVector result(m_mat.size());

         for(size_t i = 0; i < m_mat.size(); ++i) {
            result[i] = PolynomialVector::pointwise_acc_montgomery(m_mat[i], vec);
            if(with_mont) {
               result[i].tomont();
            }
         }

         return result;
      }

   private:
      explicit PolynomialMatrix(const KyberConstants& mode) : m_mat(mode.k(), PolynomialVector(mode.k())) {}

   private:
      std::vector<PolynomialVector> m_mat;
};

class Ciphertext {
   public:
      Ciphertext() = delete;

      Ciphertext(PolynomialVector b, const Polynomial& v, KyberConstants mode) :
            m_mode(std::move(mode)), m_b(std::move(b)), m_v(v) {}

      static Ciphertext from_bytes(std::span<const uint8_t> buffer, const KyberConstants& mode) {
         const size_t pvb = polynomial_vector_compressed_bytes(mode);
         const size_t pcb = polynomial_compressed_bytes(mode);

         if(buffer.size() != pvb + pcb) {
            throw Decoding_Error("Kyber: unexpected ciphertext length");
         }

         auto pv = buffer.subspan(0, pvb);
         auto p = buffer.subspan(pvb);

         return Ciphertext(decompress_polynomial_vector(pv, mode), decompress_polynomial(p, mode), mode);
      }

      secure_vector<uint8_t> to_bytes() {
         auto ct = compress(m_b, m_mode);
         const auto p = compress(m_v, m_mode);
         ct.insert(ct.end(), p.begin(), p.end());

         return ct;
      }

      secure_vector<uint8_t> indcpa_decrypt(const PolynomialVector& polynomials) {
         m_b.ntt();
         auto mp = PolynomialVector::pointwise_acc_montgomery(polynomials, m_b);
         mp.invntt_tomont();

         mp -= m_v;
         mp.reduce();
         return mp.to_message();
      }

   private:
      static size_t polynomial_vector_compressed_bytes(const KyberConstants& mode) {
         const auto k = mode.k();
         return (k == 2 || k == 3) ? k * 320 : k * 352;
      }

      static size_t polynomial_compressed_bytes(const KyberConstants& mode) {
         const auto k = mode.k();
         return (k == 2 || k == 3) ? 128 : 160;
      }

      static secure_vector<uint8_t> compress(PolynomialVector& pv, const KyberConstants& mode) {
         secure_vector<uint8_t> r(polynomial_vector_compressed_bytes(mode));

         pv.csubq();

         if(mode.k() == 2 || mode.k() == 3) {
            uint16_t t[4];
            size_t offset = 0;
            for(size_t i = 0; i < mode.k(); ++i) {
               for(size_t j = 0; j < KyberConstants::N / 4; ++j) {
                  for(size_t k = 0; k < 4; ++k) {
                     t[k] = (((static_cast<uint32_t>(pv[i][4 * j + k]) << 10) + KyberConstants::Q / 2) /
                             KyberConstants::Q) &
                            0x3ff;
                  }

                  r[0 + offset] = static_cast<uint8_t>(t[0] >> 0);
                  r[1 + offset] = static_cast<uint8_t>((t[0] >> 8) | (t[1] << 2));
                  r[2 + offset] = static_cast<uint8_t>((t[1] >> 6) | (t[2] << 4));
                  r[3 + offset] = static_cast<uint8_t>((t[2] >> 4) | (t[3] << 6));
                  r[4 + offset] = static_cast<uint8_t>(t[3] >> 2);
                  offset += 5;
               }
            }
         } else {
            uint16_t t[8];
            size_t offset = 0;
            for(size_t i = 0; i < mode.k(); ++i) {
               for(size_t j = 0; j < KyberConstants::N / 8; ++j) {
                  for(size_t k = 0; k < 8; ++k) {
                     t[k] = (((static_cast<uint32_t>(pv[i][8 * j + k]) << 11) + KyberConstants::Q / 2) /
                             KyberConstants::Q) &
                            0x7ff;
                  }

                  r[0 + offset] = static_cast<uint8_t>(t[0] >> 0);
                  r[1 + offset] = static_cast<uint8_t>((t[0] >> 8) | (t[1] << 3));
                  r[2 + offset] = static_cast<uint8_t>((t[1] >> 5) | (t[2] << 6));
                  r[3 + offset] = static_cast<uint8_t>(t[2] >> 2);
                  r[4 + offset] = static_cast<uint8_t>((t[2] >> 10) | (t[3] << 1));
                  r[5 + offset] = static_cast<uint8_t>((t[3] >> 7) | (t[4] << 4));
                  r[6 + offset] = static_cast<uint8_t>((t[4] >> 4) | (t[5] << 7));
                  r[7 + offset] = static_cast<uint8_t>(t[5] >> 1);
                  r[8 + offset] = static_cast<uint8_t>((t[5] >> 9) | (t[6] << 2));
                  r[9 + offset] = static_cast<uint8_t>((t[6] >> 6) | (t[7] << 5));
                  r[10 + offset] = static_cast<uint8_t>(t[7] >> 3);
                  offset += 11;
               }
            }
         }

         return r;
      }

      static secure_vector<uint8_t> compress(Polynomial& p, const KyberConstants& mode) {
         secure_vector<uint8_t> r(polynomial_compressed_bytes(mode));

         p.csubq();

         uint8_t t[8];
         if(mode.k() == 2 || mode.k() == 3) {
            size_t offset = 0;
            for(size_t i = 0; i < p.size() / 8; ++i) {
               for(size_t j = 0; j < 8; ++j) {
                  t[j] = ct_int_div_kyber_q((static_cast<uint16_t>(p[8 * i + j]) << 4) + KyberConstants::Q / 2) & 15;
               }

               r[0 + offset] = t[0] | (t[1] << 4);
               r[1 + offset] = t[2] | (t[3] << 4);
               r[2 + offset] = t[4] | (t[5] << 4);
               r[3 + offset] = t[6] | (t[7] << 4);
               offset += 4;
            }
         } else if(mode.k() == 4) {
            size_t offset = 0;
            for(size_t i = 0; i < p.size() / 8; ++i) {
               for(size_t j = 0; j < 8; ++j) {
                  t[j] = ct_int_div_kyber_q((static_cast<uint32_t>(p[8 * i + j]) << 5) + KyberConstants::Q / 2) & 31;
               }

               r[0 + offset] = (t[0] >> 0) | (t[1] << 5);
               r[1 + offset] = (t[1] >> 3) | (t[2] << 2) | (t[3] << 7);
               r[2 + offset] = (t[3] >> 1) | (t[4] << 4);
               r[3 + offset] = (t[4] >> 4) | (t[5] << 1) | (t[6] << 6);
               r[4 + offset] = (t[6] >> 2) | (t[7] << 3);
               offset += 5;
            }
         }

         return r;
      }

      static PolynomialVector decompress_polynomial_vector(std::span<const uint8_t> buffer,
                                                           const KyberConstants& mode) {
         BOTAN_ASSERT(buffer.size() == polynomial_vector_compressed_bytes(mode),
                      "unexpected length of compressed polynomial vector");

         PolynomialVector r(mode.k());
         const uint8_t* a = buffer.data();

         if(mode.k() == 4) {
            uint16_t t[8];
            for(size_t i = 0; i < mode.k(); ++i) {
               for(size_t j = 0; j < KyberConstants::N / 8; ++j) {
                  t[0] = (a[0] >> 0) | (static_cast<uint16_t>(a[1]) << 8);
                  t[1] = (a[1] >> 3) | (static_cast<uint16_t>(a[2]) << 5);
                  t[2] = (a[2] >> 6) | (static_cast<uint16_t>(a[3]) << 2) | (static_cast<uint16_t>(a[4]) << 10);
                  t[3] = (a[4] >> 1) | (static_cast<uint16_t>(a[5]) << 7);
                  t[4] = (a[5] >> 4) | (static_cast<uint16_t>(a[6]) << 4);
                  t[5] = (a[6] >> 7) | (static_cast<uint16_t>(a[7]) << 1) | (static_cast<uint16_t>(a[8]) << 9);
                  t[6] = (a[8] >> 2) | (static_cast<uint16_t>(a[9]) << 6);
                  t[7] = (a[9] >> 5) | (static_cast<uint16_t>(a[10]) << 3);
                  a += 11;

                  for(size_t k = 0; k < 8; ++k) {
                     r[i][8 * j + k] = (static_cast<uint32_t>(t[k] & 0x7FF) * KyberConstants::Q + 1024) >> 11;
                  }
               }
            }
         } else {
            uint16_t t[4];
            for(size_t i = 0; i < mode.k(); ++i) {
               for(size_t j = 0; j < KyberConstants::N / 4; ++j) {
                  t[0] = (a[0] >> 0) | (static_cast<uint16_t>(a[1]) << 8);
                  t[1] = (a[1] >> 2) | (static_cast<uint16_t>(a[2]) << 6);
                  t[2] = (a[2] >> 4) | (static_cast<uint16_t>(a[3]) << 4);
                  t[3] = (a[3] >> 6) | (static_cast<uint16_t>(a[4]) << 2);
                  a += 5;

                  for(size_t k = 0; k < 4; ++k) {
                     r[i][4 * j + k] = (static_cast<uint32_t>(t[k] & 0x3FF) * KyberConstants::Q + 512) >> 10;
                  }
               }
            }
         }

         return r;
      }

      static Polynomial decompress_polynomial(std::span<const uint8_t> buffer, const KyberConstants& mode) {
         BOTAN_ASSERT(buffer.size() == polynomial_compressed_bytes(mode), "unexpected length of compressed polynomial");

         Polynomial r;
         const uint8_t* a = buffer.data();

         if(mode.k() == 4) {
            uint8_t t[8];
            for(size_t i = 0; i < KyberConstants::N / 8; ++i) {
               t[0] = (a[0] >> 0);
               t[1] = (a[0] >> 5) | (a[1] << 3);
               t[2] = (a[1] >> 2);
               t[3] = (a[1] >> 7) | (a[2] << 1);
               t[4] = (a[2] >> 4) | (a[3] << 4);
               t[5] = (a[3] >> 1);
               t[6] = (a[3] >> 6) | (a[4] << 2);
               t[7] = (a[4] >> 3);
               a += 5;

               for(size_t j = 0; j < 8; ++j) {
                  r[8 * i + j] = (static_cast<uint32_t>(t[j] & 31) * KyberConstants::Q + 16) >> 5;
               }
            }
         } else {
            for(size_t i = 0; i < KyberConstants::N / 2; ++i) {
               r[2 * i + 0] = ((static_cast<uint16_t>(a[0] & 15) * KyberConstants::Q) + 8) >> 4;
               r[2 * i + 1] = ((static_cast<uint16_t>(a[0] >> 4) * KyberConstants::Q) + 8) >> 4;
               a += 1;
            }
         }

         return r;
      }

   private:
      KyberConstants m_mode;
      PolynomialVector m_b;
      Polynomial m_v;
};

}  // anonymous namespace

class Kyber_PublicKeyInternal {
   public:
      Kyber_PublicKeyInternal(KyberConstants mode, std::span<const uint8_t> polynomials, std::vector<uint8_t> seed) :
            m_mode(std::move(mode)),
            m_polynomials(PolynomialVector::from_bytes(polynomials, m_mode)),
            m_seed(std::move(seed)),
            m_public_key_bits_raw(concat(m_polynomials.to_bytes<std::vector<uint8_t>>(), m_seed)),
            m_H_public_key_bits_raw(m_mode.H()->process<std::vector<uint8_t>>(m_public_key_bits_raw)) {}

      Kyber_PublicKeyInternal(KyberConstants mode, PolynomialVector polynomials, std::vector<uint8_t> seed) :
            m_mode(std::move(mode)),
            m_polynomials(std::move(polynomials)),
            m_seed(std::move(seed)),
            m_public_key_bits_raw(concat(m_polynomials.to_bytes<std::vector<uint8_t>>(), m_seed)),
            m_H_public_key_bits_raw(m_mode.H()->process<std::vector<uint8_t>>(m_public_key_bits_raw)) {}

      const PolynomialVector& polynomials() const { return m_polynomials; }

      const std::vector<uint8_t>& seed() const { return m_seed; }

      const KyberConstants& mode() const { return m_mode; }

      const std::vector<uint8_t>& public_key_bits_raw() const { return m_public_key_bits_raw; }

      const std::vector<uint8_t>& H_public_key_bits_raw() const { return m_H_public_key_bits_raw; }

      Kyber_PublicKeyInternal() = delete;

   private:
      const KyberConstants m_mode;
      PolynomialVector m_polynomials;
      const std::vector<uint8_t> m_seed;
      const std::vector<uint8_t> m_public_key_bits_raw;
      const std::vector<uint8_t> m_H_public_key_bits_raw;
};

class Kyber_PrivateKeyInternal {
   public:
      Kyber_PrivateKeyInternal(KyberConstants mode, PolynomialVector polynomials, secure_vector<uint8_t> z) :
            m_mode(std::move(mode)), m_polynomials(std::move(polynomials)), m_z(std::move(z)) {}

      PolynomialVector& polynomials() { return m_polynomials; }

      const secure_vector<uint8_t>& z() const { return m_z; }

      const KyberConstants& mode() const { return m_mode; }

      Kyber_PrivateKeyInternal() = delete;

   private:
      KyberConstants m_mode;
      PolynomialVector m_polynomials;
      secure_vector<uint8_t> m_z;
};

class Kyber_KEM_Cryptor {
   protected:
      Kyber_KEM_Cryptor(std::shared_ptr<const Kyber_PublicKeyInternal> public_key) :
            m_public_key(std::move(public_key)),
            m_mode(m_public_key->mode()),
            m_at(PolynomialMatrix::generate(m_public_key->seed(), true, m_mode)) {}

      secure_vector<uint8_t> indcpa_enc(std::span<const uint8_t> m, std::span<const uint8_t> coins) {
         auto sp = PolynomialVector::getnoise_eta1(coins, 0, m_mode);
         auto ep = PolynomialVector::getnoise_eta2(coins, m_mode.k(), m_mode);
         auto epp = Polynomial::getnoise_eta2(coins, 2 * m_mode.k(), m_mode);

         auto k = Polynomial::from_message(m);

         sp.ntt();

         // matrix-vector multiplication
         auto bp = m_at.pointwise_acc_montgomery(sp);
         auto v = PolynomialVector::pointwise_acc_montgomery(m_public_key->polynomials(), sp);

         bp.invntt_tomont();
         v.invntt_tomont();

         bp += ep;
         v += epp;
         v += k;
         bp.reduce();
         v.reduce();

         return Ciphertext(std::move(bp), v, m_mode).to_bytes();
      }

      const KyberConstants& mode() const { return m_mode; }

   private:
      std::shared_ptr<const Kyber_PublicKeyInternal> m_public_key;
      const KyberConstants& m_mode;
      const PolynomialMatrix m_at;
};

namespace {

size_t kyber_key_length_to_encap_key_length(size_t kl) {
   switch(kl) {
      case 800:
         return 768;
      case 1184:
         return 1088;
      case 1568:
         return 1568;
      default:
         throw Internal_Error("Unexpected Kyber key length");
   }
}

}  // namespace

class Kyber_KEM_Encryptor final : public PK_Ops::KEM_Encryption_with_KDF,
                                  protected Kyber_KEM_Cryptor {
   public:
      Kyber_KEM_Encryptor(const Kyber_PublicKey& key, std::string_view kdf) :
            KEM_Encryption_with_KDF(kdf), Kyber_KEM_Cryptor(key.m_public), m_key(key) {}

      size_t raw_kem_shared_key_length() const override { return 32; }

      size_t encapsulated_key_length() const override {
         return kyber_key_length_to_encap_key_length(m_key.key_length());
      }

      void raw_kem_encrypt(std::span<uint8_t> out_encapsulated_key,
                           std::span<uint8_t> out_shared_key,
                           RandomNumberGenerator& rng) override {
         // naming from kyber spec
         auto H = mode().H();
         auto G = mode().G();
         auto KDF = mode().KDF();

         H->update(rng.random_vec(KyberConstants::kSymBytes));
         const auto shared_secret = H->final();

         // Multitarget countermeasure for coins + contributory KEM
         G->update(shared_secret);
         G->update(m_key.H_public_key_bits_raw());
         const auto g_out = G->final();

         BOTAN_ASSERT_EQUAL(g_out.size(), 64, "Expected output length of Kyber G");

         const auto lower_g_out = std::span(g_out).subspan(0, 32);
         const auto upper_g_out = std::span(g_out).subspan(32, 32);

         const auto encapsulation = indcpa_enc(shared_secret, upper_g_out);

         // TODO: avoid copy by letting Ciphertext write straight into std::span<>
         BOTAN_ASSERT_NOMSG(encapsulation.size() == out_encapsulated_key.size());
         std::copy(encapsulation.begin(), encapsulation.end(), out_encapsulated_key.begin());

         KDF->update(lower_g_out.data(), lower_g_out.size());
         KDF->update(H->process(out_encapsulated_key));
         KDF->final(out_shared_key);
      }

   private:
      const Kyber_PublicKey& m_key;
};

class Kyber_KEM_Decryptor final : public PK_Ops::KEM_Decryption_with_KDF,
                                  protected Kyber_KEM_Cryptor {
   public:
      Kyber_KEM_Decryptor(const Kyber_PrivateKey& key, std::string_view kdf) :
            PK_Ops::KEM_Decryption_with_KDF(kdf), Kyber_KEM_Cryptor(key.m_public), m_key(key) {}

      size_t raw_kem_shared_key_length() const override { return 32; }

      size_t encapsulated_key_length() const override {
         return kyber_key_length_to_encap_key_length(m_key.key_length());
      }

      void raw_kem_decrypt(std::span<uint8_t> out_shared_key, std::span<const uint8_t> encapsulated_key) override {
         // naming from kyber spec
         auto H = mode().H();
         auto G = mode().G();
         auto KDF = mode().KDF();

         const auto shared_secret = indcpa_dec(encapsulated_key);

         // Multitarget countermeasure for coins + contributory KEM
         G->update(shared_secret);
         G->update(m_key.H_public_key_bits_raw());

         const auto g_out = G->final();

         BOTAN_ASSERT_EQUAL(g_out.size(), 64, "Expected output length of Kyber G");

         const auto lower_g_out = std::span(g_out).subspan(0, 32);
         const auto upper_g_out = std::span(g_out).subspan(32, 32);

         H->update(encapsulated_key);

         const auto cmp = indcpa_enc(shared_secret, upper_g_out);
         BOTAN_ASSERT(encapsulated_key.size() == cmp.size(), "output of indcpa_enc has unexpected length");

         // Overwrite pre-k with z on re-encryption failure (constant time)
         secure_vector<uint8_t> lower_g_out_final(lower_g_out.size());
         BOTAN_ASSERT_NOMSG(lower_g_out.size() == m_key.m_private->z().size());

         const auto reencrypt_success = CT::is_equal(encapsulated_key.data(), cmp.data(), encapsulated_key.size());
         CT::conditional_copy_mem(reencrypt_success,
                                  lower_g_out_final.data(),
                                  lower_g_out.data(),
                                  m_key.m_private->z().data(),
                                  lower_g_out_final.size());

         KDF->update(lower_g_out_final);
         KDF->update(H->final());
         KDF->final(out_shared_key);
      }

   private:
      secure_vector<uint8_t> indcpa_dec(std::span<const uint8_t> c) {
         auto ct = Ciphertext::from_bytes(c, mode());
         return ct.indcpa_decrypt(m_key.m_private->polynomials());
      }

   private:
      const Kyber_PrivateKey& m_key;
};

KyberMode Kyber_PublicKey::mode() const {
   return m_public->mode().mode();
}

AlgorithmIdentifier Kyber_PublicKey::algorithm_identifier() const {
   return AlgorithmIdentifier(mode().object_identifier(), AlgorithmIdentifier::USE_EMPTY_PARAM);
}

OID Kyber_PublicKey::object_identifier() const {
   return mode().object_identifier();
}

size_t Kyber_PublicKey::estimated_strength() const {
   return m_public->mode().estimated_strength();
}

std::shared_ptr<Kyber_PublicKeyInternal> Kyber_PublicKey::initialize_from_encoding(std::span<const uint8_t> pub_key,
                                                                                   KyberMode m) {
   KyberConstants mode(m);

   if(pub_key.size() != mode.public_key_byte_length()) {
      throw Invalid_Argument("kyber public key does not have the correct byte count");
   }

   BufferSlicer s(pub_key);

   auto poly_vec = s.take(mode.polynomial_vector_byte_length());
   auto seed = s.copy_as_vector(KyberConstants::kSeedLength);
   BOTAN_ASSERT_NOMSG(s.empty());

   return std::make_shared<Kyber_PublicKeyInternal>(std::move(mode), poly_vec, std::move(seed));
}

Kyber_PublicKey::Kyber_PublicKey(const AlgorithmIdentifier& alg_id, std::span<const uint8_t> key_bits) :
      Kyber_PublicKey(key_bits, KyberMode(alg_id.oid())) {}

Kyber_PublicKey::Kyber_PublicKey(std::span<const uint8_t> pub_key, KyberMode m) :
      m_public(initialize_from_encoding(pub_key, m)) {}

Kyber_PublicKey::Kyber_PublicKey(const Kyber_PublicKey& other) :
      m_public(std::make_shared<Kyber_PublicKeyInternal>(*other.m_public)) {}

std::vector<uint8_t> Kyber_PublicKey::public_key_bits() const {
   return public_key_bits_raw();
}

const std::vector<uint8_t>& Kyber_PublicKey::public_key_bits_raw() const {
   return m_public->public_key_bits_raw();
}

const std::vector<uint8_t>& Kyber_PublicKey::H_public_key_bits_raw() const {
   return m_public->H_public_key_bits_raw();
}

size_t Kyber_PublicKey::key_length() const {
   return m_public->mode().public_key_byte_length();
}

bool Kyber_PublicKey::check_key(RandomNumberGenerator&, bool) const {
   return true;  // ??
}

std::unique_ptr<Private_Key> Kyber_PublicKey::generate_another(RandomNumberGenerator& rng) const {
   return std::make_unique<Kyber_PrivateKey>(rng, mode());
}

Kyber_PrivateKey::Kyber_PrivateKey(RandomNumberGenerator& rng, KyberMode m) {
   KyberConstants mode(m);

   auto G = mode.G();
   auto seed = G->process(rng.random_vec(KyberConstants::kSymBytes));

   const auto middle = G->output_length() / 2;

   BufferSlicer s(seed);
   auto seed1 = s.copy_as_vector(middle);
   auto seed2 = s.take(middle);
   BOTAN_ASSERT_NOMSG(s.empty());

   auto a = PolynomialMatrix::generate(seed1, false, mode);
   auto skpv = PolynomialVector::getnoise_eta1(seed2, 0, mode);
   auto e = PolynomialVector::getnoise_eta1(seed2, mode.k(), mode);

   skpv.ntt();
   e.ntt();

   // matrix-vector multiplication
   auto pkpv = a.pointwise_acc_montgomery(skpv, true);
   pkpv += e;
   pkpv.reduce();

   m_public = std::make_shared<Kyber_PublicKeyInternal>(mode, std::move(pkpv), std::move(seed1));
   m_private = std::make_shared<Kyber_PrivateKeyInternal>(
      std::move(mode), std::move(skpv), rng.random_vec(KyberConstants::kZLength));
}

Kyber_PrivateKey::Kyber_PrivateKey(const AlgorithmIdentifier& alg_id, std::span<const uint8_t> key_bits) :
      Kyber_PrivateKey(key_bits, KyberMode(alg_id.oid())) {}

Kyber_PrivateKey::Kyber_PrivateKey(std::span<const uint8_t> sk, KyberMode m) {
   KyberConstants mode(m);

   if(mode.private_key_byte_length() != sk.size()) {
      throw Invalid_Argument("kyber private key does not have the correct byte count");
   }

   BufferSlicer s(sk);

   auto skpv = PolynomialVector::from_bytes(s.take(mode.polynomial_vector_byte_length()), mode);
   auto pub_key = s.take(mode.public_key_byte_length());
   s.skip(KyberConstants::kPublicKeyHashLength);
   auto z = s.copy_as_secure_vector(KyberConstants::kZLength);

   BOTAN_ASSERT_NOMSG(s.empty());

   m_public = initialize_from_encoding(pub_key, m);
   m_private = std::make_shared<Kyber_PrivateKeyInternal>(std::move(mode), std::move(skpv), std::move(z));

   BOTAN_ASSERT(m_private && m_public, "reading private key encoding");
}

std::unique_ptr<Public_Key> Kyber_PrivateKey::public_key() const {
   return std::make_unique<Kyber_PublicKey>(*this);
}

secure_vector<uint8_t> Kyber_PrivateKey::raw_private_key_bits() const {
   return this->private_key_bits();
}

secure_vector<uint8_t> Kyber_PrivateKey::private_key_bits() const {
   return concat(m_private->polynomials().to_bytes<secure_vector<uint8_t>>(),
                 public_key_bits_raw(),
                 H_public_key_bits_raw(),
                 m_private->z());
}

std::unique_ptr<PK_Ops::KEM_Encryption> Kyber_PublicKey::create_kem_encryption_op(std::string_view params,
                                                                                  std::string_view provider) const {
   if(provider.empty() || provider == "base") {
      return std::make_unique<Kyber_KEM_Encryptor>(*this, params);
   }
   throw Provider_Not_Found(algo_name(), provider);
}

std::unique_ptr<PK_Ops::KEM_Decryption> Kyber_PrivateKey::create_kem_decryption_op(RandomNumberGenerator& rng,
                                                                                   std::string_view params,
                                                                                   std::string_view provider) const {
   BOTAN_UNUSED(rng);
   if(provider.empty() || provider == "base") {
      return std::make_unique<Kyber_KEM_Decryptor>(*this, params);
   }
   throw Provider_Not_Found(algo_name(), provider);
}

}  // namespace Botan
