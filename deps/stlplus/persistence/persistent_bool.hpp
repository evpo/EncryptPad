#ifndef STLPLUS_PERSISTENT_BOOL
#define STLPLUS_PERSISTENT_BOOL
////////////////////////////////////////////////////////////////////////////////

//   Author:    Andy Rushton
//   Copyright: (c) Southampton University 1999-2004
//              (c) Andy Rushton           2004 onwards
//   License:   BSD License, see ../docs/license.html

//   Persistence of bool

////////////////////////////////////////////////////////////////////////////////

#include "persistence_fixes.hpp"
#include "persistent_contexts.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace stlplus
{

  // exceptions: persistent_dump_failed
  void dump_bool(dump_context&, const bool& data) ;
  // exceptions: persistent_restore_failed
  void restore_bool(restore_context&, bool& data) ;

} // end namespace stlplus

#endif
