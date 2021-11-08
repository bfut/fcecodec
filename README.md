# fcecodec
`fcecodec` is a Python extension module that decodes, encodes, and operates on
(modifies) FCE data. FCE is a geometry definition file format.

With Python, this allows importing / exporting raw geometry data using modern
methods.

The Python extension module is based on a header-only library written in C.
Python bindings are written in C++ (pybind11).

## Features
* full FCE implementation
* Python: numpy integration for most functions
* Io: transparently decodes / encodes / converts the following format versions: FCE3, FCE4, FCE4M
* Io: cleanly creates FCE binary data from the ground up
* Io: imports raw geometry data
* Io: exports to Wavefront OBJ
* Io: optionally center parts on FCE encoding
* Get/Set: exposes raw geometry data (vertices, normals, triangles, texcoords)
* Get/Set: exposes attributes (vertex animation flags, triangle flags, colors, dummies (fx/light objects), etc.)
* Operation: inserts part from another mesh
* Operation: copies part
* Operation: merges parts
* Operation: deletes part
* Operation: changes part order
* Operation: deletes triangles, vertices
* Stats: prints Mesh info (e.g., number of triangles & vertices, colors, part positions, etc.)
* Stats: prints FCE binary data info (e.g., format version, half sizes, number of triangles & vertices, colors, part positions, etc.)
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

# Merge parts 0, 3 to new part
new_pid = mesh.OpMergeParts(0, 3)
assert(new_pid != -1)

# Copy part 1
new_pid = mesh.OpCopyPart(1)
assert(new_pid != -1)

# Insert (copy) part 1 from mesh_src to mesh
with open(filepath_fce_input2, "rb") as f:
    fce_buf2 = f.read()
mesh_src = fcecodec.Mesh()
mesh_src.IoDecode(fce_buf2)
new_pid = mesh.OpInsertPart(mesh_src, 1)
assert(new_pid != -1)

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