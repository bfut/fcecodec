# fcecodec
`fcecodec` is a Python extension module that decodes, encodes, and operates on
(modifies) FCE data. FCE is a geometry definition file format.

With Python, this allows importing / exporting raw geometry data using modern
methods.

The Python extension module is based on a header-only library written in C89.
Python bindings are written in C++ (pybind11).

## Features
* full FCE implementation
* Io: supported format versions: FCE3, FCE4, FCE4M
* Io: transparently decodes/encodes
* Io: cleanly encodes FCE binary data from the ground up
* Io: imports raw geometry data
* Io: exports to Wavefront OBJ
* Io: optionally center parts on FCE encoding
* Get/Set: exposes raw geometry data (vertices, normals, triangles, texcoords)
* Get/Set: exposes attributes (vertex animation flags, triangle flags, colors, dummies (fx/light objects), etc.)
* Operation: inserts part from another mesh
* Operations: changes part order, copies part, merges parts, deletes part
* Operation: deletes triangles, vertices
* Stats: prints Mesh info (e.g., number of triangles & vertices, colors, part positions, etc.)
* Stats: prints FCE binary data info (e.g., format version, half sizes, number of triangles & vertices, colors, part positions, etc.)
* Validate: validates FCE binary data
* Python: numpy integration for most functions

## Usage
The main purpose of this software is ease of use.
Ready-to-use scripts can be found in [/scripts](/scripts)

There is a tutorial for converting OBJ/MTL files to FCE at
[/scripts/doc_Obj2Fce.MD](/scripts/doc_Obj2Fce.MD)

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