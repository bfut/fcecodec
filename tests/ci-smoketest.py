# This file is distributed under: CC BY-SA 4.0 <https://creativecommons.org/licenses/by-sa/4.0/>

"""
  ci-smoketest.py - smoke-testing fcecodec module

  NOTE: module should be built with setup.py

  Show module help:
  python tests.py help
"""

import argparse
import os
import pathlib
import platform
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


# -----------------------------
filepath_fce_input = 'fce/Snowman_car.fce'
filepath_fce_output = 'fce/ci-smoketest_1.fce'

def load_mesh(mesh, path):
    print("load_mesh(", mesh, ",", path, ")")
    with open(path, "rb") as f:
        fce_buf = f.read()
    assert(fcecodec.fce_valid(fce_buf) == 1)
    assert(mesh.decode_fce(fce_buf) == 1)
    return mesh.valid()

# -----------------------------
help(fcecodec)

# -----------------------------
fcecodec.print_fce_info(filepath_fce_input)
print()

mesh = fcecodec.Mesh()
print(mesh, type(mesh))
load_mesh(mesh, filepath_fce_input)
mesh.info()
print()

mesh.encode_fce3(filepath_fce_output)
del mesh
print()

fcecodec.print_fce_info(filepath_fce_output)

print("EOF ci-smoketest.py")
