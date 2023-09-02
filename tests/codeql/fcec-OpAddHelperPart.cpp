/*
  fcec-OpAddHelperPart.cpp - print FCE stats
  fcecodec Copyright (C) 2021, 2023 Benjamin Futasz <https://github.com/bfut>

  You may not redistribute this program without its source code.

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

// #include <cstdio>
  // #include <cstdint>
  // #include <cstdlib>
// #include <cstring>
// #include <filesystem>
// #include <fstream>
#include <iostream>
// #include <vector>

#include "../../src/fcelib/fcelib.h"
#include "../../src/fcelib/fcelib_fcetypes.h"  /* line can be omitted */
#include "../../src/fcelib/fcelib_types.h"  /* line can be omitted */

#undef FSTDMAXNUMLODS
#define FSTDMAXNUMLODS 1

#define NUM_OPERATION_REPEATS 1

// int main(int argc, char **argv)
int main(void)
{
  int retv = -1;
  FcelibMesh mesh[FSTDMAXNUMLODS];

  for (int i = 0; i < FSTDMAXNUMLODS; ++i)
  {
    FCELIB_InitMesh(&mesh[i]);
  }

  for (;;)
  {
    retv = 0;

    for (int i = 0; i < FSTDMAXNUMLODS && retv == 0; ++i)
    {
      for (int r = 0; r < NUM_OPERATION_REPEATS && retv == 0; ++r)
      {
        if (FCELIB_AddHelperPart(&mesh[i]) == -1)
        {
          std::cout << r << " cannot add helper part (i,i)=(" << i << "," << i << ")" << std::endl;
          retv = -1;
          break;
        }
      }

      // FCELIB_PrintMeshInfo(mesh[i]);  // debug
    }

    break;
  }  // for (;;)  -  level 1

  for (int i = 0; i < FSTDMAXNUMLODS; ++i)
  {
    mesh[i].release(&mesh[i]);
  }

  if (retv == 0)
    std::cout << "successful" << std::endl;
  else
    std::cout << "failed" << std::endl;

  return retv;
}
