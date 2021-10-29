# fcecodec
`fcecodec` is a Python extension module that decodes, encodes, and operates on
(modifies) FCE data. FCE is a geometry definition file format.

This allows importing / exporting raw geometry data, etc. using modern methods.

The Python extension module is based on a header-only library written in C.
Python bindings are written in C++ (pybind11).

## Features
* Python: numpy integration for most functions
* Io: decodes / encodes / converts the following format versions: FCE3, FCE4, FCE4M
* Io: cleanly creates new FCE from raw geometry data
* Io: extracts raw geometry data from FCE (triangles, vertices, normals, texcoords)
* Io: exports to Wavefront OBJ
* Io: center parts on FCE encoding
* Operations: merges parts
* Operations: deletes parts
* Operations: changes part order
* Operations: deletes triangles, vertices
* Exposes attributes, e.g., triangle flags, vertex animation flags, colors, dummies (fx/light objects), etc.
* Stats: prints format version, number of triangles and vertices, colors, part positions, etc.

## Installation / Documentation
Python extension module: [_/python/README.md_](/python/README.md)<br/>
Extensive FCE format specifications in comments: [_/src/fcelib/fcelib_fcetypes.h_](/src/fcelib/fcelib_fcetypes.h)<br/>

## Example
```py
import fcecodec

filepath_fce_input = 'path/to/car_src.fce'
filepath_fce_output = 'path/to/car.fce'

with open(filepath_fce_input, "rb") as f:
    fce_buf = f.read()

# Print FCE stats to console
fcecodec.PrintFceInfo(fce_buf)

# Create mesh object
mesh = fcecodec.Mesh()

# Load FCE data to mesh object
mesh.IoDecode(fce_buf)
assert(mesh.MValid() == 1)

# Merge parts to new part
idx1 = 0
idx2 = 3
new_idx = mesh.OpMergeParts(idx1, idx2)
assert(new_idx != -1)

# Stats
mesh.PrintInfo()
print(mesh.MNumParts)
print(mesh.MNumTriags)
print(mesh.MNumVerts)

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