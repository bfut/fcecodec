/*
  fcelib_util.h
  fcecodec Copyright (C) 2021-2023 Benjamin Futasz <https://github.com/bfut>

  You may not redistribute this program without its source code.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef FCELIB_UTIL_H_
#define FCELIB_UTIL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
namespace fcelib {
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef INT_MAX
#define INT_MAX 2147483647
#endif
#ifndef INT_MIN
#define INT_MIN -2147483648
#endif

#ifndef __cplusplus
enum { kFceLibImplementedFce3Parts = 13 };
enum { kFceLibNumFce4HiBodyParts = 18 };
#else
static const int kFceLibImplementedFce3Parts = 13;
static const int kFceLibNumFce4HiBodyParts = 18;
#endif

/* Represent dummies */
static
const float kVertDiamond[6 * 3] = {
  1.0,  0.0,  0.0,
 -1.0,  0.0,  0.0,
  0.0,  1.0,  0.0,
  0.0, -1.0,  0.0,
  0.0,  0.0,  1.0,
  0.0,  0.0, -1.0,
};

static
const int kTrianglesDiamond[8 * 3] = {
  3, 6, 1,
  3, 2, 6,
  3, 5, 2,
  3, 1, 5,
  4, 1, 6,
  4, 6, 2,
  4, 2, 5,
  4, 5, 1
};

int FCELIB_MISC_CompareFloats(const void *a, const void *b)
{
  const float arg1 = *(const float*)a;
  const float arg2 = *(const float*)b;
  return (arg1 > arg2) - (arg1 < arg2);
}

int FCELIB_MISC_CompareInts(const void *a, const void *b)
{
  const int arg1 = *(const int*)a;
  const int arg2 = *(const int*)b;
  return (arg1 > arg2) - (arg1 < arg2);
}

int FCELIB_MISC_Min(const int a, const int b)
{
  if (a < b)
    return a;
  else
    return b;
}

float FCELIB_MISC_Abs(const float a)
{
  if (a < 0)
    return -a;
  else
    return a;
}

/* Returns -100 on failure. */
int FCELIB_MISC_ArrMax(const int *arr, const int arr_len)
{
  int retv = -100;
  for (;;)
  {
    int *sortedarr = (int *)malloc((size_t)arr_len * sizeof(*sortedarr));
    if (!sortedarr)
    {
      fprintf(stderr, "Warning: FCELIB_MISC_ArrMax: Cannot allocate memory, return default -100");
      break;
    }
    memcpy(sortedarr, arr, (size_t)arr_len * sizeof(*sortedarr));
    qsort(sortedarr, (size_t)arr_len, sizeof(*sortedarr), FCELIB_MISC_CompareInts);

    retv = sortedarr[arr_len - 1];
    free(sortedarr);
    break;
  }
  return retv;
}

int FCELIB_MISC_StrIsInArray(char *str, const char **arr)
{
  int retv = 0;
  int i;
  for (i = 0; i < kFceLibNumFce4HiBodyParts; ++i)
  {
    if (strncmp(str, arr[i], 64) == 0)
    {
      retv = 1;
      break;
    }
  }
  return retv;
}

#ifdef __cplusplus
}  /* extern "C" */
#endif

#ifdef __cplusplus
}  /* namespace fcelib */
#endif

#endif  /* FCELIB_UTIL_H_ */
