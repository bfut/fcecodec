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
    bfut_MergeAllParts.py - merge all parts, keep FCE version

USAGE
    python bfut_MergeAllParts.py /path/to/model.fce [/path/to/output.fce]

REQUIRES
    installing <https://github.com/bfut/fcecodec>
"""
import argparse
import pathlib

import fcecodec as fc

from bfut_mywrappers import *  # fcecodec/scripts/bfut_mywrappers.py

CONFIG = {
    "fce_version" : "keep",  # output format version; expects "keep" or "3"|"4"|"4M" for FCE3, FCE4, FCE4M, respectively
    "center_parts" : True,  # localize part vertice positions to part centroid, setting part position (expects 0|1)
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
    print(f"OUTPUT = {filepath_fce_output}", flush=True)

if __name__ == "__main__":
    main()
