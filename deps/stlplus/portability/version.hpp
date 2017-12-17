#ifndef STLPLUS_VERSION
////////////////////////////////////////////////////////////////////////////////

//   Author:    Andy Rushton
//   Copyright: (c) Southampton University 1999-2004
//              (c) Andy Rushton           2004 onwards
//   License:   BSD License, see ../docs/license.html

//   Contains just the STLplus version number

////////////////////////////////////////////////////////////////////////////////
#include "portability_fixes.hpp"
#include <string>

#define STLPLUS_VERSION "3.15"

namespace stlplus
{

  // preferred method for accessing the version number, returned as a string in the form "Major.Minor"
  std::string version(void);

}
////////////////////////////////////////////////////////////////////////////////
#endif
