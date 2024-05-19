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
  test_smoketest.py - smoke-testing fcecodec Python extension module
"""
import os
import pathlib
import platform
import re
import sys

import pytest

# Look for local build, if not installed
try:
    import fcecodec as fc
except ModuleNotFoundError:
    import sys
    PATH = pathlib.Path(pathlib.Path(__file__).parent / "../python/build")
    print(PATH)
    for x in PATH.glob("**"):
        sys.path.append(str(x.resolve()))
    del PATH

    import fcecodec as fc


# --------------------------------------
script_path = pathlib.Path(__file__).parent
filepath_fce_input = script_path / "fce/Snowman_car.fce"
filepath_fce3_output = script_path / ".out/ci-smoketest3.fce"
filepath_fce4_output = script_path / ".out/ci-smoketest4.fce"
filepath_fce4m_output = script_path / ".out/ci-smoketest4m.fce"

filepath_obj_output = script_path / ".out/ci-smoketest.obj"
filepath_mtl_output = script_path / ".out/ci-smoketest.mtl"
objtexname = "car00_Snowman.png"

#
if (script_path / ".out").exists() and not (script_path / ".out").is_dir():
    os.remove(script_path / ".out")
    os.mkdir(script_path / ".out")
elif not (script_path / ".out").exists():
    os.mkdir(script_path / ".out")

del script_path


# -------------------------------------- import python wrappers
sys.path.append(str((pathlib.Path(__file__).parent / "../python/").resolve()))
from bfut_mywrappers import *


if platform.python_implementation() != "PyPy":
    # tracemalloc -- BEGIN ---------------------------------------------------------
    # Source: https://docs.python.org/3/library/tracemalloc.html
    import linecache
    import os
    import tracemalloc

    def display_top(snapshot, key_type="lineno", limit=10):
        snapshot = snapshot.filter_traces((
            tracemalloc.Filter(False, "<frozen importlib._bootstrap>"),
            tracemalloc.Filter(False, "<unknown>"),
        ))
        top_stats = snapshot.statistics(key_type)

        print(f"Top {limit} lines")
        for index, stat in enumerate(top_stats[:limit], 1):
            frame = stat.traceback[0]
            print(f"#{index}: {frame.filename}:{frame.lineno}: "
                f"{(stat.size / 1024):.1f} KiB "
                f"({stat.size})")
            line = linecache.getline(frame.filename, frame.lineno).strip()
            if line:
                print(f"    {line}")

        other = top_stats[limit:]
        if other:
            size = sum(stat.size for stat in other)
            print(f"{len(other)} other: {(size / 1024):.1f} KiB" ,
                f"({size})")

        total = sum(stat.size for stat in top_stats)
        print(f"Total allocated size: {(total / 1024):.1f} KiB ({total})")
    # tracemalloc -- END -----------------------------------------------------------


@pytest.mark.skipif(platform.python_implementation() == "PyPy",
                    reason="'pypy-3.8' no tracemalloc")
def test_smoketest_tracemalloc():
    # tracemalloc -- BEGIN ---------------------------------------------------------
    # tracemalloc.stop()
    first_size, first_peak = tracemalloc.get_traced_memory()

    if sys.version_info[0:2] >= (3, 9):
        tracemalloc.reset_peak()

    tracemalloc.start()
    # tracemalloc -- END -----------------------------------------------------------


    # -------------------------------------- smoketest
    print(flush = True)
    print(f"version: {GetFceVersion(filepath_fce_input)}")
    # PrintFceInfo(filepath_fce_input)

    mesh = fc.Mesh()
    print(mesh, type(mesh))
    print(flush = True)

    mesh = LoadFce(mesh, filepath_fce_input)
    # mesh.PrintInfo()
    print(flush = True)

    WriteFce("3", mesh, filepath_fce3_output)
    WriteFce("4", mesh, filepath_fce4_output)
    WriteFce("4M", mesh, filepath_fce4m_output)
    ExportObj(mesh,
            filepath_obj_output, filepath_mtl_output, objtexname,
            print_damage=0, print_dummies=0,
            use_part_positions=1, print_part_positions=0)
    del mesh
    print(flush = True)

    PrintFceInfo(filepath_fce3_output)
    PrintFceInfo(filepath_fce4_output)
    PrintFceInfo(filepath_fce4m_output)

    GetFceVersion(filepath_fce3_output)
    GetFceVersion(filepath_fce4_output)
    GetFceVersion(filepath_fce4m_output)

    # tracemalloc -- BEGIN ---------------------------------------------------------
    # tracemalloc.stop()
    second_size, second_peak = tracemalloc.get_traced_memory()
    # tracemalloc.start()

    snapshot = tracemalloc.take_snapshot()
    display_top(snapshot, limit=40)

    print(f"first_size={first_size}", f"first_peak={first_peak}")
    print(f"second_size={second_size}", f"second_peak={second_peak}")
    print(flush = True)
    # tracemalloc -- END -----------------------------------------------------------

    assert first_size == 0 and second_size < 55000 and second_peak < 130000


@pytest.mark.skipif(platform.python_implementation() != "PyPy",
                   reason="test_SmoketestNoTracemalloc 'pypy-3.8' no tracemalloc")
def test_smoketest_no_tracemalloc():
    # -------------------------------------- smoketest
    print(flush = True)
    print(f"version: {GetFceVersion(filepath_fce_input)}")
    # PrintFceInfo(filepath_fce_input)

    mesh = fc.Mesh()
    print(mesh, type(mesh))
    print(flush = True)

    mesh = LoadFce(mesh, filepath_fce_input)
    # mesh.PrintInfo()
    print(flush = True)

    WriteFce("3", mesh, filepath_fce3_output)
    WriteFce("4", mesh, filepath_fce4_output)
    WriteFce("4M", mesh, filepath_fce4m_output)
    ExportObj(mesh,
            filepath_obj_output, filepath_mtl_output, objtexname,
            print_damage=0, print_dummies=0,
            use_part_positions=1, print_part_positions=0)
    del mesh
    print(flush = True)

    PrintFceInfo(filepath_fce3_output)
    PrintFceInfo(filepath_fce4_output)
    PrintFceInfo(filepath_fce4m_output)

    GetFceVersion(filepath_fce3_output)
    GetFceVersion(filepath_fce4_output)
    GetFceVersion(filepath_fce4m_output)


@pytest.mark.parametrize("vers, path",
    [ (3, filepath_fce3_output),
      (4, filepath_fce4_output),
      (5, filepath_fce4m_output) ])
def test_FceVersion(vers, path):
    print(vers, path)
    assert GetFceVersion(path) == vers


# fcecodec_GetFceVersion_3 = \
# """Filesize = 33876 (0x8454)
# Version = FCE3
# NumTriangles = 236 (* 56 = 13216)
# NumVertices = 159 (* 12 = 1908)  (* 32 = 5088)
# NumArts = 1
# XHalfSize = 0.484685
# YHalfSize = 0.731403
# ZHalfSize = 1.305500
# NumParts = 5
# NumDummies = 0
# NumPriColors = 8
# NumSecColors = 8
# VertTblOffset = 0x0000 (0x1f04), Size = 1908
# NormTblOffset = 0x0774 (0x2678), Size = 1908
# TriaTblOffset = 0x0ee8 (0x2dec), Size = 13216
# Reserve1offset = 0x4288 (0x618c), Size = 5088
# Reserve2offset = 0x5668 (0x756c), Size = 1908
# Reserve3offset = 0x5ddc (0x7ce0), Size = 1908
# Unknown1 (0x0004) = 0 (0x0000)
# Parts:
# Idx  Verts       Triags      (PartPos)                         Description          Name
#   0      0     4     0     2 (-0.001832, -0.579950, -1.036170)            high body :HLRW
#   1      4     4     2     2 (-0.001832, -0.579950, -1.036170)     left front wheel :HRRW
#   2      8     4     4     2 ( 0.004065, -0.579950,  0.786859)    right front wheel :HLFW
#   3     12   143     6   228 ( 0.000000,  0.000000,  0.000000)      left rear wheel :HB
#   4    155     4   234     2 ( 0.004065, -0.579950,  0.786859)     right rear wheel :HRFW
#          =   159     =   236
# Filesize (verts, triags) = 33876 (0x8454), diff=0
# DummyNames (Position):
# Car colors (hue, saturation, brightness, transparency):
#  0  Primary     170, 154,  91, 128
#  0  Secondary    25, 123, 114, 127
#  1  Primary      33, 115, 181, 127
#  1  Secondary   255,   9,  26, 127
#  2  Primary      18, 254, 255,   0
#  2  Secondary   255,   9,  26, 127
#  3  Primary      91, 213,  49, 127
#  3  Secondary   255,   9,  26, 127
#  4  Primary     255, 254, 165, 127
#  4  Secondary    25, 123, 114, 127
#  5  Primary     170, 184, 255,   0
#  5  Secondary   255,   9,  26, 127
#  6  Primary     255, 254,   0,   0
#  6  Secondary    24, 123, 114, 127
#  7  Primary     255,   1, 255,   0
#  7  Secondary   255,   9,  26, 127
# """

# fcecodec_GetFceVersion_4 = \
# """Filesize = 46556 (0xb5dc)
# Version = FCE4
# NumTriangles = 236 (* 12 = 2832) (* 56 = 13216)
# NumVertices = 159 (* 4 = 636)  (* 12 = 1908)  (* 32 = 5088)
# NumArts = 1
# XHalfSize = 0.484685
# YHalfSize = 0.731403
# ZHalfSize = 1.305500
# NumParts = 5
# NumDummies = 0
# NumColors = 8
# VertTblOffset = 0x0000 (0x2038), Size = 1908
# NormTblOffset = 0x0774 (0x27ac), Size = 1908
# TriaTblOffset = 0x0ee8 (0x2f20), Size = 13216
# Reserve1offset = 0x4288 (0x62c0), Size = 5088
# Reserve2offset = 0x5668 (0x76a0), Size = 1908
# Reserve3offset = 0x5ddc (0x7e14), Size = 1908
# UndamgdVertTblOffset = 0x6550 (0x8588), Size = 1908
# UndamgdNormTblOffset = 0x6cc4 (0x8cfc), Size = 1908
# DamgdVertTblOffset = 0x7438 (0x9470), Size = 1908
# DamgdNormTblOffset = 0x7bac (0x9be4), Size = 1908
# Reserve4offset = 0x8320 (0xa358), Size = 636
# AnimationTblOffset = 0x859c (0xa5d4), Size = 636
# Reserve5offset = 0x8818 (0xa850), Size = 636
# Reserve6offset = 0x8a94 (0xaacc), Size = 2832
# Unknown1 (0x0004) = 0 (0x0000)
# Unknown3 (0x0924) = 0 (0x0000)
# Parts:
# Idx  Verts       Triangles   (PartPos)                         Name
#   0      0     4     0     2 (-0.001832, -0.579950, -1.036170) :HLRW
#   1      4     4     2     2 (-0.001832, -0.579950, -1.036170) :HRRW
#   2      8     4     4     2 ( 0.004065, -0.579950,  0.786859) :HLFW
#   3     12   143     6   228 ( 0.000000,  0.000000,  0.000000) :HB
#   4    155     4   234     2 ( 0.004065, -0.579950,  0.786859) :HRFW
#          =   159     =   236
# FCE4 Filesize (verts, triags) = 46556 (0xb5dc), diff=0
# FCE4M Filesize (verts, triags) = 46715 (0xb67b), diff=-159
# DummyNames (Position):
# Car colors (hue, saturation, brightness, transparency):
#  0  Primary     170, 154,  91, 128
#  0  Interior    255, 152, 159, 128
#  0  Secondary    25, 123, 114, 127
#  0  Driver hair 255, 152, 159, 127
#  1  Primary      33, 115, 181, 127
#  1  Interior    170,  65, 128, 128
#  1  Secondary   255,   9,  26, 127
#  1  Driver hair 170,  65, 128, 127
#  2  Primary      18, 254, 255,   0
#  2  Interior    212,   2,  96, 128
#  2  Secondary   255,   9,  26, 127
#  2  Driver hair 212,   2,  96, 127
#  3  Primary      91, 213,  49, 127
#  3  Interior    170,  21, 175, 128
#  3  Secondary   255,   9,  26, 127
#  3  Driver hair 170,  21, 175, 127
#  4  Primary     255, 254, 165, 127
#  4  Interior    170,  65, 128, 128
#  4  Secondary    25, 123, 114, 127
#  4  Driver hair 170,  65, 128, 127
#  5  Primary     170, 184, 255,   0
#  5  Interior     27, 127, 192, 128
#  5  Secondary   255,   9,  26, 127
#  5  Driver hair  27, 127, 192, 127
#  6  Primary     255, 254,   0,   0
#  6  Interior    170,  21, 175, 128
#  6  Secondary    24, 123, 114, 127
#  6  Driver hair 170,  21, 175, 127
#  7  Primary     255,   1, 255,   0
#  7  Interior    255, 152, 159, 128
#  7  Secondary   255,   9,  26, 127
#  7  Driver hair 170,  65, 128, 127
# """

# fcecodec_GetFceVersion_4m = \
# """Filesize = 46715 (0xb67b)
# Version = FCE4M
# NumTriangles = 236 (* 12 = 2832) (* 56 = 13216)
# NumVertices = 159 (* 4 = 636)  (* 12 = 1908)  (* 32 = 5088)
# NumArts = 1
# XHalfSize = 0.484685
# YHalfSize = 0.731403
# ZHalfSize = 1.305500
# NumParts = 5
# NumDummies = 0
# NumColors = 8
# VertTblOffset = 0x0000 (0x2038), Size = 1908
# NormTblOffset = 0x0774 (0x27ac), Size = 1908
# TriaTblOffset = 0x0ee8 (0x2f20), Size = 13216
# Reserve1offset = 0x4288 (0x62c0), Size = 5088
# Reserve2offset = 0x5668 (0x76a0), Size = 1908
# Reserve3offset = 0x5ddc (0x7e14), Size = 1908
# UndamgdVertTblOffset = 0x6550 (0x8588), Size = 1908
# UndamgdNormTblOffset = 0x6cc4 (0x8cfc), Size = 1908
# DamgdVertTblOffset = 0x7438 (0x9470), Size = 1908
# DamgdNormTblOffset = 0x7bac (0x9be4), Size = 1908
# Reserve4offset = 0x8320 (0xa358), Size = 636
# AnimationTblOffset = 0x859c (0xa5d4), Size = 636
# Reserve5offset = 0x8818 (0xa850), Size = 636
# Reserve6offset = 0x8a94 (0xaacc), Size = 2991
# Unknown1 (0x0004) = 0 (0x0000)
# Unknown3 (0x0924) = 0 (0x0000)
# Parts:
# Idx  Verts       Triangles   (PartPos)                         Name
#   0      0     4     0     2 (-0.001832, -0.579950, -1.036170) :HLRW
#   1      4     4     2     2 (-0.001832, -0.579950, -1.036170) :HRRW
#   2      8     4     4     2 ( 0.004065, -0.579950,  0.786859) :HLFW
#   3     12   143     6   228 ( 0.000000,  0.000000,  0.000000) :HB
#   4    155     4   234     2 ( 0.004065, -0.579950,  0.786859) :HRFW
#          =   159     =   236
# FCE4 Filesize (verts, triags) = 46556 (0xb5dc), diff=159
# FCE4M Filesize (verts, triags) = 46715 (0xb67b), diff=0
# DummyNames (Position):
# Car colors (hue, saturation, brightness, transparency):
#  0  Primary     170, 154,  91, 128
#  0  Interior    255, 152, 159, 128
#  0  Secondary    25, 123, 114, 127
#  0  Driver hair 255, 152, 159, 127
#  1  Primary      33, 115, 181, 127
#  1  Interior    170,  65, 128, 128
#  1  Secondary   255,   9,  26, 127
#  1  Driver hair 170,  65, 128, 127
#  2  Primary      18, 254, 255,   0
#  2  Interior    212,   2,  96, 128
#  2  Secondary   255,   9,  26, 127
#  2  Driver hair 212,   2,  96, 127
#  3  Primary      91, 213,  49, 127
#  3  Interior    170,  21, 175, 128
#  3  Secondary   255,   9,  26, 127
#  3  Driver hair 170,  21, 175, 127
#  4  Primary     255, 254, 165, 127
#  4  Interior    170,  65, 128, 128
#  4  Secondary    25, 123, 114, 127
#  4  Driver hair 170,  65, 128, 127
#  5  Primary     170, 184, 255,   0
#  5  Interior     27, 127, 192, 128
#  5  Secondary   255,   9,  26, 127
#  5  Driver hair  27, 127, 192, 127
#  6  Primary     255, 254,   0,   0
#  6  Interior    170,  21, 175, 128
#  6  Secondary    24, 123, 114, 127
#  6  Driver hair 170,  21, 175, 127
#  7  Primary     255,   1, 255,   0
#  7  Interior    255, 152, 159, 128
#  7  Secondary   255,   9,  26, 127
#  7  Driver hair 170,  65, 128, 127
# """

# @pytest.mark.xfail(sys.platform.startswith("win"),
#                    reason="test_PrintFceInfo() wurlitzer requires fcntl, n/a on windows")
# @pytest.mark.parametrize("vers, path",
#     [ (fcecodec_GetFceVersion_3, filepath_fce3_output),
#       (fcecodec_GetFceVersion_4, filepath_fce4_output),
#       (fcecodec_GetFceVersion_4m, filepath_fce4m_output) ])
# def test_PrintFceInfo(vers, path):
#     import wurlitzer
#     with wurlitzer.pipes() as (out, err):
#         fc.PrintFceInfo(path.read_bytes())
#     stdout = out.read()
#     assert len(err.read()) == 0 and stdout == vers

def test_version():
    script_path = pathlib.Path(__file__).parent.resolve()
    __version__ = re.findall(
        r"#define FCECVERS \"(.*)\"",
        (script_path / "../src/fcelib/fcelib.h").read_text("utf-8")
    )[0]
    print(f"VERSION_INFO={__version__}")
    if hasattr(fc, "__version__"):
        print(f"fc.__version__={fc.__version__}")
    else:
        print(f'hasattr(fc, "__version__")={hasattr(fc, "__version__")}')
    assert hasattr(fc, "__version__") and fc.__version__ == __version__
