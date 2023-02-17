# fcecodec - Python extension module
This file describes installation, and usage of `fcecodec` as Python extension
module.

## Installation
Requires Python 3.8+

        cd ~
        python -m pip install --upgrade numpy pip wheel setuptools pybind11
        git clone https://github.com/bfut/fcecodec.git
        python -m pip install --verbose fcecodec/python

#### Windows
Install `Visual Studio` with the Python Development workload. For a detailed
description, see the _Prerequisites_ section on
[microsoft.com: C++ extensions for Python](https://docs.microsoft.com/en-us/visualstudio/python/working-with-c-cpp-python-in-visual-studio?view=vs-2019#prerequisites)

Installing `Anaconda` and `git` is recommended.

Once these prerequisites have been met, installation generally works as
described above.

## Examples
For a script template and handy function wrappers, see
[/scripts/fcecodecScriptTemplate.py](/scripts/fcecodecScriptTemplate.py) and
[/python/bfut_mywrappers.py](/python/bfut_mywrappers.py)

```py
import fcecodec

filepath_fce_input = "path/to/car_src.fce"
filepath_fce_input2 = "path/to/car_src2.fce"
filepath_fce_output = "path/to/car.fce"

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
assert mesh.MValid() == 1

# Merge parts 0, 3 to new part
new_pid = mesh.OpMergeParts(0, 3)
assert new_pid != -1

# Copy part 1
new_pid = mesh.OpCopyPart(1)
assert new_pid != -1

# Insert/copy part 1 from mesh2 to mesh
with open(filepath_fce_input2, "rb") as f:
    fce_buf2 = f.read()
mesh2 = fcecodec.Mesh()
mesh2.IoDecode(fce_buf2)
new_pid = mesh.OpInsertPart(mesh2, 1)
assert new_pid != -1

# Encode to FCE4
out_buf = mesh.IoEncode_Fce4()
with open(filepath_fce_output, "wb") as f:
    f.write(out_buf)
```

## Documentation
```
Help on module fcecodec:

NAME
    fcecodec - FCE decoder/encoder

CLASSES
    pybind11_builtins.pybind11_object(builtins.object)
        Mesh
    
    class Mesh(pybind11_builtins.pybind11_object)
     |  Method resolution order:
     |      Mesh
     |      pybind11_builtins.pybind11_object
     |      builtins.object
     |  
     |  Methods defined here:
     |  
     |  IoDecode(...)
     |      IoDecode(self: fcecodec.Mesh, arg0: str) -> None
     |  
     |  IoEncode_Fce3(...)
     |      IoEncode_Fce3(self: fcecodec.Mesh, center_parts: bool = True) -> bytes
     |  
     |  IoEncode_Fce4(...)
     |      IoEncode_Fce4(self: fcecodec.Mesh, center_parts: bool = True) -> bytes
     |  
     |  IoEncode_Fce4M(...)
     |      IoEncode_Fce4M(self: fcecodec.Mesh, center_parts: bool = True) -> bytes
     |  
     |  IoExportObj(...)
     |      IoExportObj(self: fcecodec.Mesh, objpath: str, mtlpath: str, texname: str, print_damage: int = 0, print_dummies: int = 0, use_part_positions: int = 1, print_part_positions: int = 0) -> None
     |  
     |  IoGeomDataToNewPart(...)
     |      IoGeomDataToNewPart(self: fcecodec.Mesh, vert_idxs: numpy.ndarray[numpy.int32], vert_texcoords: numpy.ndarray[numpy.float32], vert_pos: numpy.ndarray[numpy.float32], normals: numpy.ndarray[numpy.float32]) -> int
     |      
     |      vert_idxs: 012..., vert_texcoords: uuuvvv... , vert_pos: xyzxyzxyz..., normals: xyzxyzxyz...
     |  
     |  MGetColors(...)
     |      MGetColors(self: fcecodec.Mesh) -> buffer
     |  
     |  MGetDummyNames(...)
     |      MGetDummyNames(self: fcecodec.Mesh) -> List[str]
     |  
     |  MGetDummyPos(...)
     |      MGetDummyPos(self: fcecodec.Mesh) -> buffer
     |  
     |  MSetColors(...)
     |      MSetColors(self: fcecodec.Mesh, colors: numpy.ndarray[numpy.uint8]) -> None
     |      
     |      Expects shape=(N, 4, 4)
     |  
     |  MSetDummyNames(...)
     |      MSetDummyNames(self: fcecodec.Mesh, names: List[str]) -> None
     |  
     |  MSetDummyPos(...)
     |      MSetDummyPos(self: fcecodec.Mesh, positions: numpy.ndarray[numpy.float32]) -> None
     |      
     |      Expects shape (N*3, ) for N dummies
     |  
     |  MValid(...)
     |      MValid(self: fcecodec.Mesh) -> bool
     |  
     |  OpAddHelperPart(...)
     |      OpAddHelperPart(self: fcecodec.Mesh, name: str, new_center: numpy.ndarray[numpy.float32] = [0.0, 0.0, 0.0]) -> int
     |      
     |      Add diamond-shaped part at coordinate origin or at optionally given position.
     |  
     |  OpCenterPart(...)
     |      OpCenterPart(self: fcecodec.Mesh, pid: int) -> bool
     |      
     |      Center specified part to local centroid. Does not move part w.r.t. to global coordinates.
     |  
     |  OpCopyPart(...)
     |      OpCopyPart(self: fcecodec.Mesh, pid_src: int) -> int
     |      
     |      Copy specified part. Returns new part index.
     |  
     |  OpDelUnrefdVerts(...)
     |      OpDelUnrefdVerts(self: fcecodec.Mesh) -> bool
     |      
     |      Delete all vertices that are not referenced by any triangle.
     |  
     |  OpDeletePart(...)
     |      OpDeletePart(self: fcecodec.Mesh, pid: int) -> bool
     |  
     |  OpDeletePartTriags(...)
     |      OpDeletePartTriags(self: fcecodec.Mesh, pid: int, idxs: List[int]) -> bool
     |  
     |  OpInsertPart(...)
     |      OpInsertPart(self: fcecodec.Mesh, mesh_src: fcecodec.Mesh, pid_src: int) -> int
     |      
     |      Insert (copy) specified part from mesh_src. Returns new part index.
     |  
     |  OpMergeParts(...)
     |      OpMergeParts(self: fcecodec.Mesh, pid1: int, pid2: int) -> int
     |      
     |      Returns new part index.
     |  
     |  OpMovePart(...)
     |      OpMovePart(self: fcecodec.Mesh, pid: int) -> int
     |      
     |      Move up specified part towards order 0. Returns new part index.
     |  
     |  OpSetPartCenter(...)
     |      OpSetPartCenter(self: fcecodec.Mesh, pid: int, new_center: numpy.ndarray[numpy.float32]) -> bool
     |      
     |      Center specified part to given position. Does not move part w.r.t. to global coordinates.
     |  
     |  PGetName(...)
     |      PGetName(self: fcecodec.Mesh, pid: int) -> str
     |  
     |  PGetPos(...)
     |      PGetPos(self: fcecodec.Mesh, pid: int) -> buffer
     |  
     |  PGetTriagsFlags(...)
     |      PGetTriagsFlags(self: fcecodec.Mesh, pid: int) -> buffer
     |  
     |  PGetTriagsTexcoords(...)
     |      PGetTriagsTexcoords(self: fcecodec.Mesh, pid: int) -> buffer
     |      
     |      uuuvvv..., Returns (N*6, ) numpy array for N triangles.
     |  
     |  PGetTriagsTexpages(...)
     |      PGetTriagsTexpages(self: fcecodec.Mesh, pid: int) -> buffer
     |  
     |  PGetTriagsVidx(...)
     |      PGetTriagsVidx(self: fcecodec.Mesh, pid: int) -> buffer
     |      
     |      Returns (N*3, ) numpy array of global vert indexes for N triangles.
     |  
     |  PNumTriags(...)
     |      PNumTriags(self: fcecodec.Mesh, pid: int) -> int
     |  
     |  PNumVerts(...)
     |      PNumVerts(self: fcecodec.Mesh, pid: int) -> int
     |  
     |  PSetName(...)
     |      PSetName(self: fcecodec.Mesh, pid: int, name: str) -> None
     |  
     |  PSetPos(...)
     |      PSetPos(self: fcecodec.Mesh, pid: int, pos: numpy.ndarray[numpy.float32]) -> None
     |  
     |  PSetTriagsFlags(...)
     |      PSetTriagsFlags(self: fcecodec.Mesh, pid: int, arr: numpy.ndarray[numpy.int32]) -> None
     |      
     |      Expects (N, ) numpy array for N triangles
     |  
     |  PSetTriagsTexcoords(...)
     |      PSetTriagsTexcoords(self: fcecodec.Mesh, pid: int, arr: numpy.ndarray[numpy.float32]) -> None
     |      
     |      arr: uuuvvv..., Expects (N*6, ) numpy array for N triangles.
     |  
     |  PSetTriagsTexpages(...)
     |      PSetTriagsTexpages(self: fcecodec.Mesh, pid: int, arr: numpy.ndarray[numpy.int32]) -> None
     |      
     |      Expects (N, ) numpy array for N triangles
     |  
     |  PrintInfo(...)
     |      PrintInfo(self: fcecodec.Mesh) -> None
     |  
     |  PrintParts(...)
     |      PrintParts(self: fcecodec.Mesh) -> None
     |  
     |  PrintTriags(...)
     |      PrintTriags(self: fcecodec.Mesh) -> None
     |  
     |  PrintVerts(...)
     |      PrintVerts(self: fcecodec.Mesh) -> None
     |  
     |  __init__(...)
     |      __init__(self: fcecodec.Mesh) -> None
     |  
     |  ----------------------------------------------------------------------
     |  Readonly properties defined here:
     |  
     |  MNumParts
     |  
     |  MNumTriags
     |  
     |  MNumVerts
     |  
     |  MVertsGetMap_idx2order
     |      Maps from global vert indexes (contained in triangles) to global vertex order.
     |  
     |  ----------------------------------------------------------------------
     |  Data descriptors defined here:
     |  
     |  MNumArts
     |      Usually equal to 1. Larger values enable multi-texture access for cop#.fce
     |  
     |  MUnknown3
     |      FCE4M only. Unknown purpose.
     |  
     |  MVertsAnimation
     |      Returns (N, ) numpy array for N vertices.
     |  
     |  MVertsDamgdNorms
     |      Returns (N*3, ) numpy array for N vertices.
     |  
     |  MVertsDamgdPos
     |      Local vertice positions. Returns (N*3, ) numpy array for N vertices.
     |  
     |  MVertsNorms
     |      Returns (N*3, ) numpy array for N vertices.
     |  
     |  MVertsPos
     |      Local vertice positions. Returns (N*3, ) numpy array for N vertices.
     |  
     |  ----------------------------------------------------------------------
     |  Static methods inherited from pybind11_builtins.pybind11_object:
     |  
     |  __new__(*args, **kwargs) from pybind11_builtins.pybind11_type
     |      Create and return a new object.  See help(type) for accurate signature.

FUNCTIONS
    GetFceVersion(...) method of builtins.PyCapsule instance
        GetFceVersion(buf: str) -> int
        
        Returns 3 (FCE3), 4 (FCE4), 5 (FCE4M), negative (invalid)
    
    PrintFceInfo(...) method of builtins.PyCapsule instance
        PrintFceInfo(buf: str) -> None
    
    ValidateFce(...) method of builtins.PyCapsule instance
        ValidateFce(buf: str) -> int
        
        Returns 1 for valid FCE data, 0 otherwise.

VERSION
    1.3
```