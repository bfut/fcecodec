<a title="'Python (Linux)' workflow Status" href="https://github.com/bfut/fcecodec/actions?query=workflow%3Alinux"><img alt="'linux' workflow Status" src="https://img.shields.io/github/workflow/status/bfut/fcecodec/linux?longCache=true&style=flat&label=Python%20&#40Linux&#41"></a>
<a title="'Python (macOS)' workflow Status" href="https://github.com/bfut/fcecodec/actions?query=workflow%3Amacos"><img alt="'macos' workflow Status" src="https://img.shields.io/github/workflow/status/bfut/fcecodec/macos?longCache=true&style=flat&label=Python%20&#40macOS&#41"></a>
<a title="'Python (Windows)' workflow Status" href="https://github.com/bfut/fcecodec/actions?query=workflow%3Awindows"><img alt="'windows' workflow Status" src="https://img.shields.io/github/workflow/status/bfut/fcecodec/windows?longCache=true&style=flat&label=Python%20&#40Windows&#41"></a>

# fcecodec
`fcecodec` is a Python extension module that decodes, encodes, and operates on
FCE data. FCE is a geometry definition file format.
The Python extension module is based on a header-only library written in C89.
Python bindings are written in C++ (pybind11). Supported on Windows and Linux.
Tested on macOS.

## Usage
[/scripts](/scripts) contains ready-to-use scripts (Obj2Fce, Fce2Obj, etc.)<br/>
[/scripts/doc_Obj2Fce.md](/scripts/doc_Obj2Fce.md) is an OBJ/MTL to FCE
conversion tutorial

[fcecodec-example.ipynb](https://colab.research.google.com/github/bfut/notebooks/blob/main/fcecodec/fcecodec-example.ipynb)
[![fcecodec-example.ipynb](https://colab.research.google.com/assets/colab-badge.svg)](https://colab.research.google.com/github/bfut/notebooks/blob/main/fcecodec/fcecodec-example.ipynb)

## Features
* Io: full FCE implementation (FCE3, FCE4, FCE4M)
* Io: decodes/encodes transparently
* Io: optionally centers parts on FCE encoding
* Io: imports raw geometry data
* Io: exports to Wavefront OBJ
* Io: validates FCE binary data
* Scripts: converts Wavefront OBJ to finished FCE
* Get/Set: exposes raw geometry data (vertices, normals, triangles, texcoords)
* Get/Set: exposes attributes (triangle flags, texpages, vert animation flags, colors, dummies, etc.)
* Operation: inserts part from another mesh
* Operations: changes part order, copies part, merges parts, deletes part
* Operations: deletes triangles, vertices
* Stats: prints Mesh info
* Stats: prints FCE binary data info

## Installation / Documentation
Python extension module: [/python/README.md](/python/README.md)<br/>
FCE format documentation: [/src/fcelib/fcelib_fcetypes.h](/src/fcelib/fcelib_fcetypes.h)<br/>

## References
FCE3 specifications taken from [1].
FCE4 specifications are loosely adapted from [1] and [2].
[2] was apparently based on [3].

[1] D. Auroux et al. [_The unofficial Need For Speed III file format specifications - Version 1.0_](/references/unofficial_nfs3_file_specs_10.txt) [1998]<br/>
[2] A. Sadhra [_NFS4Loader.h_](/references/OpenNFS/NFS4Loader.h) via _OpenNFS_ [2015]<br/>
[3] Addict [_NFS4 FCE file format specifications_](/references/nfs4_fce_spec_by_Addict.txt) [1999]<br/>

## Information
__fcecodec License:__ GNU General Public License v3.0+<br/>
__Website:__ <https://github.com/bfut/fcecodec>
