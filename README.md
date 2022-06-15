# fcecodec
`fcecodec` is a Python extension module that decodes, encodes, and operates on
(modifies) FCE data. FCE is a geometry definition file format.

[/scripts](/scripts) contains some ready-to-use scripts (Obj2Fce, Fce2Obj,
PrintFceInfo, etc.)<br/>
[/scripts/doc_Obj2Fce.MD](/scripts/doc_Obj2Fce.MD) is a tutorial for preparing
OBJ/MTL files for conversion to finished FCE

The Python extension module is based on a header-only library written in C89.
Python bindings are written in C++ (pybind11). Supported on Windows and Linux.
Tested on macOS.

## Example
[fcecodec-example.ipynb](https://colab.research.google.com/github/bfut/notebooks/blob/main/fcecodec/fcecodec-example.ipynb)
[![fcecodec-example.ipynb](https://colab.research.google.com/assets/colab-badge.svg)](https://colab.research.google.com/github/bfut/notebooks/blob/main/fcecodec/fcecodec-example.ipynb)

## Features
* full FCE implementation
* Io: supported format versions: FCE3, FCE4, FCE4M
* Io: transparently decodes/encodes
* Io: optionally center parts on FCE encoding
* Io: imports raw geometry data
* Io: exports to Wavefront OBJ
* Scripts: converts Wavefront OBJ to finished FCE
* Get/Set: exposes raw geometry data (vertices, normals, triangles, texcoords)
* Get/Set: exposes attributes (triangle flags, texpages, vert animation flags, colors, dummies, etc.)
* Operation: inserts part from another mesh
* Operations: changes part order, copies part, merges parts, deletes part
* Operation: deletes triangles, vertices
* Stats: prints Mesh info
* Stats: prints FCE binary data info
* Validate: validates FCE binary data
* Python: numpy integration for most functions

## Installation / Documentation
Python extension module: [_/python/README.md_](/python/README.md)<br/>
Extensive FCE format documentation in comments: [_/src/fcelib/fcelib_fcetypes.h_](/src/fcelib/fcelib_fcetypes.h)<br/>

## References
FCE3 specifications taken from [1]. FCE4 specifications partially adapted from
[1] and [2]. Thank you.

[1] D. Auroux et al. [_The unofficial Need For Speed III file format specifications - Version 1.0_](/references/unofficial_nfs3_file_specs_10.txt) [1998]<br/>
[2] A. Sadhra [_NFS4Loader.h_](/references/OpenNFS/NFS4Loader.h) via _OpenNFS_ [2015]<br/>

## Information
__fcecodec License:__ GNU General Public License v3.0+<br/>
__Website:__ <https://github.com/bfut/fcecodec>