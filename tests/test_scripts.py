# fcecodec Copyright (C) 2021-2024 Benjamin Futasz <https://github.com/bfut>
#
# You may not redistribute this program without its source code.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
"""
  test_scripts.py - testing scripts in ./scripts
"""
import os
import pathlib
import subprocess
import sys

import fcecodec as fc
# import pytest

PYTHON_EXECUTABLE = sys.executable

sys.path.append(str((pathlib.Path(__file__).parent / "../scripts/").resolve()))
from bfut_mywrappers import *  # fcecodec/scripts/bfut_mywrappers.py

SCRIPT_PATH = pathlib.Path(__file__).parent
filepath_fce_input = SCRIPT_PATH / "fce/Snowman_car.fce"
filepath_obj_input = SCRIPT_PATH / "fce/Snowman_car.obj"
filepath_mergeallparts_output = SCRIPT_PATH / ".out/test_mergeallparts.fce"
filepath_fce3_output = SCRIPT_PATH / ".out/test_scripts3.fce"
filepath_fce4_output = SCRIPT_PATH / ".out/test_scripts4.fce"
filepath_fce4m_output = SCRIPT_PATH / ".out/test_scripts4m.fce"

filepath_obj_output = SCRIPT_PATH / ".out/Snowman_car.obj"
filepath_mtl_output = SCRIPT_PATH / ".out/Snowman_car.mtl"
objtexname = "car00_Snowman.tga"

#
if (SCRIPT_PATH / ".out").exists() and not (SCRIPT_PATH / ".out").is_dir():
    os.remove(SCRIPT_PATH / ".out")
    os.mkdir(SCRIPT_PATH / ".out")
elif not (SCRIPT_PATH / ".out").exists():
    os.mkdir(SCRIPT_PATH / ".out")


# Generate src->X and src->X->Y
def generate_data(version1, version2, filepath_fce_output):
    outpath = str(filepath_fce_output) + f"diff{version1}{version2}"
    mesh = fc.Mesh()

    mesh = LoadFce(mesh, filepath_fce_input)
    WriteFce(version1, mesh, filepath_fce_output, center_parts=0)

    mesh = LoadFce(mesh, filepath_fce_output)
    WriteFce(version2, mesh, outpath, center_parts=0)
    return outpath


def test_Fce2Obj():
    # The following OBJ string is:
    # Copyright (C) 2001-2024 Benjamin Futasz <https://github.com/bfut>
    # License: CC BY-NC-SA 4.0
    #     <https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode>
    fcecodec_Fce2Obj_src = filepath_obj_input.read_text("utf-8")

    assert pathlib.Path("./scripts/bfut_Fce2Obj.py").exists()
    assert filepath_fce_input.exists()
    p = subprocess.run(f"{PYTHON_EXECUTABLE} ./scripts/bfut_Fce2Obj.py {str(filepath_fce_input)} {str(filepath_obj_output)}",
                       shell=True, capture_output=True, encoding="utf8", check=True)
    outpath = filepath_obj_output
    assert outpath.exists()
    obj_output = outpath.read_text(encoding="utf-8")
    outpath.unlink()
    outpath.with_suffix(".mtl").unlink()
    assert p.returncode == 0 and len(p.stderr) == 0 and obj_output == fcecodec_Fce2Obj_src


def test_MergeAllParts():
    assert pathlib.Path("./scripts/bfut_MergeAllParts.py").exists()
    assert filepath_fce_input.exists()
    p = subprocess.run(
        f"{PYTHON_EXECUTABLE} ./scripts/bfut_MergeAllParts.py {str(filepath_fce_input)} {str(filepath_mergeallparts_output)}",
        shell=True, capture_output=True, encoding="utf8", check=True)
    # outpath = filepath_fce_input.parent / (filepath_fce_input.stem + "_out" + filepath_fce_input.suffix)
    outpath = filepath_mergeallparts_output
    assert pathlib.Path("./tests/fce/test_scripts_MergeAllParts_4_Snowman_car.fce").exists()
    assert outpath.exists()
    p_cmp = subprocess.run(f"cmp ./tests/fce/test_scripts_MergeAllParts_4_Snowman_car.fce {str(outpath)}", shell=True, check=True)
    p_cksum = subprocess.run(f"cksum ./tests/fce/test_scripts_MergeAllParts_4_Snowman_car.fce {str(outpath)}", shell=True, check=True)
    print(p_cksum.stdout)
    print(p_cksum.stderr)
    if p_cksum.returncode != 0:
        print(p_cksum.stderr)
    outpath.unlink()
    assert p.returncode == 0
    assert len(p.stderr) == 0
    assert p_cmp.returncode == 0


def test_PrintFceInfo():
    fcecodec_PrintFceInfo_src = \
"""Filesize = 46556 (0xb5dc)
Version = FCE4
NumTriangles = 236 (* 12 = 2832) (* 56 = 13216)
NumVertices = 159 (* 4 = 636)  (* 12 = 1908)  (* 32 = 5088)
NumArts = 1
XHalfSize = 0.484685
YHalfSize = 0.751403
ZHalfSize = 1.305500
NumParts = 5
NumDummies = 0
NumColors = 8
VertTblOffset = 0x0000 (0x2038), Size = 1908
NormTblOffset = 0x0774 (0x27ac), Size = 1908
TriaTblOffset = 0x0ee8 (0x2f20), Size = 13216
Reserve1offset = 0x4288 (0x62c0), Size = 5088
Reserve2offset = 0x5668 (0x76a0), Size = 1908
Reserve3offset = 0x5ddc (0x7e14), Size = 1908
UndamgdVertTblOffset = 0x6550 (0x8588), Size = 1908
UndamgdNormTblOffset = 0x6cc4 (0x8cfc), Size = 1908
DamgdVertTblOffset = 0x7438 (0x9470), Size = 1908
DamgdNormTblOffset = 0x7bac (0x9be4), Size = 1908
Reserve4offset = 0x8320 (0xa358), Size = 636
AnimationTblOffset = 0x859c (0xa5d4), Size = 636
Reserve5offset = 0x8818 (0xa850), Size = 636
Reserve6offset = 0x8a94 (0xaacc), Size = 2832
Unknown1 (0x0004) = 0 (0x0000)
Unknown3 (0x0924) = 0 (0x0000)
Parts:
Idx  Verts       Triangles   (PartPos)                         Name
  0      0     4     0     2 (-0.001832, -0.579950, -1.036170) :HLRW
  1      4     4     2     2 (-0.001832, -0.579950, -1.036170) :HRRW
  2      8     4     4     2 ( 0.004065, -0.579950,  0.786859) :HLFW
  3     12   143     6   228 ( 0.000000,  0.000000, -0.000000) :HB
  4    155     4   234     2 ( 0.004065, -0.579950,  0.786859) :HRFW
         =   159     =   236
FCE4 Filesize (verts, triags) = 46556 (0xb5dc), diff=0
FCE4M Filesize (verts, triags) = 46715 (0xb67b), diff=-159
DummyNames (Position):
Car colors (hue, saturation, brightness, transparency):
 0  Primary     170, 154,  91, 128
 0  Interior    255, 152, 159, 128
 0  Secondary    25, 123, 114, 127
 0  Driver hair 255, 152, 159, 127
 1  Primary      33, 115, 181, 127
 1  Interior    170,  65, 128, 128
 1  Secondary   255,   9,  26, 127
 1  Driver hair 170,  65, 128, 127
 2  Primary      18, 254, 255,   0
 2  Interior    212,   2,  96, 128
 2  Secondary   255,   9,  26, 127
 2  Driver hair 212,   2,  96, 127
 3  Primary      91, 213,  49, 127
 3  Interior    170,  21, 175, 128
 3  Secondary   255,   9,  26, 127
 3  Driver hair 170,  21, 175, 127
 4  Primary     255, 254, 165, 127
 4  Interior    170,  65, 128, 128
 4  Secondary    25, 123, 114, 127
 4  Driver hair 170,  65, 128, 127
 5  Primary     170, 184, 255,   0
 5  Interior     27, 127, 192, 128
 5  Secondary   255,   9,  26, 127
 5  Driver hair  27, 127, 192, 127
 6  Primary     255, 254,   0,   0
 6  Interior    170,  21, 175, 128
 6  Secondary    24, 123, 114, 127
 6  Driver hair 170,  21, 175, 127
 7  Primary     255,   1, 255,   0
 7  Interior    255, 152, 159, 128
 7  Secondary   255,   9,  26, 127
 7  Driver hair 170,  65, 128, 127
"""
    assert pathlib.Path("./scripts/bfut_PrintFceInfo.py").exists()
    assert filepath_fce_input.exists()
    p = subprocess.run(f"{PYTHON_EXECUTABLE} ./scripts/bfut_PrintFceInfo.py {str(filepath_fce_input)}",
                       shell=True, capture_output=True, encoding="utf8", check=True)
    assert p.returncode == 0 and len(p.stderr) == 0 and p.stdout == fcecodec_PrintFceInfo_src
