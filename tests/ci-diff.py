"""
  ci-diff.py -

  This file is distributed under: CC BY-SA 4.0
      <https://creativecommons.org/licenses/by-sa/4.0/>
"""
import os
import pathlib
import sys

# Look for local build, if not installed
try:
    import fcecodec
except ModuleNotFoundError:

    import sys
    p = pathlib.Path(pathlib.Path(__file__).parent / "../python/build")
    print(p)
    for x in p.glob("**"):
        sys.path.append(str(x.resolve()))

    import fcecodec


# --------------------------------------
script_path = pathlib.Path(__file__).parent
filepath_fce_input = pathlib.Path(script_path / 'fce/Snowman_car.fce')
filepath_fce3_output  = pathlib.Path(script_path / '.out/ci-smoketest3.fce')
filepath_fce4_output  = pathlib.Path(script_path / '.out/ci-smoketest4.fce')
filepath_fce4m_output = pathlib.Path(script_path / '.out/ci-smoketest4m.fce')

filepath_obj_output = pathlib.Path(script_path / '.out/ci-smoketest.obj')
filepath_mtl_output = pathlib.Path(script_path / '.out/ci-smoketest.mtl')
objtexname = 'car00_Snowman.png'

try:
    os.mkdir(pathlib.Path(script_path / '.out'))
except FileExistsError:
    None


# -------------------------------------- import python wrappers
sys.path.append(str( pathlib.Path(pathlib.Path(__file__).parent / "../python/").resolve()))
from bfut_mywrappers import *


# --------------------------------------
mesh = fcecodec.Mesh()
mesh = LoadFce(mesh, filepath_fce_input)

WriteFce('3', mesh, filepath_fce3_output, center_parts=0)
WriteFce('4', mesh, filepath_fce4_output, center_parts=0)
WriteFce("4M", mesh, filepath_fce4m_output, center_parts=0)
# ExportObj(mesh,
#           filepath_obj_output, filepath_mtl_output, objtexname,
#           print_damage=0, print_dummies=0)

center_parts = 0

# -------------------------------------- compare src->X with src->X->X
mesh = LoadFce(mesh, filepath_fce3_output)
WriteFce('3', mesh, str(filepath_fce3_output) + "diff33", center_parts)

mesh = LoadFce(mesh, filepath_fce4_output)
WriteFce('4', mesh, str(filepath_fce4_output) + "diff44", center_parts)

mesh = LoadFce(mesh, filepath_fce4m_output)
WriteFce("4M", mesh, str(filepath_fce4m_output) + "diff4m4m", center_parts)


# -------------------------------------- compare src->4 with src->4m->4
mesh = LoadFce(mesh, str(filepath_fce4m_output))
WriteFce('4', mesh, str(filepath_fce4_output) + "diff4m4", center_parts)