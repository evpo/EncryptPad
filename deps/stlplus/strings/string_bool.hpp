#ifndef STLPLUS_STRING_BOOL
#define STLPLUS_STRING_BOOL
////////////////////////////////////////////////////////////////////////////////

//   Author:    Andy Rushton
//   Copyright: (c) Southampton University 1999-2004
//              (c) Andy Rushton           2004 onwards
//   License:   BSD License, see ../docs/license.html

//   Conversion of string to/from bool

////////////////////////////////////////////////////////////////////////////////
#include "strings_fixes.hpp"
#include "format_types.hpp"
#include <string>
#include <stdexcept>

namespace stlplus
{

  ////////////////////////////////////////////////////////////////////////////////

  // exceptions: std::invalid_argument
  std::string bool_to_string(bool i,
                             unsigned radix = 10,
                             radix_display_t display = radix_c_style_or_hash,
                             unsigned width = 0);

  ////////////////////////////////////////////////////////////////////////////////

  // exceptions: std::invalid_argument
  bool string_to_bool(const std::string& value,
                      unsigned radix = 0);

  ////////////////////////////////////////////////////////////////////////////////

} // end namespace stlplus

#endif
