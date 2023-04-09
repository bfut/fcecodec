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
    bfut_SetNormals (project verts to unit sphere).py - description

HOW TO USE
    python "bfut_SetNormals (project verts to unit sphere).py" /path/to/model.fce

REQUIRES
    installing <https://github.com/bfut/fcecodec>
"""
import argparse
import pathlib

import fcecodec
import numpy as np

CONFIG = {
    "fce_version"  : "keep",  # output format version; expects "keep" or "3"|"4"|"4M" for FCE3, FCE4, FCE4M, respectively
    "center_parts" : 1,  # localize part vertice positions to part centroid, setting part position (expects 0|1)
    "sphere_radius": 1.0,
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
def GetFceVersion(path):
    with open(path, "rb") as f:
        version = fcecodec.GetFceVersion(f.read(0x2038))
        assert version > 0
        return version

def LoadFce(mesh, path):
    with open(path, "rb") as f:
        mesh.IoDecode(f.read())
        assert mesh.MValid() is True
        return mesh

def WriteFce(version, mesh, path, center_parts=1, mesh_function=None):
    if mesh_function is not None:  # e.g., HiBody_ReorderTriagsTransparentToLast
        mesh = mesh_function(mesh, version)
    with open(path, "wb") as f:
        if version == "3":
            buf = mesh.IoEncode_Fce3(center_parts)
        elif version == "4":
            buf = mesh.IoEncode_Fce4(center_parts)
        else:
            buf = mesh.IoEncode_Fce4M(center_parts)
        assert fcecodec.ValidateFce(buf) == 1
        f.write(buf)


#
def main():
    if CONFIG["fce_version"] == "keep":
        fce_outversion = str(GetFceVersion(filepath_fce_input))
        if fce_outversion == "5":
            fce_outversion = "4M"
    else:
        fce_outversion = CONFIG["fce_version"]
    if not isinstance(CONFIG["sphere_radius"], float):
        raise ValueError(f'expected CONFIG["sphere_radius"] as float, is {type(CONFIG["sphere_radius"])}')
    sphere_radius = CONFIG["sphere_radius"]

    mesh = fcecodec.Mesh()
    mesh = LoadFce(mesh, filepath_fce_input)

    ## do stuff here
    # Get vertices
    print(f"mesh.MVertsNorms={np.reshape(mesh.MVertsNorms, (mesh.MNumVerts, 3))[:10]}")
    norms_o = mesh.MVertsNorms
    verts_o = mesh.MVertsPos

    # Project vertices to sphere surface
    # https://stackoverflow.com/questions/9604132/how-to-project-a-point-on-to-a-sphere
    # FCE vertices positions are local, i.e., already centered around the origin point
    norms = verts_o.copy()
#    norms = np.roll(norms, 8)
    print(f"mesh.MVertsPos={np.reshape(norms, (mesh.MNumVerts, 3))[:10]}")
    for i in range(mesh.MNumVerts):
        xyz = norms[i*3+0:i*3+3].copy()
        # xyz[2] = -xyz[2]
        # xyz = np.roll(xyz, 1)
        xyz_norm = np.linalg.norm(xyz, ord=None)
        xyz_sph = sphere_radius * (xyz_norm + np.finfo(float).eps)**-1 * xyz
        norms[i*3+0:i*3+3] = xyz_sph.copy()

        if i < 3:
            print(f"{i}")
            print(f"xyz={xyz}")
            print(f"xyz_norm={xyz_norm}")
            print(f"sphere_radius={sphere_radius}")
            print(f"xyz_sph={xyz_sph} with norm {np.linalg.norm(xyz_sph, ord=None)}")
    print(f"mesh.MVertsPos={np.reshape(norms, (mesh.MNumVerts, 3))[:10]}")
    mesh.MVertsNorms = norms
    print(f"mesh.MVertsNorms={np.reshape(mesh.MVertsNorms, (mesh.MNumVerts, 3))[:10]}")

    ## done doing stuff
    WriteFce(fce_outversion, mesh, filepath_fce_output, CONFIG["center_parts"],
             mesh_function=None)
    print(f"FILE = {filepath_fce_output}", flush=True)

if __name__ == "__main__":
    main()
