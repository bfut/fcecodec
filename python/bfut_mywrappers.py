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

def ReorderTriagsTransparentToLast(mesh, pid_opaq):
    """ Copy original part, delete semi-transparent triags in original,
        delete opaque triags in copy, merge both, delete original & temporary,
        move merged part to original index, clean-up """
    print(mesh.PGetName(pid_opaq))
    pid_transp = mesh.OpCopyPart(pid_opaq)

    # delete semi-transparent triags in original
    flags = mesh.PGetTriagsFlags(pid_opaq)
    triags_idxs = []
    for i in range(flags.shape[0]):
        if flags[i] & 0x8 == 0x8:
            triags_idxs += [i]
    print(f"{triags_idxs}")
    mesh.OpDeletePartTriags(pid_opaq, triags_idxs)

    # delete opaque triags in copy
    flags = mesh.PGetTriagsFlags(pid_transp)
    triags_idxs = []
    for i in range(flags.shape[0]):
        if flags[i] & 0x8 < 0x8:
            triags_idxs += [i]
    print(f"{triags_idxs}")
    mesh.OpDeletePartTriags(pid_transp, triags_idxs)

    # merge both & rename
    new_pid = mesh.OpMergeParts(pid_opaq, pid_transp)
    mesh.PSetName(new_pid, mesh.PGetName(pid_opaq))

    # delete original & temporary
    mesh.OpDeletePart(pid_transp)
    mesh.OpDeletePart(pid_opaq)
    new_pid -= 2

    # move merged part to original index
    while new_pid > pid_opaq:
        new_pid = mesh.OpMovePart(new_pid)
        print("new_pid", new_pid)

    # clean-up
    mesh.OpDelUnrefdVerts()
    return mesh

def HiBody_ReorderTriagsTransparentToLast(mesh, version):
    """ Not implemented for FCE4M because windows are separate parts """
    if version == "3":
        mesh = ReorderTriagsTransparentToLast(mesh, 0)  # high body
        if mesh.MNumParts >= 12:
            mesh = ReorderTriagsTransparentToLast(mesh, 12)  # high headlights
    elif version == "4":
        for pn in (":HB", ":OT", ":OL"):
            pid = GetPartIdxFromName(mesh, pn)
            if pid >= 0:
                mesh = ReorderTriagsTransparentToLast(mesh, pid)
    return mesh

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

def WriteFce(version, mesh, path, center_parts = 1, transparent_triags_to_last = 0):
    if transparent_triags_to_last:
        mesh = HiBody_ReorderTriagsTransparentToLast(mesh, version)
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
              use_part_positions, print_part_positions):
    mesh.IoExportObj(str(objpath), str(mtlpath), str(texname), print_damage,
                     print_dummies, use_part_positions, print_part_positions)

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
