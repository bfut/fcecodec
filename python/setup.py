from setuptools import setup

# Available at setup time due to pyproject.toml
from pybind11.setup_helpers import Pybind11Extension, build_ext
from pybind11 import get_cmake_dir

import sys

import os
import pathlib

script_path = pathlib.Path(__file__).parent.resolve()
os.chdir(script_path)

__version__ = "0.10+dev"
long_description = (script_path / "../README.md").read_text(encoding="utf-8")

# The main interface is through Pybind11Extension.
# * You can add cxx_std=11/14/17, and then build_ext can be removed.
# * You can set include_pybind11=false to add the include directory yourself,
#   say from a submodule.
#
# Note:
#   Sort input source files if you glob sources to ensure bit-for-bit
#   reproducible builds (https://github.com/pybind/python_example/pull/53)

ext_modules = [
    Pybind11Extension("fcecodec",
        sorted(["fcecodecmodule.cpp"]),
        # Example: passing in the version to the compiled code
        define_macros = [('VERSION_INFO', __version__)],
        ),
]

setup(
    name="fcecodec",
    version=__version__,
    author="Benjamin Futasz",
    url="https://github.com/bfut/fcecodec",
    license="GPLv3+",
    description="FCE decoder/encoder",
    long_description=long_description,
    long_description_content_type="text/markdown",
    ext_modules=ext_modules,
    extras_require={"test": "pytest"},
    # Currently, build_ext only provides an optional "highest supported C++
    # level" feature, but in the future it may provide more features.
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
)
