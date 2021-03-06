# fcecodec Copyright (C) 2021-2022 Benjamin Futasz <https://github.com/bfut>
#
# You may not redistribute this program without its source code.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
"""
setup.py - adapted from https://github.com/pybind/python_example/blob/master/setup.py
"""
import os
import platform
import pathlib
import setuptools

# Available at setup time due to pyproject.toml
from pybind11.setup_helpers import Pybind11Extension, build_ext


# ------------------------------------------------------------------------------

if os.environ.get("FCECVERBOSE") is not None:
    print(f'FCECVERBOSE={os.environ["FCECVERBOSE"]}')

script_path = pathlib.Path(__file__).parent.resolve()
os.chdir(script_path)
with open(script_path / "../src/fcelib/fcelib.h", mode="r", encoding="utf8") as f:
    for _ in range(33):
        next(f)
    __version__ = f.readline().rstrip().split("\"")[-2]
    print(f"VERSION_INFO={__version__}")

long_description = (script_path / "../README.md").read_text(encoding="utf-8")
extra_compile_args = [
    ("-DPYMEM_MALLOC"),
]
if platform.system() == "Windows":
    extra_compile_args += [
        ("/wd4267")  # prevents warnings on conversion from size_t to int
    ]
else:
    extra_compile_args += [
        # # debug
        # ("-g"),
        ("-pedantic-errors"),
    ]

    if "gcc" in platform.python_compiler().lower():
        extra_compile_args += [
            ("-fasynchronous-unwind-tables"),
            ("-Wall"),
            ("-Wextra"),
            ("-Wstack-protector"),
            ("-Woverlength-strings"),
            ("-Wpointer-arith"),
            ("-Wunused-local-typedefs"),
            ("-Wunused-result"),
            ("-Wvarargs"),
            ("-Wvla"),
            ("-Wwrite-strings"),

            # # https://developers.redhat.com/blog/2018/03/21/compiler-and-linker-flags-gcc
            # ("-D_FORTIFY_SOURCE=2"),
            # ("-D_GLIBCXX_ASSERTIONS"),
            # ("-fexceptions"),
            # ("-fplugin=annobin"),
            # ("-fstack-clash-protection"),
            # ("-fstack-protector-strong"),
            # ("-fcf-protection"),
            # ("-Werror=format-security"),
            # ("-Wl,-z,defs"),
            # ("-Wl,-z,now"),
            # ("-Wl,-z,relro"),
        ]
    elif "clang" in platform.python_compiler().lower():
        extra_compile_args += [
            # ("-Weverything"),
            ("-Wno-braced-scalar-init"),
            # ("-Wno-newline-eof"),
        ]

# The main interface is through Pybind11Extension.
# * You can add cxx_std=11/14/17, and then build_ext can be removed.
# * You can set include_pybind11=false to add the include directory yourself,
#   say from a submodule.
#
# Note:
#   Sort input source files if you glob sources to ensure bit-for-bit
#   reproducible builds (https://github.com/pybind/python_example/pull/53)

ext_modules = [
    Pybind11Extension(
        "fcecodec",
        sorted(["fcecodecmodule.cpp"]),
        # Example: passing in the version to the compiled code
        define_macros=[
            # https://github.com/pybind/python_example/issues/12
            # https://github.com/pybind/python_example/blob/master/src/main.cpp
            ("VERSION_INFO", __version__),
            ("FCECVERBOSE", os.environ.get("FCECVERBOSE", 0)),  # 0 if key not set
        ],
        extra_compile_args=extra_compile_args
    ),
]

setuptools.setup(
    name="fcecodec",
    version=__version__,
    author="Benjamin Futasz",
    url="https://github.com/bfut/fcecodec",
    license="GPLv3+",
    description="FCE decoder/encoder",
    long_description=long_description,
    long_description_content_type="text/markdown",
    ext_modules=ext_modules,
    python_requires=">=3.8",
    # extras_require={"test": "pytest"},
    # # Currently, build_ext only provides an optional "highest supported C++
    # # level" feature, but in the future it may provide more features.
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
)
