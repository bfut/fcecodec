"""
    fcecodecScriptTemplate.py - description

HOW TO USE
    python fcecodecScriptTemplate.py /path/to/model.fce

REQUIRES
    installing <https://github.com/bfut/fcecodec>

LICENSE
    This file is in the PUBLIC DOMAIN.
"""
import argparse
import pathlib
import sys
import numpy as np

import fcecodec

CONFIG = {
    "fce_version"  : "keep",  # output format version; expects "keep" or "3"|"4"|"4M" for FCE3, FCE4, FCE4M, respectively
    "center_parts" : 1,  # localize part vertice positions to part centroid, setting part position (expects 0|1)
}

script_path = pathlib.Path(__file__).parent

# Parse command (or print module help)
parser = argparse.ArgumentParser()
parser.add_argument("cmd", nargs="+", help="path")
args = parser.parse_args()

filepath_fce_input = pathlib.Path(args.cmd[0])
filepath_fce_output = pathlib.Path(
    filepath_fce_input.parent,
    (filepath_fce_input.stem + "_out" + filepath_fce_input.suffix)
)


# -------------------------------------- wrappers
sys.path.append(str(pathlib.Path(pathlib.Path(__file__)).resolve()))
sys.path.append(str(pathlib.Path(pathlib.Path(__file__).parent / "../python/").resolve()))
from bfut_mywrappers import *


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
    ## do stuff here


    ## done doing stuff
    WriteFce(fce_outversion, mesh, filepath_fce_output, CONFIG["center_parts"])
    PrintFceInfo(filepath_fce_output)
    print("FILE =", filepath_fce_output, flush=True)

if __name__ == "__main__":
    main()