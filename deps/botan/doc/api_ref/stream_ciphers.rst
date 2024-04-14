Stream Ciphers
========================

In contrast to block ciphers, stream ciphers operate on a plaintext stream
instead of blocks. Thus encrypting data results in changing the internal state
of the cipher and encryption of plaintext with arbitrary length is possible in
one go (in byte amounts). All implemented stream ciphers derive from the base
class :cpp:class:`StreamCipher` (`botan/stream_cipher.h`).

.. warning::

   Using a stream cipher without an authentication code is extremely insecure,
   because an attacker can trivially modify messages. Prefer using an
   authenticated cipher mode such as GCM or SIV.

.. warning::

   Encrypting more than one message with the same key requires careful management
   of initialization vectors. Otherwise the keystream will be reused, which causes
   the security of the cipher to completely fail.

.. cpp:class:: StreamCipher

  .. cpp:function:: std::string name() const

     Returns a human-readable string of the name of this algorithm.

  .. cpp:function:: void clear()

     Clear the key.

  .. cpp:function:: std::unique_ptr<StreamCipher> new_object() const

     Return a newly allocated object of the same type as this one.
     The new object is unkeyed.

  .. cpp:function:: void set_key(const uint8_t* key, size_t length)

     Set the stream cipher key. If the length is not accepted, an
     ``Invalid_Key_Length`` exception is thrown.

  .. cpp:function:: bool valid_keylength(size_t length) const

     This function returns true if and only if *length* is a valid
     keylength for the algorithm.

  .. cpp:function:: size_t minimum_keylength() const

     Return the smallest key length (in bytes) that is acceptable for the
     algorithm.

  .. cpp:function:: size_t maximum_keylength() const

     Return the largest key length (in bytes) that is acceptable for the
     algorithm.

  .. cpp:function:: bool valid_iv_length(size_t iv_len) const

     This function returns true if and only if *length* is a valid IV length for
     the stream cipher. Some ciphers do not support IVs at all, and will return
     false for any value except zero.

  .. cpp:function:: size_t default_iv_length() const

     Returns some default IV size, normally the largest IV supported by the cipher.
     If this function returns zero, then IVs are not supported and any call to
     ``set_iv`` with a non-empty value will fail.

  .. cpp:function:: void set_iv(const uint8_t*, size_t len)

     Load IV into the stream cipher state. This should happen after the key is
     set and before any operation (encrypt/decrypt/seek) is called.

     If the cipher does not support IVs, then a call with ``len`` equal to zero
     will be accepted and any other length will cause a ``Invalid_IV_Length``
     exception.

  .. cpp:function:: void seek(uint64_t offset)

     Sets the state of the stream cipher and keystream according to the passed
     *offset*, exactly as if *offset* bytes had first been encrypted. The key
     and (if required) the IV have to be set before this can be called. Not all
     ciphers support seeking; such objects will throw ``Not_Implemented`` in
     this case.

  .. cpp:function:: void cipher(const uint8_t* in, uint8_t* out, size_t n)

     Processes *n* bytes plain/ciphertext from *in* and writes the result to *out*.

  .. cpp:function:: void cipher1(uint8_t* inout, size_t n)

     Processes *n* bytes plain/ciphertext in place. Acts like :cpp:func:`cipher`\ (inout, inout, n).

  .. cpp:function:: void encipher(std::vector<uint8_t> inout)
  .. cpp:function:: void encrypt(std::vector<uint8_t> inout)
  .. cpp:function:: void decrypt(std::vector<uint8_t> inout)

     Processes plain/ciphertext *inout* in place. Acts like :cpp:func:`cipher`\ (inout.data(), inout.data(), inout.size()).

.. _stream_ciphers_example:

Code Example
-----------------

The following code encrypts a provided plaintext using ChaCha20.

.. literalinclude:: /../src/examples/chacha.cpp
   :language: cpp

Available Stream Ciphers
----------------------------

Botan provides the following stream ciphers. If in doubt, pick ChaCha20 or CTR(AES-256).

CTR-BE
~~~~~~~

Counter mode converts a block cipher into a stream cipher. It offers
parallel execution and can seek within the output stream, both useful
properties.

CTR mode requires a nonce, which can be any length up to the block size of the
underlying cipher. If it is shorter than the block size, sufficient zero bytes
are appended.

It is possible to choose the width of the counter portion, which can improve
performance somewhat, but limits the maximum number of bytes that can safely be
encrypted. Different protocols have different conventions for the width of the
counter portion. This is done by specifying the width (which must be at least 4
bytes, allowing to encrypt 2\ :sup:`32` blocks of data) for example using
"CTR(AES-256,8)" will select a 64-bit (8 byte) counter.

(The ``-BE`` suffix refers to big-endian convention for the counter.
Little-endian counter mode is rarely used and not currently implemented.)

Algorithm specification name:
``CTR-BE(<BlockCipher>,<optional counter size>)`` (reported name) /
``CTR(<BlockCipher>,<optional counter size>)``

- Counter size (in bytes) defaults to the block size of the underlying cipher
- If the counter size is the same as the underlying cipher,
  the name will be reported as ``CTR-BE(<BlockCipher>)``.
- Examples: ``CTR-BE(AES-128)``, ``CTR-BE(AES-128,8)``

OFB
~~~~~

Another stream cipher based on a block cipher. Unlike CTR mode, it does not
allow parallel execution or seeking within the output stream. Prefer CTR.

Available if ``BOTAN_HAS_OFB`` is defined.

Algorithm specification name:
``OFB(<BlockCipher>)``, e.g. ``OFB(AES-256)``

ChaCha
~~~~~~~~

A very fast cipher, now widely deployed in TLS as part of the ChaCha20Poly1305
AEAD. Can be used with 8 (fast but dangerous), 12 (balance), or 20 rounds
(conservative). Even with 20 rounds, ChaCha is very fast. Use 20 rounds.

ChaCha supports an optional IV (which defaults to all zeros). It can be of
length 64, 96 or (since 2.8) 192 bits. Using ChaCha with a 192 bit nonce is also
known as XChaCha.

Available if ``BOTAN_HAS_CHACHA`` is defined.

Algorithm specification names:

- ``ChaCha20``, alias for ``ChaCha(20)``
- ``ChaCha(<optional rounds>)``

  - Optional rounds defaults to 20
  - Examples: ``ChaCha(20)``, ``ChaCha(12)``

Salsa20
~~~~~~~~~

An earlier iteration of the ChaCha design, this cipher is popular due to its use
in the libsodium library. Prefer ChaCha.

Salsa supports an optional IV (which defaults to all zeros). It can be of length
64 or 192 bits. Using Salsa with a 192 bit nonce is also known as XSalsa.

Available if ``BOTAN_HAS_SALSA20`` is defined.

Algorithm specification name: ``Salsa20``

SHAKE-128
~~~~~~~~~~~~

This is the SHAKE-128 XOF exposed as a stream cipher. It is slower
than ChaCha and somewhat obscure, and was primarily implemented to
support a particular post-quantum scheme which is no longer supported.

SHAKE does not support IVs, nor seeking within the cipher stream.

Available if ``BOTAN_HAS_SHAKE_CIPHER`` is defined.

.. warning::

  SHAKE support (as a stream cipher) is deprecated and will be removed
  in a future major release.

Algorithm specification names:

- ``SHAKE-128`` (reported name) / ``SHAKE-128-XOF``
- ``SHAKE-256`` (reported name) / ``SHAKE-256-XOF``

RC4
~~~~

An old and very widely deployed stream cipher notable for its simplicity. It
does not support IVs or seeking within the cipher stream. Compared to modern
algorithms like ChaCha20, it is also quite slow.

.. warning::

   RC4 is prone to numerous attacks. **Avoid in new code** and use only if
   required for compatibility with existing systems.

Available if ``BOTAN_HAS_RC4`` is defined.

Algorithm specification names:

- ``RC4`` (reported name) / ``ARC4``
- ``MARK-4``
- ``RC4(SKIP)`` (reported name) / ``ARC4(SKIP)``

  - ``RC4(0)`` is an alias for ``RC4``
  - ``RC4(256)`` is an alias for ``MARK-4``
  - Examples: ``RC4(3)``
