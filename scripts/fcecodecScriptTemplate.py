"""
  fcecodecScriptTemplate.py - description
"""
CONFIG = {
    "fce_version"  : 'keep',  # output format version; expects 'keep' or '3'|'4'|'4M' for FCE3, FCE4, FCE4M, respectively
    "center_parts" : 1,  # localize part vertice positions to part centroid, setting part position (expects 0|1)
}
import argparse
import pathlib
import sys
import numpy as np

script_path = pathlib.Path(__file__).parent

# Look for local build, if not installed
try:
    import fcecodec
except ModuleNotFoundError:
    import sys
    p = pathlib.Path(script_path / "../python/build")
    print(p)
    for x in p.glob("**"):
        sys.path.append(str(x.resolve()))
    import fcecodec

# Parse command (or print module help)
parser = argparse.ArgumentParser()
parser.add_argument("cmd", nargs=1, help="path")
args = parser.parse_args()

filepath_fce_input = pathlib.Path(args.cmd[0])
filepath_fce_output = filepath_fce_input.with_stem(filepath_fce_input.stem + "_o")


# -------------------------------------- wrappers
sys.path.append(str( pathlib.Path(pathlib.Path(__file__).parent / "../python/").resolve()))
from bfut_mywrappers import *
if 0:
    import bfut_mywrappers
    help(bfut_mywrappers)


# -------------------------------------- workload
if CONFIG["fce_version"] == 'keep':
    fce_outversion = str(GetFceVersion(filepath_fce_input))
    if fce_outversion == '5':
        fce_outversion = '4M'
else:
    fce_outversion = CONFIG["fce_version"]
mesh = fcecodec.Mesh()
mesh = LoadFce(mesh, filepath_fce_input)
## do stuff here


## done doing stuff
WriteFce(fce_outversion, mesh, filepath_fce_output, CONFIG["center_parts"])
PrintFceInfo(filepath_fce_output)
print("FILE =", filepath_fce_output, flush=True)