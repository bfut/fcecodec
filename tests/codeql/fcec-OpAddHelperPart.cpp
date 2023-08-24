// #include <cstdio>
  //#include <cstdint>
  //#include <cstdlib>
//#include <cstring>
//#include <filesystem>
//#include <fstream>
#include <iostream>
//#include <vector>

#define FCELIB_PREVIEW_MESH2
#include "../../src/fcelib/fcelib.h"
#include "../../src/fcelib/fcelib_fcetypes.h"  /* line can be omitted */
#include "../../src/fcelib/fcelib_types.h"  /* line can be omitted */

#undef FSTDMAXNUMLODS
#define FSTDMAXNUMLODS 1

#define NUM_OPERATION_REPEATS 1

//int main(int argc, char **argv)
int main(void)
{
  int retv = -1;
  fcelib::FcelibMesh mesh[FSTDMAXNUMLODS];

  for (int i = 0; i < FSTDMAXNUMLODS; ++i)
  {
    fcelib::FCELIB_InitMesh(&mesh[i]);
  }

  for (;;)
  {
    retv = 0;

    for (int i = 0; i < FSTDMAXNUMLODS && retv == 0; ++i)
    {
      for (int r = 0; r < NUM_OPERATION_REPEATS && retv == 0; ++r)
      {
#if 0
        if (fcelib::FCELIB_TYPES_AddParts(&mesh[i], 1) == -1)
        {
          std::cout << r << " cannot add helper part (i,i)=(" << i << "," << i << ")" << std::endl;
          retv = -1;
          break;
        }
#endif
#if 1
        if (fcelib::FCELIB_AddHelperPart(&mesh[i]) == -1)
        {
          std::cout << r << " cannot add helper part (i,i)=(" << i << "," << i << ")" << std::endl;
          retv = -1;
          break;
        }
#endif
      }

      //fcelib::FCELIB_PrintMeshInfo(mesh[i]);  // debug
    }

    break;
  }  // for (;;)  -  level 1

  for (int i = 0; i < FSTDMAXNUMLODS; ++i)
  {
    fcelib::FCELIB_FreeMesh(&mesh[i]);
  }


  if (retv == 0)
    std::cout << "successful" << std::endl;
  else
    std::cout << "failed" << std::endl;


  return retv;
}
