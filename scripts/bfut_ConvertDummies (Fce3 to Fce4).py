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
    bfut_ConvertDummies (Fce3 to Fce4).py - convert dummy names from FCE3 to FCE4 / FCE4M

USAGE
    python bfut_ConvertDummies (Fce3 to Fce4).py /path/to/model.fce [/path/to/output.fce]

REQUIRES
    installing <https://github.com/bfut/fcecodec>
"""
import argparse
import pathlib
import re

import fcecodec as fc
import numpy as np

from bfut_mywrappers import *  # fcecodec/scripts/bfut_mywrappers.py

CONFIG = {
    "fce_version" : "keep",  # output format version; expects "keep" or "3"|"4"|"4M" for FCE3, FCE4, FCE4M, respectively
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


# -------------------------------------- script functions
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

def DummiesFce3ToFce4(dms_pos, dms_names):
    for i in range(len(dms_names)):
        x = dms_names[i]
        tmp = []
        if len(str(x)) < 1:
            pass
        elif bool(re.search(r"\d", x)) or x[0] == ":":  # name contains integer
            pass  # do not convert canonical FCE4/FCE4M names
        elif x[0] == "H":
            tmp.append("HWY")  # kind, color, breakable
            if len(x) > 3:
                tmp.append(x[3])  # flashing
            else:
                tmp.append("N")
            tmp.append("5")  # intensity
            if len(x) > 3 and x[3] != "N":
                tmp.append("5")  # time
                if x[2] == "L":
                    tmp.append("0")  # delay
                else:  # == "R"
                    tmp.append("5")  # delay
            dms_names[i] = "".join(tmp)
        elif x[0] == "T":
            tmp.append("TRYN5")  # kind, color, breakable, flashing, intensity
            dms_names[i] = "".join(tmp)
        elif x[0] == "M":
            tmp.append("S")  # kind
            if len(x) > 2 and x[2] == "L":  # left
                tmp.append("RNO535")  # color, breakable, flashing, intensity, time, delay
            else:  # == "R"  # right
                tmp.append("BNE530")
            dms_names[i] = "".join(tmp)
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
    mesh = fc.Mesh()
    mesh = LoadFce(mesh, filepath_fce_input)

    dms_pos, dms_names = GetDummies(mesh)
    dms_pos, dms_names = DummiesFce3ToFce4(dms_pos, dms_names)
    mesh = SetDummies(mesh, dms_pos, dms_names)

    WriteFce(fce_outversion, mesh, filepath_fce_output)
    PrintFceInfo(filepath_fce_output)
    print(f"FILE = {filepath_fce_output}", flush=True)

if __name__ == "__main__":
    main()
