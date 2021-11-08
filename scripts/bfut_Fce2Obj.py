"""
  bfut_Fce2Obj.py - export given FCE file to OBJ/MTL files in FCE file directory, with triangles flag hex value as materials name

REQUIRES: installing <https://github.com/bfut/fcecodec>

DEFAULT PARAMETERS:
  objtexname=<fce_filename>00.png : texture file path in MTL file
  print_damage=1 : prints parts damage model to extra shapes named DAMAGE_<partname> (only relevant for FCE4, FCE4M)
  print_dummies=1 : prints extra shapes named DUMMY_<dummyname> for each dummy, centered around dummy position

LICENSE:
  This file is distributed under: CC BY-SA 4.0
      <https://creativecommons.org/licenses/by-sa/4.0/>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.
"""
import argparse
import os
import pathlib
import sys

script_path = pathlib.Path(__file__).parent

# Look for local build, if not installed
try:
    import fcecodec
except ModuleNotFoundError:
    import sys
    p = pathlib.Path(script_path / "../python/build")
    # print(p)
    for x in p.glob("**"):
        sys.path.append(str(x.resolve()))
    import fcecodec

# Parse command (or print module help)
parser = argparse.ArgumentParser()
parser.add_argument("cmd", nargs=1, help="path")
args = parser.parse_args()

filepath_fce_input = pathlib.Path(args.cmd[0])
output_path_stem = pathlib.Path(filepath_fce_input.parent / filepath_fce_input.stem)
filepath_obj_output = output_path_stem.with_suffix(".obj")
filepath_mtl_output = output_path_stem.with_suffix(".mtl")


# -------------------------------------- parameters
objtexname = filepath_fce_input.stem + "00.png"
print_damage = 1
print_dummies = 1


# -------------------------------------- wrappers
def LoadFce(mesh, path):
    # print("LoadFce(", mesh, ",", path, ")")
    with open(path, "rb") as f:
        fce_buf = f.read()
    assert(fcecodec.ValidateFce(fce_buf) == 1)
    mesh.IoDecode(fce_buf)
    assert(mesh.MValid() == True)
    return mesh

def ExportObj(mesh, objpath, mtlpath, texname, print_damage, print_dummies):
    # print("IoExportObj(", mesh, objpath, mtlpath, texname, print_damage, print_dummies, ")")
    mesh.IoExportObj(str(objpath), str(mtlpath), str(texname), print_damage, print_dummies)


# -------------------------------------- workload
if print_damage == 1:
    print("printing parts damage model to extra shapes")
if print_dummies == 1:
    print("printing extra shapes for each dummy")
mesh = fcecodec.Mesh()
mesh = LoadFce(mesh, filepath_fce_input)
os.chdir(filepath_obj_output.parent)
ExportObj(mesh,
          filepath_obj_output.name, filepath_mtl_output.name, objtexname,
          print_damage, print_dummies)
print(filepath_obj_output)
print(filepath_mtl_output)