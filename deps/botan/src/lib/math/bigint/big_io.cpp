/*
* BigInt Input/Output
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <botan/bigint.h>
#include <istream>
#include <ostream>

namespace Botan {

/*
* Write the BigInt into a stream
*/
std::ostream& operator<<(std::ostream& stream, const BigInt& n) {
   const auto stream_flags = stream.flags();
   if(stream_flags & std::ios::oct) {
      throw Invalid_Argument("Octal output of BigInt not supported");
   }

   const size_t base = (stream_flags & std::ios::hex) ? 16 : 10;

   std::string enc;

   if(base == 10) {
      enc = n.to_dec_string();
   } else {
      enc = n.to_hex_string();
   }

   stream.write(enc.data(), enc.size());

   if(!stream.good()) {
      throw Stream_IO_Error("BigInt output operator has failed");
   }
   return stream;
}

/*
* Read the BigInt from a stream
*/
std::istream& operator>>(std::istream& stream, BigInt& n) {
   std::string str;
   std::getline(stream, str);
   if(stream.bad() || (stream.fail() && !stream.eof())) {
      throw Stream_IO_Error("BigInt input operator has failed");
   }
   n = BigInt(str);
   return stream;
}

}  // namespace Botan
