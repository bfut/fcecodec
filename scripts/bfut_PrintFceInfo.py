"""
    bfut_PrintFceInfo.py - print given FCE file stats to console

HOW TO USE
    python bfut_PrintFceInfo.py /path/to/model.fce

REQUIRES
    installing <https://github.com/bfut/fcecodec>

LICENSE
    Copyright (C) 2021 Benjamin Futasz <https://github.com/bfut>
    This file is distributed under: CC BY-NC 4.0
        <https://creativecommons.org/licenses/by-sa/4.0/>
"""
import argparse

import fcecodec

# Parse command (or print module help)
parser = argparse.ArgumentParser()
parser.add_argument("cmd", nargs="+", help="path")
args = parser.parse_args()

filepath_fce_input = args.cmd[0]


# -------------------------------------- wrappers
def PrintFceInfo(path):
    with open(path, "rb") as f:
        buf = f.read()
        fcecodec.PrintFceInfo(buf)
        assert fcecodec.ValidateFce(buf) == 1


#
def main():
    PrintFceInfo(filepath_fce_input)

if __name__ == "__main__":
    main()