# Copyright (C) 2022 and later Benjamin Futasz <https://github.com/bfut>
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
    bfut_ConvertDummiesToFce3.py - description

HOW TO USE
    python bfut_ConvertDummiesToFce3.py /path/to/model.fce

REQUIRES
    installing <https://github.com/bfut/fcecodec>
"""
import argparse
import pathlib
import sys

import fcecodec
import numpy as np

CONFIG = {
    "fce_version"  : "keep",  # output format version; expects "keep" or "3"|"4"|"4M" for FCE3, FCE4, FCE4M, respectively
    "center_parts" : 0,  # localize part vertice positions to part centroid, setting part position (expects 0|1)
}

script_path = pathlib.Path(__file__).parent

# Parse command-line
parser = argparse.ArgumentParser()
parser.add_argument("path", nargs="+", help="file path")
args = parser.parse_args()

filepath_fce_input = pathlib.Path(args.path[0])
if len(args.path) < 2:
    filepath_fce_output = filepath_fce_input.parent / (filepath_fce_input.stem + "_out" + filepath_fce_input.suffix)
else:
    filepath_fce_output = pathlib.Path(args.path[1])


# -------------------------------------- wrappers
sys.path.append(str(pathlib.Path(__file__).resolve()))
sys.path.append(str((pathlib.Path(__file__).parent / "../python/").resolve()))
from bfut_mywrappers import *


def GetDummies(mesh):
    dms_pos = mesh.MGetDummyPos()
    dms_pos = np.reshape(dms_pos, (int(dms_pos.shape[0] / 3), 3))
    dms_names = mesh.MGetDummyNames()
    return dms_pos, dms_names

def SetDummies(mesh, dms_pos, dms_names):
    dms_pos = np.reshape(dms_pos, (int(dms_pos.shape[0] * 3)))
    dms_pos = dms_pos.astype("float32")
    mesh.MSetDummyPos(dms_pos)
    mesh.MSetDummyNames(dms_names)
    return mesh

def DummiesToFce3(dms_pos, dms_names):
    for i in range(len(dms_names)):
        x = dms_names[i]
        """
        if x[0] in [":", "B", "I", "M", "P", "R"]:
            print(x, "->", dms_names[i])
            continue
        # """
        if x[0] == "B":
            # dms_names[i] = "TRLO"  # convert brake lights to taillights?
            pass
        elif x[0] in ["H", "I"]:
            if x[3] == "O":
                dms_names[i] = "HFLO"
            elif x[3] == "E":
                dms_names[i] = "HFRE"
            elif dms_pos[i, 0] < 0:  # left-hand
                dms_names[i] = "HFLN"
            else:
                dms_names[i] = "HFRN"
            """
            # T - fce4 taillights seemingly work for fce3
            elif x[0] == "T":
                if x[3] == "O":
                    dms_names[i] = "TRLO"
                elif x[3] == "E":
                    dms_names[i] = "TRRE"
                elif dms_pos[i, 0] < 0:  # left-hand
                    dms_names[i] = "TRLN"
                else:
                    dms_names[i] = "TRRN"
            # """
        elif x[0] == "S":
            if x[1] == "B":
                dms_names[i] = "SMLN"  # blue
            else:
                dms_names[i] = "SMRN"  # red
        print(x, "->", dms_names[i])
    return dms_pos, dms_names

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

    dms_pos, dms_names = GetDummies(mesh)
    dms_pos, dms_names = DummiesToFce3(dms_pos, dms_names)
    mesh = SetDummies(mesh, dms_pos, dms_names)

    WriteFce(fce_outversion, mesh, filepath_fce_output, CONFIG["center_parts"])
    PrintFceInfo(filepath_fce_output)
    print("FILE =", filepath_fce_output, flush=True)

if __name__ == "__main__":
    main()
