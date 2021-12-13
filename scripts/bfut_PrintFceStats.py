"""
  bfut_PrintFceStats.py - print given FCE file stats to console

REQUIRES: installing <https://github.com/bfut/fcecodec>

LICENSE:
  Copyright (C) 2021 and later Benjamin Futasz <https://github.com/bfut>
  This file is distributed under: CC BY-NC 4.0
      <https://creativecommons.org/licenses/by-sa/4.0/>
"""
import argparse
import os
import pathlib
import sys

import fcecodec

# Parse command (or print module help)
parser = argparse.ArgumentParser()
parser.add_argument("cmd", nargs='+', help="path")
args = parser.parse_args()

if os.name == "nt":
    filepath_fce_input = ' '.join(args.cmd)[:]
    filepath_fce_input = pathlib.Path(filepath_fce_input)
else:
    filepath_fce_input = args.cmd[0]


# -------------------------------------- wrappers
def PrintFceInfo(path):
    with open(path, "rb") as f:
        buf = f.read()
        fcecodec.PrintFceInfo(buf)
        assert(fcecodec.ValidateFce(buf) == 1)


# -------------------------------------- workload
PrintFceInfo(filepath_fce_input)