"""
  fcecodec_mywrappers.py - python wrappers for fcecodec i/o functions

  NOTE: module should be built with setup.py build|install

  This file is distributed under: CC BY-SA 4.0
      <https://creativecommons.org/licenses/by-sa/4.0/>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  This header may not be removed or altered from any source distribution.
"""

import fcecodec

def PrintFceInfo(path):
    with open(path, "rb") as f:
#        print("PrintFceInfo(", path, ")")
        buf = f.read()
        fcecodec.PrintFceInfo(buf)
        assert(fcecodec.ValidateFce( buf ) == 1)

def LoadFce(mesh, path):
#    print("LoadFce(", mesh, ",", path, ")")
    with open(path, "rb") as f:
        fce_buf = f.read()
    assert(fcecodec.ValidateFce(fce_buf) == 1)
    mesh.Decode(fce_buf)
    assert(mesh.Valid() == True)
    return mesh

def WriteFce(version, mesh, path):
    with open(path, "wb") as f:
        if version == 3:
            buf = mesh.Encode_Fce3()
        elif version == 4:
            buf = mesh.Encode_Fce4()
        else:
            buf = mesh.Encode_Fce4M()
        assert(fcecodec.ValidateFce( buf ) == 1)
        f.write(buf)

def ExportObj(mesh, objpath, mtlpath, texname, print_damage, print_dummies):
#    print("ExportObj(", mesh, objpath, mtlpath, texname, print_damage, print_dummies, ")")
    mesh.ExportObj(str(objpath), str(mtlpath), texname, print_damage, print_dummies)