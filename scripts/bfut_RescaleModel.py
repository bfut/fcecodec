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
#     claim that you wrote the original software. If you use this software
#     in a product, an acknowledgment in the product documentation would be
#     appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#     misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.
"""
    bfut_RescaleModel.py - rescales model size by factor

USAGE
    python bfut_RescaleModel.py /path/to/model.fce [/path/to/output.fce]
    python bfut_RescaleModel.py 1.2 /path/to/model.fce [/path/to/output.fce]

    The rescale factor can be set in the script header or passed as float argument.
    Dummy positions are rescaled accordingly as well.
    This script centers all parts to their respective centroids by default.

REQUIRES
    installing <https://github.com/bfut/fcecodec>
"""
import argparse
import pathlib

import fcecodec

CONFIG = {
    "fce_version"  : "keep",  # output format version; expects "keep" or "3"|"4"|"4M" for FCE3, FCE4, FCE4M, respectively
    "center_parts" : True,  # localize part vertice positions to part centroid, setting part position (expects 0|1)
    "rescale_factor" : 1.2,  # 1.1 and 1.2 give good results for vanilla FCE4/FCE4M models; unchanged if 1.0 or smaller than 0.1
}


# -------------------------------------- wrappers
def GetFceVersion(path):
    with open(path, "rb") as f:
        version = fcecodec.GetFceVersion(f.read(0x2038))
        assert version > 0
        return version

def PrintFceInfo(path):
    with open(path, "rb") as f:
        buf = f.read()
        fcecodec.PrintFceInfo(buf)
        assert fcecodec.ValidateFce(buf) == 1

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
    # Parse command-line
    parser = argparse.ArgumentParser()
    parser.add_argument("path", nargs="+", help="file path")
    args = parser.parse_args()

    # Handle paths: 0 or 1 parameter, mandatory inpath, optional outpath
    arg_ofs = 0
    if not pathlib.Path(args.path[arg_ofs]).is_file():
        arg_ofs += 1
    filepath_fce_input = pathlib.Path(args.path[arg_ofs + 0])
    if len(args.path) < arg_ofs + 1:
        filepath_fce_output = filepath_fce_input.parent / (filepath_fce_input.stem + "_out" + filepath_fce_input.suffix)
    else:
        filepath_fce_output = pathlib.Path(args.path[arg_ofs + 1])


    # Process configuration / parameters
    if CONFIG["fce_version"] == "keep":
        fce_outversion = str(GetFceVersion(filepath_fce_input))
        if fce_outversion == "5":
            fce_outversion = "4M"
    else:
        fce_outversion = CONFIG["fce_version"]

    if arg_ofs > 0:
        rescale_factor = float(args.path[0])
    else:
        rescale_factor = float(CONFIG["rescale_factor"])
    if rescale_factor < 0.1:
        rescale_factor = 1.0

    # Load FCE
    mesh = fcecodec.Mesh()
    mesh = LoadFce(mesh, filepath_fce_input)

    # Rescale
    print(f"rescale_factor={rescale_factor}")
    if rescale_factor > 0.0 and abs(rescale_factor) > 0.1:
        for pid in reversed(range(mesh.MNumParts)):
            mesh.OpCenterPart(pid)
            ppos = mesh.PGetPos(pid)  # xyz
            mesh.PSetPos(pid, ppos * rescale_factor)
        v = mesh.MVertsPos  # xyzxyzxyz...
        mesh.MVertsPos = v * rescale_factor
        dv = mesh.MGetDummyPos()  # xyzxyzxyz...
        mesh.MSetDummyPos(dv * rescale_factor)

    # Write FCE
    WriteFce(fce_outversion, mesh, filepath_fce_output, CONFIG["center_parts"])
    PrintFceInfo(filepath_fce_output)
    print("OUTPUT =", filepath_fce_output, flush=True)

if __name__ == "__main__":
    main()
