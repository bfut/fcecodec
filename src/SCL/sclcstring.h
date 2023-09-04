/*
  SCL - SCLprint.h
  Copyright (C) 2022 and later Benjamin Futasz <https://github.com/bfut>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#ifndef SCLCSTRING_H_
#define SCLCSTRING_H_

#if !defined(__cplusplus) || (defined(__cplusplus) && (__cplusplus >= 201103L))
#include <stdarg.h>

#ifdef sprintf
#undef sprintf
#endif
int SCL_sprintf(char *buf, const char *fmt, ...)
{
  int n = -1;
  va_list ap;
  va_start(ap, fmt);
#if !defined(__cplusplus) || defined(_MSC_VER)
  n = vsnprintf_s(buf, sizeof(SCL_SPRINTF_BUFSZ), sizeof(SCL_SPRINTF_BUFSZ)-1, fmt, ap);
#else
  n = vsnprintf(buf, sizeof(SCL_SPRINTF_BUFSZ), fmt, ap);
#endif
  va_end(ap);
  return n;
}
#define sprintf SCL_sprintf
#endif

#if !defined(__cplusplus) || defined(_MSC_VER)
#ifdef strncpy
#undef strncpy
#endif
char *SCL_strncpy(char *dest, const char *src, size_t count)
{
  strncpy_s(dest, sizeof(SCL_STRNCPY_BUFSZ), src, count);
  return dest;
}
#define strncpy SCL_strncpy
#endif

#endif  /* SCLCSTRING_H_ */
