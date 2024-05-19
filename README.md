# fcecodec
`fcecodec` is a Python extension module that decodes, encodes, and operates on
FCE data. FCE is a geometry definition file format.
The Python extension module is based on a dependency-free, header-only library written in C89.
Python bindings are written in C++ (pybind11). Supported on Windows and Linux.
Tested on macOS.

The intended usage is for transparent pre- and post-processing tasks specific to FCE.
Exporting OBJ is supported in library, importing OBJ is fully supported via script.
This modern FCE implementation was needed as vintage tools are generally closed source, unmaintainable and exclusive to Windows.
Given `fcecodec`, many previously manual chores can be automatized in simple Python scripts.

Blender Import/Export Add-on: [fcecodec_blender](https://github.com/bfut/fcecodec_blender)

## Installation / Documentation
Python extension module: [/python/README.md](/python/README.md)<br/>
FCE format documentation: [/src/fcelib/fcelib_fcetypes.h](/src/fcelib/fcelib_fcetypes.h)<br/>

## Usage
[/scripts](/scripts) contains ready-to-use scripts (Obj2Fce, Fce2Obj, etc.)<br/>
[/scripts/doc_Obj2Fce.md](/scripts/doc_Obj2Fce.md) is an OBJ/MTL to FCE
conversion tutorial

[fcecodec-example.ipynb](https://colab.research.google.com/github/bfut/notebooks/blob/main/fcecodec/fcecodec-example.ipynb)
[![fcecodec-example.ipynb](https://colab.research.google.com/assets/colab-badge.svg)](https://colab.research.google.com/github/bfut/notebooks/blob/main/fcecodec/fcecodec-example.ipynb)

## Key features
* Io: full FCE implementation (FCE3, FCE4, FCE4M) with validation
* Io: decodes/encodes transparently
* Io: exports to Wavefront OBJ
* Scripts: provides Python API
* Get/Set: exposes raw geometry data (vertices, normals, triangles, texcoords)
* Get/Set: exposes attributes (triangle flags, texpages, vert animation flags, colors, dummies, etc.)
* Op: inserts part from another mesh
* Op: changes part order, copies part, merges parts, deletes part
* Op: deletes triangles, vertices
* Stats: print stats

## References
FCE3 specifications taken from [1].
FCE4 specifications are loosely adapted from [1] and [2].
[2] was apparently based on [3].
FCE4M specifications own work.

[1] D. Auroux et al. [_The unofficial Need For Speed III file format specifications - Version 1.0_](/references/unofficial_nfs3_file_specs_10.txt) [1998]<br/>
[2] A. Sadhra [_NFS4Loader.h_](/references/OpenNFS/NFS4Loader.h) via _OpenNFS_ [2015]<br/>
[3] Addict [_NFS4 FCE file format specifications_](/references/nfs4_fce_spec_by_Addict.txt) [1999]<br/>

## Information
__fcecodec License:__ GNU General Public License v2.0+<br/>
__Website:__ <https://github.com/bfut/fcecodec>

Third party licenses

__sclpython.h:__ zlib License<br/>
__fcecodec scripts:__ zlib License
