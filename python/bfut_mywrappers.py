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
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.
"""
    bfut_mywrappers.py - wrapping i/o functions etc.
"""
import fcecodec
import numpy as np

def ReorderTriagsTransparentDetachedAndToLast(mesh, pid_opaq):
    """ Copy original part, delete semi-transparent triags in original,
        delete opaque triags in copy, clean-up unreferenced verts, merge parts,
        delete temporary & original, move merged part to original index """
    pid_transp = mesh.OpCopyPart(pid_opaq)
    mesh.OpDeletePartTriags(pid_opaq, np.where(mesh.PGetTriagsFlags(pid_opaq) & 0x8 == 0x8)[0])
    mesh.OpDeletePartTriags(pid_transp, np.where(mesh.PGetTriagsFlags(pid_transp) & 0x8 < 0x8)[0])
    mesh.OpDelUnrefdVerts()
    new_pid = mesh.OpMergeParts(pid_opaq, pid_transp)  # last part idx
    mesh.PSetName(new_pid, mesh.PGetName(pid_opaq))
    mesh.OpDeletePart(pid_transp)
    mesh.OpDeletePart(pid_opaq)
    new_pid -= 2  # last part idx is now smaller
    while new_pid > pid_opaq:
        new_pid = mesh.OpMovePart(new_pid)  # move merged part to original index
    return mesh

def HiBody_ReorderTriagsTransparentToLast(mesh, version):
    """ Not implemented for FCE4M because windows are separate parts """
    if version in ("3", 3):
        mesh = ReorderTriagsTransparentDetachedAndToLast(mesh, 0)  # high body
        if mesh.MNumParts >= 12:
            mesh = ReorderTriagsTransparentDetachedAndToLast(mesh, 12)  # high headlights
    elif version in ("4", 4):
        for partname in (":HB", ":OT", ":OL"):
            pid = GetMeshPartnameIdx(mesh, partname)
            if pid >= 0:
                mesh = ReorderTriagsTransparentDetachedAndToLast(mesh, pid)
    return mesh

def GetFceVersion(path):
    with open(path, "rb") as f:
        version = fcecodec.GetFceVersion(f.read(0x2038))
        assert version > 0
        return version

def PrintFceInfo(path):
    with open(path, "rb") as f:
        buf = f.read()
        fcecodec.PrintFceInfo(buf)
        assert fcecodec.ValidateFce(buf) == 1

def LoadFce(mesh, path):
    with open(path, "rb") as f:
        mesh.IoDecode(f.read())
        assert mesh.MValid() is True
        return mesh

def WriteFce(version, mesh, path, center_parts=False, mesh_function=None):
    if mesh_function is not None:  # e.g., HiBody_ReorderTriagsTransparentToLast
        mesh = mesh_function(mesh, version)
    with open(path, "wb") as f:
        if version in ("3", 3):
            buf = mesh.IoEncode_Fce3(center_parts)
        elif version in ("4", 4):
            buf = mesh.IoEncode_Fce4(center_parts)
        else:
            buf = mesh.IoEncode_Fce4M(center_parts)
        assert fcecodec.ValidateFce(buf) == 1
        f.write(buf)

def ExportObj(mesh, objpath, mtlpath, texname, print_damage, print_dummies,
              use_part_positions, print_part_positions):
    mesh.IoExportObj(str(objpath), str(mtlpath), str(texname), print_damage,
                     print_dummies, use_part_positions, print_part_positions)

def GetMeshPartnames(mesh):
    return [mesh.PGetName(pid) for pid in range(mesh.MNumParts)]

def GetMeshPartnameIdx(mesh, partname):
    for pid in range(mesh.MNumParts):
        if mesh.PGetName(pid) == partname:
            return pid
    print(f"GetMeshPartnameIdx: Warning: cannot find \"{partname}\"")
    return -1

def GetPartGlobalOrderVidxs(mesh, pid):
    map_verts = mesh.MVertsGetMap_idx2order
    part_vidxs = mesh.PGetTriagsVidx(pid)
    for i in range(part_vidxs.shape[0]):
        # print(part_vidxs[i], map_verts[part_vidxs[i]])
        part_vidxs[i] = map_verts[part_vidxs[i]]
    return part_vidxs
