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
    bfut_MergeAllParts.py - merge all parts, keep FCE version

HOW TO USE
    python bfut_MergeAllParts.py /path/to/model.fce

REQUIRES
    installing <https://github.com/bfut/fcecodec>
"""
import argparse
import pathlib

import fcecodec

CONFIG = {
    "fce_version"  : "keep",  # output format version; expects 'keep' or '3'|'4'|'4M' for FCE3, FCE4, FCE4M, respectively
    "center_parts" : 0,  # localize part vertice positions to part centroid, setting part position (expects 0|1)
}

# Parse command-line
parser = argparse.ArgumentParser()
parser.add_argument("path", nargs="+", help="file path")
args = parser.parse_args()

filepath_fce_input = pathlib.Path(args.path[0])
filepath_fce_output = filepath_fce_input.parent / (filepath_fce_input.stem + "_out" + filepath_fce_input.suffix)


# -------------------------------------- wrappers
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


#
def main():
    if CONFIG["fce_version"] == "keep":
        fce_outversion = str(GetFceVersion(filepath_fce_input))
        if fce_outversion == "5":
            fce_outversion = "4M"
    else:
        fce_outversion = CONFIG["fce_version"]
    mesh = fcecodec.Mesh()
    mesh = LoadFce(mesh, filepath_fce_input)

    # merge
    if mesh.MNumParts > 1:
        pid1 = mesh.MNumParts - 1
        for pid2 in reversed(range(mesh.MNumParts - 1)):
            print("merging parts", pid1, pid2)
            pid1 = mesh.OpMergeParts(pid1, pid2)

    # cleanup
    while mesh.MNumParts > 1:
        print("deleting part", mesh.PGetName(0))
        mesh.OpDeletePart(0)

    if fce_outversion == "4":
        mesh.PSetName(0, ":HB")
    elif fce_outversion == "4M":
        mesh.PSetName(0, ":Hbody")
    WriteFce(fce_outversion, mesh, filepath_fce_output, CONFIG["center_parts"])
    PrintFceInfo(filepath_fce_output)
    print("FILE =", filepath_fce_output, flush=True)

if __name__ == "__main__":
    main()
