# Copyright (C) 2023 and later Benjamin Futasz <https://github.com/bfut>
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
    bfut_SetHighChromeToTransparent.py - description

USAGE
    python bfut_SetHighChromeToTransparent.py /path/to/model.fce [/path/to/output.fce]

REQUIRES
    installing <https://github.com/bfut/fcecodec>
"""
import argparse
import pathlib

import fcecodec as fc
import numpy as np

CONFIG = {
    "fce_version"  : "keep",  # output format version; expects "keep" or "3"|"4"|"4M" for FCE3, FCE4, FCE4M, respectively
}

# Parse command-line
parser = argparse.ArgumentParser()
parser.add_argument("path", nargs="+", help="file path")
args = parser.parse_args()

# Handle paths: mandatory inpath, optional outpath
filepath_fce_input = pathlib.Path(args.path[0])
if len(args.path) < 2:
    filepath_fce_output = filepath_fce_input.parent / (filepath_fce_input.stem + "_out" + filepath_fce_input.suffix)
else:
    filepath_fce_output = pathlib.Path(args.path[1])


# -------------------------------------- wrappers
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
    if version == "3":
        mesh = ReorderTriagsTransparentDetachedAndToLast(mesh, 0)  # high body
        if mesh.MNumParts >= 12:
            mesh = ReorderTriagsTransparentDetachedAndToLast(mesh, 12)  # high headlights
    elif version == "4":
        for partname in (":HB", ":OT", ":OL"):
            pid = GetMeshPartnameIdx(mesh, partname)
            if pid >= 0:
                mesh = ReorderTriagsTransparentDetachedAndToLast(mesh, pid)
    return mesh

def GetFceVersion(path):
    with open(path, "rb") as f:
        version = fc.GetFceVersion(f.read(0x2038))
        assert version > 0
        return version

def PrintFceInfo(path):
    with open(path, "rb") as f:
        buf = f.read()
        fc.PrintFceInfo(buf)
        assert fc.ValidateFce(buf) == 1

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
        assert fc.ValidateFce(buf) == 1
        f.write(buf)

def GetMeshPartnameIdx(mesh, partname):
    for pid in range(mesh.MNumParts):
        if mesh.PGetName(pid) == partname:
            return pid
    print(f"GetMeshPartnameIdx: Warning: cannot find \"{partname}\"")
    return -1


#
def main():
    if CONFIG["fce_version"] == "keep":
        fce_outversion = str(GetFceVersion(filepath_fce_input))
        if fce_outversion == "5":
            fce_outversion = "4M"
    else:
        fce_outversion = CONFIG["fce_version"]
    mesh = fc.Mesh()
    mesh = LoadFce(mesh, filepath_fce_input)

    # obtain target part indexes
    if str(GetFceVersion(filepath_fce_input)) == "3":
        pids = np.array([0, ])  # assume FCE3 input has high body at index 0
    else:
        pids = np.array([GetMeshPartnameIdx(mesh, pn) for pn in (":HB", ":OT",  # FCE4
                                                                 ":Hbody")])  # FCE4M

    # edit parts
    for pid in pids[pids >= 0]:
        flags = mesh.PGetTriagsFlags(pid)
        # triangle flags documentation in fcelib_fcetypes.h
        flags = np.where(flags & 0x2 == 0x2, flags + 0x8, flags)
        mesh.PSetTriagsFlags(pid, flags)

    WriteFce(fce_outversion, mesh, filepath_fce_output, mesh_function=HiBody_ReorderTriagsTransparentToLast)
    PrintFceInfo(filepath_fce_output)
    print(f"FILE = {filepath_fce_output}", flush=True)

if __name__ == "__main__":
    main()
