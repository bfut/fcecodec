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
    bfut_Fce2Obj.py - export FCE to Wavefront OBJ/MTL

DESCRIPTION
    Exports given FCE file to OBJ/MTL files in FCE file directory,
    with triangles flag hex values as materials names.

USAGE
    python bfut_Fce2Obj.py /path/to/model.fce [/path/to/output.obj]

    The output MTL path will differ from OBJ path by its extension ".mtl"

REQUIRES
    installing <https://github.com/bfut/fcecodec>
"""
import argparse
import pathlib

import fcecodec as fc

from bfut_mywrappers import *  # fcecodec/scripts/bfut_mywrappers.py

# Parse command-line
parser = argparse.ArgumentParser()
parser.add_argument("path", nargs="+", help="file path")
args = parser.parse_args()

# Handle paths: mandatory inpath, optional outpath
filepath_fce_input = pathlib.Path(args.path[0])
if len(args.path) < 2:
    filepath_obj_output = filepath_fce_input.with_suffix(".obj")
else:
    filepath_obj_output = pathlib.Path(args.path[1])

filepath_mtl_output = filepath_obj_output.with_suffix(".mtl")

CONFIG = {
    "fallback_texname"     : filepath_fce_input.stem + ".png",  # texture file path in MTL file
    "print_damage"         : 2,  # prints parts damage model to shapes named DAMAGE_<partname> (relevant for FCE4, FCE4M)
    "print_dummies"        : 1,  # prints shapes named DUMMY_##_<dummyname> for each dummy, centered on dummy position
    "use_part_positions"   : 1,
    "print_part_positions" : 1,  # prints shapes named PARTPOS_<partname> for each part, centered on part position
}

# -------------------------------------- more wrappers
def HeuristicTgaSearch(path, suffix=".tga"):
    """
    Heuristic search for TGA file in the same directory as the given file path.
    Returns "path/to/texname.tga" if found, else empty string.

    priority: <file>.tga <file>00.tga <any>.tga
    """
    path = pathlib.Path(path)
    suffix = str(suffix).lower()
    if not path.is_dir() and path.is_file():
        pdir = path.parent
    else:
        pdir = path
    texname = None
    pl = list(pdir.iterdir())
    pl.sort()
    for f in pl:
        fp = pathlib.Path(f.name)
        if fp.suffix.lower() != suffix:
            continue
        if fp.stem.lower() == path.stem.lower():
            texname = pdir / fp
            break
        if fp.stem.lower() == path.stem.lower() + "00":
            texname = pdir / fp
            break
    if not texname:
        for f in pl:
            fp = pathlib.Path(f.name)
            if fp.suffix.lower() == suffix:
                texname = pdir / fp
                break
    return str(texname) if texname else ""

#
def main():
    print_damage = CONFIG["print_damage"]
    if print_damage == 2 and GetFceVersion(filepath_fce_input) in ["3", 3]:
        print_damage = 0
    else:
        print_damage = 1
    if print_damage == 1:
        print("printing parts damage model to extra shapes")
    else:
        print("ignoring parts damage model")

    if CONFIG["print_dummies"] == 1:
        print("printing extra shapes for each dummy")
    if CONFIG["use_part_positions"] == 0:
        print("ignoring part positions")
    if CONFIG["print_part_positions"] == 1:
        print("printing extra shapes for each part position")
    mesh = fc.Mesh()
    mesh = LoadFce(mesh, filepath_fce_input)

    texname = HeuristicTgaSearch(filepath_fce_input.parent)
    if texname == "":
        texname = HeuristicTgaSearch(filepath_fce_input.parent, ".png")
    if texname == "":
        texname = HeuristicTgaSearch(filepath_fce_input.parent, ".bmp")
    if texname == "":
        texname = HeuristicTgaSearch(filepath_fce_input.parent, ".jpg")
    texname = texname if texname != "" else CONFIG["fallback_texname"]
    print(f"texname: {texname}")

    ExportObj(mesh,
        filepath_obj_output, filepath_mtl_output, texname,
        print_damage, CONFIG["print_dummies"],
        CONFIG["use_part_positions"], CONFIG["print_part_positions"])
    print(f"filepath_obj_output: {filepath_obj_output}")
    print(f"filepath_mtl_output: {filepath_mtl_output}")

if __name__ == "__main__":
    main()
