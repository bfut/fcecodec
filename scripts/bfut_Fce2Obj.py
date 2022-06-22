"""
    bfut_Fce2Obj.py

DESCRIPTION
    export given FCE file to OBJ/MTL files in FCE file directory,
    with triangles flag hex values as materials names

HOW TO USE
    python bfut_Fce2Obj.py /path/to/model.obj

REQUIRES
    installing <https://github.com/bfut/fcecodec>

LICENSE
    Copyright (C) 2021 and later Benjamin Futasz <https://github.com/bfut>
    This file is distributed under: zlib License

    This software is provided 'as-is', without any express or implied
    warranty.  In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
        claim that you wrote the original software. If you use this software
        in a product, an acknowledgment in the product documentation would be
        appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be
        misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.
"""
import argparse
import os
import pathlib

import fcecodec

# Parse command (or print module help)
parser = argparse.ArgumentParser()
parser.add_argument("cmd", nargs="+", help="path")
args = parser.parse_args()

filepath_fce_input = pathlib.Path(args.cmd[0])
output_path_stem = pathlib.Path(filepath_fce_input.parent / filepath_fce_input.stem)
filepath_obj_output = output_path_stem.with_suffix(".obj")
filepath_mtl_output = output_path_stem.with_suffix(".mtl")

CONFIG = {
    "objtexname"         : filepath_fce_input.stem + "00.png",  # texture file path in MTL file
    "print_damage"       : 1,  # prints parts damage model to shapes named DAMAGE_<partname> (relevant for FCE4, FCE4M)
    "print_dummies"      : 1,  # prints shapes named DUMMY_##_<dummyname> for each dummy, centered on dummy position
    "use_part_positions" : 1,
}


# -------------------------------------- wrappers
def LoadFce(mesh, path):
    with open(path, "rb") as f:
        fce_buf = f.read()
    assert fcecodec.ValidateFce(fce_buf) == 1
    mesh.IoDecode(fce_buf)
    assert mesh.MValid() is True
    return mesh

def ExportObj(mesh, objpath, mtlpath, texname, print_damage, print_dummies,
              use_part_positions):
    mesh.IoExportObj(str(objpath), str(mtlpath), str(texname), print_damage,
                     print_dummies, use_part_positions)


#
def main():
    if CONFIG["print_damage"] == 1:
        print("printing parts damage model to extra shapes")
    if CONFIG["print_dummies"] == 1:
        print("printing extra shapes for each dummy")
    if CONFIG["use_part_positions"] == 0:
        print("ignoring part positions")
    mesh = fcecodec.Mesh()
    mesh = LoadFce(mesh, filepath_fce_input)
    os.chdir(filepath_obj_output.parent)
    print(flush=True)
    ExportObj(mesh,
            filepath_obj_output.name, filepath_mtl_output.name, CONFIG["objtexname"],
            CONFIG["print_damage"], CONFIG["print_dummies"],
            CONFIG["use_part_positions"])
    print(filepath_obj_output)
    print(filepath_mtl_output)

if __name__ == "__main__":
    main()
