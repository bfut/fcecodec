# fcecodec
`fcecodec` is a Python extension module that decodes, encodes, and operates on
(modifies) FCE data. FCE is a geometry definition file format.

This also allows importing / exporting raw geometry data, etc. using modern methods.

The Python extension module is based on a header-only library written in C.
Python bindings are written in C++ (pybind11).

## Features
* Python: numpy integration for most functions
* Io: decodes / encodes / converts the following format versions: FCE3, FCE4, FCE4M
* Io: cleanly creates new FCE from raw geometry data
* Io: exports to Wavefront OBJ
* Io: center parts on FCE encoding
* Get/Set: exposes raw geometry data from FCE (vertices, normals, triangles, texcoords, triangles flag, vertices animation flag)
* Get/Set: exposes attributes, e.g., triangle flags, vertex animation flags, colors, dummies (fx/light objects), etc.
* Operations: inserts part from another mesh
* Operations: copies part
* Operations: merges parts
* Operations: deletes part
* Operations: changes part order
* Operations: deletes triangles, vertices
* Stats: prints number of triangles and vertices, colors, part positions, etc.
* Stats: prints FCE binary data info (e.g., format version, number of triangles and vertices, colors, part positions, etc.)
* Validate: validates FCE binary data

## Installation / Documentation
Python extension module: [_/python/README.md_](/python/README.md)<br/>
Extensive FCE format specifications in comments: [_/src/fcelib/fcelib_fcetypes.h_](/src/fcelib/fcelib_fcetypes.h)<br/>

## Example
```py
import fcecodec

filepath_fce_input = 'path/to/car_src.fce'
filepath_fce_input2 = 'path/to/car_src2.fce'
filepath_fce_output = 'path/to/car.fce'

with open(filepath_fce_input, "rb") as f:
    fce_buf = f.read()

# Print FCE stats
fcecodec.PrintFceInfo(fce_buf)

# Create Mesh object
mesh = fcecodec.Mesh()

# Load FCE data to Mesh object
mesh.IoDecode(fce_buf)

# Print Mesh object stats
mesh.PrintInfo()
print(mesh.MNumParts)
print(mesh.MNumTriags)
print(mesh.MNumVerts)

# Validate Mesh object
assert(mesh.MValid() == 1)

# Merge parts to new part
idx1 = 0
idx2 = 3
new_idx = mesh.OpMergeParts(idx1, idx2)
assert(new_idx != -1)

# Copy part
idx = 0
new_idx = mesh.OpInsertPart(mesh, idx)
assert(new_idx != -1)

# Insert (copy) part from another mesh
with open(filepath_fce_input2, "rb") as f:
    fce_buf2 = f.read()
mesh_src = fcecodec.Mesh()
mesh_src.IoDecode(fce_buf2)
idx = 1
new_idx = mesh.OpInsertPart(mesh_src, idx)
assert(new_idx != -1)

# Encode to FCE4
out_buf = mesh.IoEncode_Fce4()
with open(path, "wb") as f:
    f.write(out_buf)
```

## References
FCE3 specifications taken from [1]. FCE4 specifications partially adapted from
[1] and [2]. Thank you.

[1] D. Auroux et al. [_The unofficial Need For Speed III file format specifications - Version 1.0_](/references/unofficial_nfs3_file_specs_10.txt) [1998]<br/>
[2] A. Sadhra [_NFS4Loader.h_](/references/OpenNFS/NFS4Loader.h) via _OpenNFS_ [2015]<br/>

## Information
__fcecodec License:__ GNU General Public License v3.0+<br/>
__Website:__ <https://github.com/bfut/fcecodec>