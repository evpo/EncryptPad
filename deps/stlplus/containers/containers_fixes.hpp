#ifndef STLPLUS_CONTAINERS_FIXES
#define STLPLUS_CONTAINERS_FIXES
////////////////////////////////////////////////////////////////////////////////

// Author:    Andy Rushton
// Copyright: (c) Southampton University 1999-2004
//            (c) Andy Rushton           2004 onwards
// License:   BSD License, see ../docs/license.html

// Contains work arounds for OS or Compiler specific problems with container
// templates

// These have virtually disappeared as C++ is now a very stable language

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Unnecessary compiler warnings
////////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
// Microsoft Visual Studio
// shut up the following irritating warnings
//   4290 - C++ exception specification ignored
//   4355 - 'this' used in base member initialisation list
//   4996 - 'xxxx' was declared deprecated
#pragma warning(disable: 4290 4355 4996)
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
