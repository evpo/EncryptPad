#ifndef STLPLUS_PERSISTENT_ENUM
#define STLPLUS_PERSISTENT_ENUM
////////////////////////////////////////////////////////////////////////////////

//   Author:    Andy Rushton
//   Copyright: (c) Southampton University 1999-2004
//              (c) Andy Rushton           2004 onwards
//   License:   BSD License, see ../docs/license.html

//   Persistence of enumeration types

////////////////////////////////////////////////////////////////////////////////
#include "persistence_fixes.hpp"
#include "persistent_contexts.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace stlplus
{

  // exceptions: persistent_dump_failed
  template<typename T>
  void dump_enum(dump_context&, const T& data) ;

  // exceptions: persistent_restore_failed
  template<typename T>
  void restore_enum(restore_context&, T& data) ;

} // end namespace stlplus

  ////////////////////////////////////////////////////////////////////////////////
#include "persistent_enum.tpp"
#endif
