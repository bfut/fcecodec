"""
  bfut_PrintFceStats.py - print given FCE file stats to console

REQUIRES: installing <https://github.com/bfut/fcecodec>

LICENSE:
  Copyright (C) 2021 and later Benjamin Futasz <https://github.com/bfut>
  This file is distributed under: CC BY-NC 4.0
      <https://creativecommons.org/licenses/by-sa/4.0/>
"""
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
    print(p)
    for x in p.glob("**"):
        sys.path.append(str(x.resolve()))
    import fcecodec

# Parse command (or print module help)
parser = argparse.ArgumentParser()
parser.add_argument("cmd", nargs=1, help="path")
args = parser.parse_args()

filepath_fce_input = args.cmd[0]


# -------------------------------------- wrappers
def PrintFceInfo(path):
    with open(path, "rb") as f:
        buf = f.read()
        fcecodec.PrintFceInfo(buf)
        assert(fcecodec.ValidateFce(buf) == 1)


# -------------------------------------- workload
PrintFceInfo(filepath_fce_input)