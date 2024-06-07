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
  test_diff.py -
"""
import os
import pathlib
import subprocess
import sys

import fcecodec as fc
import pytest

sys.path.append(str((pathlib.Path(__file__).parent / "../scripts/").resolve()))
from bfut_mywrappers import *  # fcecodec/scripts/bfut_mywrappers.py


SCRIPT_PATH = pathlib.Path(__file__).parent
filepath_fce_input = SCRIPT_PATH / "fce/Snowman_car.fce"
filepath_fce3_output = SCRIPT_PATH / ".out/test_diff3.fce"
filepath_fce4_output = SCRIPT_PATH / ".out/test_diff4.fce"
filepath_fce4m_output = SCRIPT_PATH / ".out/test_diff4m.fce"

filepath_obj_output = SCRIPT_PATH / ".out/test_diff.obj"
filepath_mtl_output = SCRIPT_PATH / ".out/test_diff.mtl"
objtexname = "car00_Snowman.png"

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


@pytest.mark.parametrize("version1, version2, filepath_fce_output",
    [ ("3", "3", filepath_fce3_output),
      ("4", "4", filepath_fce4_output),
      ("4m", "4m", filepath_fce4m_output) ])
def test_diff_identical_src_X_X(version1, version2, filepath_fce_output):
    print(f"Compare src->{version1} with src->{version1}->{version1} (required identical)")
    outpath = generate_data(version1, version2, filepath_fce_output)
    p = subprocess.run(["cmp", str(filepath_fce_output), outpath], check=False)
    subprocess.run(["cksum", str(filepath_fce_output), outpath], check=False)
    print()
    assert p.returncode == 0


def test_diff_identical_src_4m_4_to_4():
    print("Compare src->4 with src->4m->4 (required identical)")
    outpath = str(filepath_fce4m_output) + "diff4m4"
    mesh = fc.Mesh()
    mesh = LoadFce(mesh, filepath_fce4m_output)
    WriteFce("4", mesh, outpath, center_parts=0)
    p = subprocess.run(["cmp", str(filepath_fce4_output), outpath], check=False)
    subprocess.run(["cksum", str(filepath_fce4_output), outpath], check=False)
    print()
    assert p.returncode == 0


def test_diff_different_src_to_4():
    print("Compare src with src->4 (expected different, src not fcecodec-encoded)")
    p = subprocess.run(["cmp", str(filepath_fce_input), filepath_fce4_output], check=False)
    subprocess.run(["cksum", str(filepath_fce_input), filepath_fce4_output], check=False)
    print()
    assert p.returncode == 1
