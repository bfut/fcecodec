# Copyright (C) 2023 and later Benjamin Futasz <https://github.com/bfut>
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
    bfut_SaveFceAsFce4.py - change file FCE version to FCE4 and overwrite input

USAGE
    python "bfut_SaveFceAsFce4.py" /path/to/model.fce [/path/to/output.fce]

REQUIRES
    installing fcecodec <https://github.com/bfut/fcecodec>
"""
import argparse
import pathlib

import fcecodec

CONFIG = {
    "fce_version"        : "4",  # output format version; expects "keep" or "3"|"4"|"4M" for FCE3, FCE4, FCE4M, respectively
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


# -------------------------------------- wrappers
def LoadFce(mesh, path):
    with open(path, "rb") as f:
        mesh.IoDecode(f.read())
        assert mesh.MValid() is True
        return mesh

def WriteFce(version, mesh, path, center_parts=False, mesh_function=None):
    if mesh_function is not None:  # e.g., HiBody_ReorderTriagsTransparentToLast
        mesh = mesh_function(mesh, version)
    with open(path, "wb") as f:
        if version in ("3", 3):
            buf = mesh.IoEncode_Fce3(center_parts)
        elif version in ("4", 4):
            buf = mesh.IoEncode_Fce4(center_parts)
        else:
            buf = mesh.IoEncode_Fce4M(center_parts)
        assert fcecodec.ValidateFce(buf) == 1
        f.write(buf)

#
def main():
    mesh = fcecodec.Mesh()
    mesh = LoadFce(mesh, filepath_fce_input)
    WriteFce(CONFIG["fce_version"], mesh, filepath_fce_output)
    print(f"FILE = {filepath_fce_output}", flush=True)

if __name__ == "__main__":
    main()

