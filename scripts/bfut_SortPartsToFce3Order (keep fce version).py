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
    bfut_SortPartsToFce3Order (keep fce version).py - order documented in fcelib_fcetypes.h

USAGE
    python "bfut_SortPartsToFce3Order (keep fce version).py" /path/to/model.fce [/path/to/output.fce]

REQUIRES
    installing <https://github.com/bfut/fcecodec>
"""
import argparse
import pathlib

import fcecodec as fc

from bfut_mywrappers import *  # fcecodec/scripts/bfut_mywrappers.py

CONFIG = {
    "fce_version"  : "keep",  # output format version; expects "keep" or "3"|"4"|"4M" for FCE3, FCE4, FCE4M, respectively
}

# Parse command-line
parser = argparse.ArgumentParser()
parser.add_argument("path", nargs="+", help="file path")
args = parser.parse_args()

filepath_fce_input = pathlib.Path(args.path[0])
if len(args.path) < 2:
    filepath_fce_output = filepath_fce_input.parent / (filepath_fce_input.stem + "_out" + filepath_fce_input.suffix)
else:
    filepath_fce_output = pathlib.Path(args.path[1])

#
def PrintMeshParts_order(mesh, part_names_sorted):
    print("pid  IS                          SHOULD")
    for pid in range(mesh.MNumParts):
        print(f"{pid:<2} {mesh.PGetName(pid):<12} {part_names_sorted[pid]:<12}")

def AssertPartsOrder(mesh, part_names_sorted, onlywarn=False):
    for pid in range(mesh.MNumParts):
        if mesh.PGetName(pid) != part_names_sorted[pid]:
            PrintMeshParts_order(mesh, part_names_sorted)
            if not onlywarn:
                raise AssertionError (f"pid={pid} {mesh.PGetName(pid)} != {part_names_sorted[pid]}")
            print(f"AssertPartsOrder: Warning pid={pid} {mesh.PGetName(pid)} != {part_names_sorted[pid]}")

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

    # sort
    if mesh.MNumParts > 1:
        priority_dic = {  # NB1: front wheel order differs for high body/medium body
            "high body": 0,
            "left front wheel": 1,
            "right front wheel": 2,
            "left rear wheel": 3,
            "right rear wheel": 4,
            "medium body": 5,
            "medium r front wheel": 6,
            "medium l front wheel": 7,
            "medium r rear wheel": 8,
            "medium l rear wheel": 9,
            "small body": 10,
            "tiny body": 11,
            "high headlights": 12,

            ":HB": 0,
            ":HLFW": 1,
            ":HRFW": 2,
            ":HLRW": 3,
            ":HRRW": 4,
            ":MB": 5,
            ":MRFW": 6,
            ":MLFW": 7,
            ":MRRW": 8,
            ":MLRW": 9,
            ":LB": 10,
            ":TB": 11,
            ":OL": 12,

            ":Hbody": 0,
        }

        part_names = []
        for pid in reversed(range(mesh.MNumParts)):
            part_names += [mesh.PGetName(pid)]
        part_names_sorted = sorted(part_names, key=lambda x: priority_dic.get(x, 64))

        for target_idx in range(0, len(part_names_sorted)):
            pname = part_names_sorted[target_idx]
            current_idx = GetMeshPartnameIdx(mesh, pname)
            # print(f" {pname} {current_idx} -> {target_idx}")
            while current_idx > target_idx:
                current_idx = mesh.OpMovePart(current_idx)
        AssertPartsOrder(mesh, part_names_sorted, onlywarn=True)
        # PrintMeshParts(mesh, part_names_sorted)

    WriteFce(fce_outversion, mesh, filepath_fce_output)
    PrintFceInfo(filepath_fce_output)
    print(f"OUTPUT = {filepath_fce_output}", flush=True)

if __name__ == "__main__":
    main()
