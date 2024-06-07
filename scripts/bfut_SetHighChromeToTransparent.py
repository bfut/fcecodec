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

from bfut_mywrappers import *  # fcecodec/scripts/bfut_mywrappers.py

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
