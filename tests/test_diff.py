"""
  test_diff.py -
"""
import os
import pathlib
import subprocess
import sys

import pytest

script_path = pathlib.Path(__file__).parent
filepath_fce_input = script_path / "fce/Snowman_car.fce"
filepath_fce3_output = script_path / ".out/test_diff3.fce"
filepath_fce4_output = script_path / ".out/test_diff4.fce"
filepath_fce4m_output = script_path / ".out/test_diff4m.fce"

filepath_obj_output = script_path / ".out/test_diff.obj"
filepath_mtl_output = script_path / ".out/test_diff.mtl"
objtexname = "car00_Snowman.png"

#
try:
    os.mkdir(script_path / ".out")
except FileExistsError:
    pass

#
sys.path.append(str( pathlib.Path(pathlib.Path(__file__).parent / "../python/").resolve()))
from bfut_mywrappers import *


# Generate src->X and src->X->Y
def GenerateData(version1, version2, filepath_fce_output):
    outpath = str(filepath_fce_output) + f"diff{version1}{version2}"
    mesh = fcecodec.Mesh()

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
    outpath = GenerateData(version1, version2, filepath_fce_output)
    p = subprocess.run(["cmp", str(filepath_fce_output), outpath])
    subprocess.run(["cksum", str(filepath_fce_output), outpath])
    print()
    assert p.returncode == 0


def test_diff_identical_src_4m_4_to_4():
    print("Compare src->4 with src->4m->4 (required identical)")
    outpath = str(filepath_fce4m_output) + f"diff4m4"
    mesh = fcecodec.Mesh()
    mesh = LoadFce(mesh, filepath_fce4m_output)
    WriteFce("4", mesh, outpath, center_parts=0)
    p = subprocess.run(["cmp", str(filepath_fce4_output), outpath])
    subprocess.run(["cksum", str(filepath_fce4_output), outpath])
    print()
    assert p.returncode == 0


def test_diff_different_src_to_4():
    print("Compare src with src->4 (expected different, src not fcecodec-encoded)")
    p = subprocess.run(["cmp", str(filepath_fce_input), filepath_fce4_output])
    subprocess.run(["cksum", str(filepath_fce_input), filepath_fce4_output])
    print()
    assert p.returncode == 1