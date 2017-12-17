////////////////////////////////////////////////////////////////////////////////

//   Author:    Andy Rushton
//   Copyright: (c) Southampton University 1999-2004
//              (c) Andy Rushton           2004 onwards
//   License:   BSD License, see ../docs/license.html

////////////////////////////////////////////////////////////////////////////////
#include "persistent_string.hpp"

////////////////////////////////////////////////////////////////////////////////

void stlplus::dump_string(stlplus::dump_context& context, const std::string& data)

{
  stlplus::dump_basic_string(context, data, stlplus::dump_char);
}

void stlplus::restore_string(stlplus::restore_context& context, std::string& data)

{
  stlplus::restore_basic_string(context, data, stlplus::restore_char);
}

////////////////////////////////////////////////////////////////////////////////
