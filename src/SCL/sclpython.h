/*
  SCL - SCLpython.h
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

#ifndef SCLPYTHON_H_
#define SCLPYTHON_H_

#ifdef SCL_PY_PRINTF
#include <stdarg.h>

#include <Python.h>

/* Python doc: format should limit the total size of the formatted output string to 1000 bytes or less – after 1000 bytes, the output string is truncated. */
/* Python doc: If a problem occurs, or sys.stdout is unset, the formatted message is written to the real (C level) stdout. */
#ifdef printf
#undef printf
#endif
#define printf PySys_WriteStdout

#ifdef fprintf
#undef fprintf
#endif
#define SCL_PY_PRINTF_BUFSZ 1024
char scl_py_printf_buf[SCL_PY_PRINTF_BUFSZ] = {};
/*
  fprintf(stderr, ...) and fprintf(stream, ...)

  no support for fprintf(stdout, ...) -> use printf(...) instead
*/
int SCL_PY_fprintf(std::FILE *stream, const char *fmt, ...) {
  int n = -1;
  va_list ap;
  va_start(ap, fmt);

  if (stream != stderr)
  {
    n = vfprintf(stream, fmt, ap);
  }
  else if (stream == stderr)
  {
    n = PyOS_vsnprintf(scl_py_printf_buf, sizeof(scl_py_printf_buf), fmt, ap);
    scl_py_printf_buf[n] = '\0';
    /* Python doc: format should limit the total size of the formatted output string to 1000 bytes or less – after 1000 bytes, the output string is truncated. */
    /* Python doc: If a problem occurs, or sys.stdout is unset, the formatted message is written to the real (C level) stdout. */
    PySys_WriteStderr("%s", scl_py_printf_buf);
  }

  va_end(ap);
  return n;
}
#define fprintf SCL_PY_fprintf
#endif  /* SCL_PY_PRINTF */

#endif  /* SCLPYTHON_H_ */
