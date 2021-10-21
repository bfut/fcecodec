# fcecodec
`fcecodec` decodes, encodes, and modifies FCE data. FCE is a geometry definition file format.

As Python module, allows using modern data analysis methods: export / import raw geometry data; modify attributes.

The library itself is written in C. Python bindings are written in C++ (pybind).

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

with open(filepath_fce_input, "rb") as f:
    fce_buf = f.read()

# Print FCE stats to console
fcecodec.print_fce_info(fce_buf)

# Create mesh object
mesh = fcecodec.Mesh()

# Load FCE data to mesh object
mesh.decode(fce_buf)
assert(mesh.valid() == 1)

# Merge parts to new part
idx1 = 0
idx2 = 3
new_idx = mesh.merge_parts(idx1, idx2)
assert(new_idx != -1)

# Print stats
mesh.info()
print(mesh.num_parts)
print(mesh.num_triags)
print(mesh.num_verts)

# Encode to FCE3
out_buf = mesh.encode_fce3()

# Write to file
with open(path, "wb") as f:
    f.write(out_buf)
```

## References
FCE3 specifications taken from [1]. FCE4 specifications partially adapted from [1] and [2]. Thank you.

[1] D. Auroux et al. [_The unofficial Need For Speed III file format specifications - Version 1.0_](/references/unofficial_nfs3_file_specs_10.txt) [1998]<br/>
[2] A. Sadhra [_NFS4Loader.h_](/references/OpenNFS/NFS4Loader.h) via [_OpenNFS_](https://github.com/OpenNFS) [2015]<br/>

## Information
__License:__ GNU General Public License v3.0+<br/>
__Website:__ <https://github.com/bfut/fcecodec>
