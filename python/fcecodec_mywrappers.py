"""
  fcecodec_mywrappers.py - wrapping i/o functions etc.

  This file is distributed under: CC BY-SA 4.0
      <https://creativecommons.org/licenses/by-sa/4.0/>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  This header may not be removed or altered from any source distribution.
"""

import fcecodec
import numpy as np

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
    mesh.IoDecode(fce_buf)
    assert(mesh.MValid() == True)
    return mesh

def WriteFce(version, mesh, path, center_parts = 1):
    with open(path, "wb") as f:
        if version == 3:
            buf = mesh.IoEncode_Fce3(center_parts)
        elif version == 4:
            buf = mesh.IoEncode_Fce4()
        else:
            buf = mesh.IoEncode_Fce4M()
        assert(fcecodec.ValidateFce(buf) == 1)
        f.write(buf)

def ExportObj(mesh, objpath, mtlpath, texname, print_damage, print_dummies):
#    print("IoExportObj(", mesh, objpath, mtlpath, texname, print_damage, print_dummies, ")")
    mesh.IoExportObj(str(objpath), str(mtlpath), texname, print_damage, print_dummies)

def GetPartNames(mesh):
    part_names = np.empty(shape=(mesh.MNumParts, ), dtype='U64')
    for i in range(mesh.MNumParts):
        part_names[i] = mesh.PGetName(i)
        i += 1
    return part_names

def GetPartIdxFromName(mesh, p_name):
    retv = -1
    pid = -1
    for name in GetPartNames(mesh):
        pid += 1
        if p_name == name:
            retv = pid
            break
    if retv < 0: print("GetPartIdxFromName: Warning: cannot find p_name")
    return retv