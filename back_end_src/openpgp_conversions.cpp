//**********************************************************************************
//EncryptPad Copyright 2016 Evgeny Pokhilko 
//<http://www.evpo.net/encryptpad>
//
//This file is part of EncryptPad
//
//EncryptPad is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 2 of the License, or
//(at your option) any later version.
//
//EncryptPad is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with EncryptPad.  If not, see <http://www.gnu.org/licenses/>.
//**********************************************************************************
#include "openpgp_conversions.h"
#include "assert.h"

/* This is an extract from GnuPG to pack s2k iterations into one byte */

#define S2K_DECODE_COUNT(_val) ((16ul + ((_val) & 15)) << (((_val) >> 4) + 6))

unsigned int DecodeS2KIterations(unsigned char count)
{
    return S2K_DECODE_COUNT(count);
}

/* Pack an s2k iteration count into the form specified in 2440.  If
   we're in between valid values, round up.  With value 0 return the
   old default.  */
unsigned char
EncodeS2KIterations (unsigned int iterations)
{
  unsigned char c=0;
  unsigned char result;
  unsigned int count;

  /*assert(iterations > 1024)*/

  if (iterations >= 65011712)
    return 255;

  /* Need count to be in the range 16-31 */
  for (count=iterations>>6; count>=32; count>>=1)
    c++;

  result = (c<<4)|(count-16);

  if (S2K_DECODE_COUNT(result) < iterations)
    result++;

  return result;
}

