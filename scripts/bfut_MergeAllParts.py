"""
  bfut_MergeAllParts.py - merge all parts, keep FCE version

HOW TO USE
    python bfut_MergeAllParts.py /path/to/model.fce

REQUIRES
    installing <https://github.com/bfut/fcecodec>

LICENSE
  Copyright (C) 2021 Benjamin Futasz <https://github.com/bfut>
  This file is distributed under: CC BY-NC 4.0
      <https://creativecommons.org/licenses/by-sa/4.0/>
"""
CONFIG = {
    "fce_version"  : 'keep',  # output format version; expects 'keep' or '3'|'4'|'4M' for FCE3, FCE4, FCE4M, respectively
    "center_parts" : 1,  # localize part vertice positions to part centroid, setting part position (expects 0|1)
}
import argparse
import pathlib

import fcecodec

# Parse command (or print module help)
parser = argparse.ArgumentParser()
parser.add_argument("cmd", nargs='+', help="path")
args = parser.parse_args()

filepath_fce_input = pathlib.Path(args.cmd[0])
filepath_fce_output = pathlib.Path(
                  filepath_fce_input.parent,
                  (filepath_fce_input.stem + "_out" + filepath_fce_input.suffix)
              )


# -------------------------------------- wrappers
def GetFceVersion(path):
    with open(path, "rb") as f:
        buf = f.read(0x2038)
        version = fcecodec.GetFceVersion(buf)
        assert(version > 0)
        return version

def PrintFceInfo(path):
    with open(path, "rb") as f:
        buf = f.read()
        fcecodec.PrintFceInfo(buf)
        assert(fcecodec.ValidateFce(buf) == 1)

def LoadFce(mesh, path):
    with open(path, "rb") as f:
        fce_buf = f.read()
    assert(fcecodec.ValidateFce(fce_buf) == 1)
    mesh.IoDecode(fce_buf)
    assert(mesh.MValid() == True)
    return mesh

def WriteFce(version, mesh, path, center_parts = 1):
    with open(path, "wb") as f:
        # print(version == '3', version == '4', version)
        if version == '3':
            buf = mesh.IoEncode_Fce3(center_parts)
        elif version == '4':
            buf = mesh.IoEncode_Fce4(center_parts)
        else:
            buf = mesh.IoEncode_Fce4M(center_parts)
        assert(fcecodec.ValidateFce(buf) == 1)


# -------------------------------------- workload
if CONFIG["fce_version"] == 'keep':
    fce_outversion = str(GetFceVersion(filepath_fce_input))
    if fce_outversion == '5':
        fce_outversion = '4M'
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

WriteFce(CONFIG["fce_version"], mesh, filepath_fce_output, CONFIG["center_parts"])
PrintFceInfo(filepath_fce_output)
print("FILE =", filepath_fce_output, flush=True)