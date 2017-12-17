////////////////////////////////////////////////////////////////////////////////

//   Author:    Andy Rushton
//   Copyright: (c) Southampton University 1999-2004
//              (c) Andy Rushton           2004 onwards
//   License:   BSD License, see ../docs/license.html
//
//  Originally this used the stlplus::dprintf function but rewritten to use
//  iostreams to break the dependency between the libraries and to be more C++ey
//
////////////////////////////////////////////////////////////////////////////////
#include "string_float.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <cstdlib>

namespace stlplus
{

  ////////////////////////////////////////////////////////////////////////////////
  // floating-point types

  std::string float_to_string(float f, real_display_t display, unsigned width, unsigned precision)
  {
    return double_to_string((double)f, display, width, precision);
  }

  std::string double_to_string(double f, real_display_t display, unsigned width, unsigned precision)
  {
    std::ostringstream stream;
    switch(display)
    {
    case display_mixed:
      // default for iostream
      break;
    case display_fixed:
      stream << std::fixed;
      break;
    case display_floating:
      stream << std::scientific;
      break;
    default:
      throw std::invalid_argument("stlplus::double_to_string: invalid radix display value");
    }
    if (width != 0)
      stream << std::setw(width);
    stream << std::setprecision(precision);
    if (!(stream << f))
      throw std::invalid_argument("stlplus::double_to_string: iostream error");
    return std::string(stream.str());
  }

  ////////////////////////////////////////////////////////////////////////////////

  float string_to_float(const std::string& value)
  {
    return (float)string_to_double(value);
  }

  double string_to_double(const std::string& value)
  {
    // TODO - error checking
    return std::atof(value.c_str());
  }

  ////////////////////////////////////////////////////////////////////////////////

}
