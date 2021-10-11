# fcecodec
`fcecodec` decodes, encodes, and modifies FCE data. FCE is a geometry definition file format.

As Python module, allows using modern data analysis methods: export / import raw geometry data; modify attributes.

The library itself is written in C. Python bindings are written in C++ (pybind). Developed for Windows and Linux.

## Features
* decodes / encodes the following FCE format versions: FCE3, FCE4, FCE4M
* exports to OBJ
* Python: extracts / inserts raw geometry data (vertices, triangles)
* Python: modifies attributes such as triangle flags, colors, and dummies (FCE light objects)

## Documentation
Python extension module: [_/python/README.md_](/python/README.md)<br/>
FCE format specifications in comments: [_src/fcelib/fcelib_fcetypes.h_](/src/fcelib/fcelib_fcetypes.h)<br/>

## Example
```py
import fcecodec
import numpy as np

filepath_fce_input = 'path/to/car_src.fce'
filepath_fce_output = 'path/to/car.fce'

# Print FCE stats to console
fcecodec.print_fce_info(filepath_fce_input)

# Read source file to buf
with open(filepath_fce_input, "rb") as f:
    fce_buf = f.read()
    
# Create mesh object
mesh = fcecodec.Mesh()

# Load fce data to mesh object
assert(mesh.decode_fce( fce_buf ) == 1)
assert(mesh.valid() == 1)

# Merge parts to new part
idx1 = 0
idx2 = 3
new_idx = mesh.merge_parts(idx1, idx2)
assert(new_idx != -1)

# Copy specified part from mesh to new mesh (m2)
m2 = fcecodec.Mesh(mesh, new_idx)
assert(m2 != mesh)

# Copy specified part from given mesh to m2
retv = m2.copy_part(mesh, 2)
assert(retv != -1)

# Print stats
m2.info()
print(m2.num_parts())
print(m2.num_triags())
print(m2.num_verts())
```

## References
FCE3 specifications taken from [1]. FCE4 specifications mostly adapted from [1] and [2]. Thank you.

[1] D. Auroux et al. [_The unofficial Need For Speed III file format specifications - Version 1.0_](/references/unofficial_nfs3_file_specs_10.txt) [1998]<br/>
[2] A. Sadhra [_NFS4Loader.h_](/references/OpenNFS/NFS4Loader.h) via [_OpenNFS_](https://github.com/OpenNFS) [2015]<br/>

## Information
__License:__ GNU General Public License v3.0+<br/>
__Website:__ <https://github.com/bfut/fcecodec>
