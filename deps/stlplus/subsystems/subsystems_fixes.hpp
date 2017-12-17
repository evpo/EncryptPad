#ifndef STLPLUS_SUBSYSTEMS_FIXES
#define STLPLUS_SUBSYSTEMS_FIXES
////////////////////////////////////////////////////////////////////////////////

//   Author:    Andy Rushton
//   Copyright: (c) Southampton University 1999-2004
//              (c) Andy Rushton           2004 onwards
//   License:   BSD License, see ../docs/license.html

//   Contains work arounds for OS or Compiler specific problems

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Unnecessary compiler warnings
////////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
// Microsoft Visual Studio
// shut up the following irritating warnings
//   4290 - C++ exception specification ignored
//   4996 - 'xxxx' was declared deprecated
#pragma warning(disable: 4290 4996)
#endif

#ifdef __BORLANDC__
// Borland
// Shut up the following irritating warnings
//   8026 - Functions with exception specifications are not expanded inline
//   8027 - Functions with xxx are not expanded inline
#pragma warn -8026
#pragma warn -8027
#endif

////////////////////////////////////////////////////////////////////////////////
#endif
