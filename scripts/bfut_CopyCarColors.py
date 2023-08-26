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
    bfut_CopyCarColors.py - copy car colors from source to target, overwriting target

USAGE
    python bfut_CopyCarColors.py /path/to/source.fce /path/to/target.fce

REQUIRES
    installing <https://github.com/bfut/fcecodec>
"""
import argparse
import pathlib

import fcecodec as fc

CONFIG = {
    "fce_version" : "keep",  # output format version; expects "keep" or "3"|"4"|"4M" for FCE3, FCE4, FCE4M, respectively
}


# -------------------------------------- wrappers
def GetFceVersion(path):
    with open(path, "rb") as f:
        version = fc.GetFceVersion(f.read(0x2038))
        assert version > 0
        return version

def PrintFceInfo(path):
    with open(path, "rb") as f:
        buf = f.read()
        fc.PrintFceInfo(buf)
        assert fc.ValidateFce(buf) == 1

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
        assert fc.ValidateFce(buf) == 1
        f.write(buf)


#
def main():
    # Parse command-line
    parser = argparse.ArgumentParser()
    parser.add_argument("path", nargs=2, help="file path")
    args = parser.parse_args()

    # Handle paths: mandatory source_path, mandatory target_path
    filepath_fce_input_source = pathlib.Path(args.path[0])
    filepath_fce_input = pathlib.Path(args.path[1])
    filepath_fce_output = filepath_fce_input

    if CONFIG["fce_version"] == "keep":
        fce_outversion = str(GetFceVersion(filepath_fce_input))
        if fce_outversion == "5":
            fce_outversion = "4M"
    else:
        fce_outversion = CONFIG["fce_version"]

    # Load FCE
    mesh = fc.Mesh()
    mesh = LoadFce(mesh, filepath_fce_input)

    # Copy colors
    mesh_source = fc.Mesh()
    mesh_source = LoadFce(mesh_source, filepath_fce_input_source)
    mesh.MSetColors(mesh_source.MGetColors())

    # Output FCE
    WriteFce(fce_outversion, mesh, filepath_fce_output)
    PrintFceInfo(filepath_fce_output)
    print(f"FILE = {filepath_fce_output}", flush=True)


if __name__ == "__main__":
    main()
