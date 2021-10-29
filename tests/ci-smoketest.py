"""
  ci-smoketest.py - smoke-testing extension module

  This file is distributed under: CC BY-SA 4.0
      <https://creativecommons.org/licenses/by-sa/4.0/>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  This header may not be removed or altered from any source distribution.
"""

import argparse
import os
import pathlib
import platform
import sys
import time


# tracemalloc -- BEGIN ---------------------------------------------------------
# Source: https://docs.python.org/3/library/tracemalloc.html
import linecache
import os
import tracemalloc

def display_top(snapshot, key_type='lineno', limit=10):
    snapshot = snapshot.filter_traces((
        tracemalloc.Filter(False, "<frozen importlib._bootstrap>"),
        tracemalloc.Filter(False, "<unknown>"),
    ))
    top_stats = snapshot.statistics(key_type)

    print("Top %s lines" % limit)
    for index, stat in enumerate(top_stats[:limit], 1):
        frame = stat.traceback[0]
        print("#%s: %s:%s: %.1f KiB"
              % (index, frame.filename, frame.lineno, stat.size / 1024),
              "({:d})".format(stat.size))
        line = linecache.getline(frame.filename, frame.lineno).strip()
        if line:
            print('    %s' % line)

    other = top_stats[limit:]
    if other:
        size = sum(stat.size for stat in other)
        print("%s other: %.1f KiB" % (len(other), size / 1024),
              "({:d})".format(size))

    total = sum(stat.size for stat in top_stats)
    print("Total allocated size: %.1f KiB" % (total / 1024), "({:d})".format(total))

# tracemalloc -- END -----------------------------------------------------------

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

del script_path


# -------------------------------------- import python wrappers
sys.path.append(str( pathlib.Path(pathlib.Path(__file__).parent / "../python/").resolve()))
from fcecodec_mywrappers import *


# tracemalloc -- BEGIN ---------------------------------------------------------
# tracemalloc.stop()
first_size, first_peak = tracemalloc.get_traced_memory()

if sys.version_info[0:2] >= (3, 9):
    tracemalloc.reset_peak()

tracemalloc.start()
# tracemalloc -- END -----------------------------------------------------------


# -------------------------------------- smoketest
PrintFceInfo(filepath_fce_input)
print(flush = True)
mesh = fcecodec.Mesh()
print(mesh, type(mesh), flush = True)

start = time.time()
mesh = LoadFce(mesh, filepath_fce_input)
print(time.time() - start, flush=True)
mesh.PrintInfo()
print(flush = True)

start = time.time()
WriteFce(3, mesh, filepath_fce3_output)
print(time.time() - start, flush=True)
WriteFce(4, mesh, filepath_fce4_output)
WriteFce("4m", mesh, filepath_fce4m_output)
ExportObj(mesh,
          filepath_obj_output, filepath_mtl_output, objtexname,
          print_damage=0, print_dummies=0)
del mesh
print(flush = True)

PrintFceInfo(filepath_fce3_output)
PrintFceInfo(filepath_fce4_output)
PrintFceInfo(filepath_fce4m_output)

print("EOF ci-smoketest.py", flush = True)


# tracemalloc -- BEGIN ---------------------------------------------------------
# tracemalloc.stop()
second_size, second_peak = tracemalloc.get_traced_memory()
# tracemalloc.start()

snapshot = tracemalloc.take_snapshot()
display_top(snapshot, limit=40)

print("first_size={:d}".format(first_size), "first_peak={:d}".format(first_peak), flush = True)
print("second_size={:d}".format(second_size), "second_peak={:d}".format(second_peak), flush = True)

# tracemalloc -- END -----------------------------------------------------------