"""
  bfut_MergeAll.py - merge all parts, keep FCE version

REQUIRES: installing <https://github.com/bfut/fcecodec>

LICENSE:
  This file is distributed under: CC BY-SA 4.0
      <https://creativecommons.org/licenses/by-sa/4.0/>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.
"""
CONFIG = {
    "fce_version"  : 'keep version',  # output format version; expects 'keep version' or '3'|'4'|'4M' for FCE3, FCE4, FCE4M output, respectively
    "center_parts" : 1,  # localize part vertice positions to part centroid, setting part position (expects 0|1)
}
import argparse
import pathlib
import sys

script_path = pathlib.Path(__file__).parent

# Look for local build, if not installed
try:
    import fcecodec
except ModuleNotFoundError:
    import sys
    p = pathlib.Path(script_path / "../python/build")
    # print(p)
    for x in p.glob("**"):
        sys.path.append(str(x.resolve()))
    import fcecodec

# Parse command (or print module help)
parser = argparse.ArgumentParser()
parser.add_argument("cmd", nargs=1, help="path")
args = parser.parse_args()

filepath_fce_input = pathlib.Path(args.cmd[0])
filepath_fce_output = filepath_fce_input.with_stem(filepath_fce_input.stem + "_out")


# -------------------------------------- wrappers
sys.path.append(str( pathlib.Path(pathlib.Path(__file__).parent / "../python/").resolve()))
from bfut_mywrappers import *


# -------------------------------------- workload
if CONFIG["fce_version"] == 'keep_version':
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