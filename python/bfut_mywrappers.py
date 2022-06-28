# Copyright (C) 2021 and later Benjamin Futasz <https://github.com/bfut>
#
# This software is provided 'as-is', without any express or implied
# warranty.  In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#     claim that you wrote the original software. If you use this software
#     in a product, an acknowledgment in the product documentation would be
#     appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#     misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.
"""
    bfut_mywrappers.py - wrapping i/o functions etc.
"""
import fcecodec
import numpy as np

def GetFceVersion(path):
    with open(path, "rb") as f:
        buf = f.read(0x2038)
        version = fcecodec.GetFceVersion(buf)
        assert version > 0
        return version

def PrintFceInfo(path):
    with open(path, "rb") as f:
        buf = f.read()
        fcecodec.PrintFceInfo(buf)
        assert fcecodec.ValidateFce(buf) == 1

def LoadFce(mesh, path):
    with open(path, "rb") as f:
        fce_buf = f.read()
    assert fcecodec.ValidateFce(fce_buf) == 1
    mesh.IoDecode(fce_buf)
    assert mesh.MValid() is True
    return mesh

def WriteFce(version, mesh, path, center_parts = 1):
    with open(path, "wb") as f:
        # print(version == "3", version == "4", version)
        if version == "3":
            buf = mesh.IoEncode_Fce3(center_parts)
        elif version == "4":
            buf = mesh.IoEncode_Fce4(center_parts)
        else:
            buf = mesh.IoEncode_Fce4M(center_parts)
        assert fcecodec.ValidateFce(buf) == 1
        f.write(buf)

def ExportObj(mesh, objpath, mtlpath, texname, print_damage, print_dummies,
              use_part_positions):
    mesh.IoExportObj(str(objpath), str(mtlpath), str(texname), print_damage,
                     print_dummies, use_part_positions)

def GetPartNames(mesh):
    part_names = np.empty(shape=(mesh.MNumParts, ), dtype="U64")
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

def GetPartGlobalOrderVidxs(mesh, pid):
    map_verts = mesh.MVertsGetMap_idx2order
    part_vidxs = mesh.PGetTriagsVidx(pid)
    for i in range(part_vidxs.shape[0]):
        # print(part_vidxs[i], map_verts[part_vidxs[i]])
        part_vidxs[i] = map_verts[part_vidxs[i]]
    return part_vidxs
