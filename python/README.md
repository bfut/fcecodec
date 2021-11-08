# fcecodec - Python extension module
This file describes installation, and usage of `fcecodec` as Python extension
module.

## Examples
See scripts used for testing<br/>
[/tests/ci-smoketest.py](/tests/ci-smoketest.py)<br/>
[/python/fcecodec_mywrappers.py](/python/fcecodec_mywrappers.py)

## Installation
Requires Python 3.7+ and the following dependencies

        python -m pip install --upgrade wheel setuptools pybind11

Install `fcecodec`

        git clone https://github.com/bfut/fcecodec.git
        python -m pip install -e fcecodec/python

#### Windows
`Microsoft Visual Studio` has to be installed. For a detailed description,
see the _Prerequisites_ section on
[microsoft.com: C++ extensions for Python](https://docs.microsoft.com/en-us/visualstudio/python/working-with-c-cpp-python-in-visual-studio?view=vs-2019#prerequisites)

Installing the `Anaconda` distribution, `bash`, and `git` is recommended.

Once these prerequisites have been met, installation generally works as
described above.

## Usage
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
     |      IoExportObj(self: fcecodec.Mesh, objpath: str, mtlpath: str, texname: str, print_damage: int = 0, print_dummies: int = 0) -> None
     |  
     |  IoGeomDataToNewPart(...)
     |      IoGeomDataToNewPart(self: fcecodec.Mesh, vert_idxs: numpy.ndarray[numpy.int32], vert_texcoords: numpy.ndarray[numpy.float32], vert_pos: numpy.ndarray[numpy.float32], normals: numpy.ndarray[numpy.float32]) -> int
     |      
     |      vert_idxs: 012..., vert_texcoords: uuuvvv... , vert_pos: xyzxyzxyz..., normals: xyzxyzxyz...
     |  
     |  IoGeomDataToNewPart_numpy(...)
     |      IoGeomDataToNewPart_numpy(self: fcecodec.Mesh, vert_idxs: numpy.ndarray[numpy.int32], vert_texcoords: numpy.ndarray[numpy.float32], vert_pos: numpy.ndarray[numpy.float32], normals: numpy.ndarray[numpy.float32]) -> int
     |      
     |      vert_idxs: 012..., vert_texcoords: uuuvvv... , vert_pos: xyzxyzxyz..., normals: xyzxyzxyz...
     |  
     |  MGetColors(...)
     |      MGetColors(self: fcecodec.Mesh) -> buffer
     |  
     |  MGetColors_numpy(...)
     |      MGetColors_numpy(self: fcecodec.Mesh) -> buffer
     |  
     |  MGetDummyNames(...)
     |      MGetDummyNames(self: fcecodec.Mesh) -> List[str]
     |  
     |  MGetDummyPos(...)
     |      MGetDummyPos(self: fcecodec.Mesh) -> buffer
     |  
     |  MGetDummyPos_numpy(...)
     |      MGetDummyPos_numpy(self: fcecodec.Mesh) -> buffer
     |  
     |  MSetColors(...)
     |      MSetColors(self: fcecodec.Mesh, colors: numpy.ndarray[numpy.uint8]) -> None
     |      
     |      Expects shape=(N, 4, 4)
     |  
     |  MSetColors_numpy(...)
     |      MSetColors_numpy(self: fcecodec.Mesh, colors: numpy.ndarray[numpy.uint8]) -> None
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
     |  MSetDummyPos_numpy(...)
     |      MSetDummyPos_numpy(self: fcecodec.Mesh, positions: numpy.ndarray[numpy.float32]) -> None
     |      
     |      Expects shape (N*3, ) for N dummies
     |  
     |  MValid(...)
     |      MValid(self: fcecodec.Mesh) -> bool
     |  
     |  OpCenterPart(...)
     |      OpCenterPart(self: fcecodec.Mesh, pid: int) -> bool
     |      
     |      Center specified part vertices positions to local centroid.
     |  
     |  OpCopyPart(...)
     |      OpCopyPart(self: fcecodec.Mesh, pid_src: int) -> int
     |      
     |      Copy specified part. Returns new part index.
     |  
     |  OpDelPartTriags(...)
     |      OpDelPartTriags(self: fcecodec.Mesh, pid: int, idxs: List[int]) -> bool
     |  
     |  OpDelUnrefdVerts(...)
     |      OpDelUnrefdVerts(self: fcecodec.Mesh) -> bool
     |      
     |      Delete all vertices that are not referenced by any triangle.
     |  
     |  OpDeletePart(...)
     |      OpDeletePart(self: fcecodec.Mesh, pid: int) -> bool
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
     |  PGetName(...)
     |      PGetName(self: fcecodec.Mesh, pid: int) -> str
     |  
     |  PGetPos(...)
     |      PGetPos(self: fcecodec.Mesh, pid: int) -> List[float[3]]
     |  
     |  PGetPos_numpy(...)
     |      PGetPos_numpy(self: fcecodec.Mesh, pid: int) -> buffer
     |  
     |  PGetTriagsFlags(...)
     |      PGetTriagsFlags(self: fcecodec.Mesh, pid: int) -> List[int]
     |  
     |  PGetTriagsFlags_numpy(...)
     |      PGetTriagsFlags_numpy(self: fcecodec.Mesh, pid: int) -> buffer
     |  
     |  PGetTriagsTexcoords_numpy(...)
     |      PGetTriagsTexcoords_numpy(self: fcecodec.Mesh, pid: int) -> buffer
     |      
     |      uuuvvv..., Returns (N*6, ) numpy array for N triangles.
     |  
     |  PGetTriagsTexpages(...)
     |      PGetTriagsTexpages(self: fcecodec.Mesh, pid: int) -> List[int]
     |  
     |  PGetTriagsTexpages_numpy(...)
     |      PGetTriagsTexpages_numpy(self: fcecodec.Mesh, pid: int) -> buffer
     |  
     |  PGetTriagsVidx(...)
     |      PGetTriagsVidx(self: fcecodec.Mesh, pid: int) -> List[int]
     |      
     |      Returns (N*3, ) array of global vert indexes for N triangles.
     |  
     |  PGetTriagsVidx_numpy(...)
     |      PGetTriagsVidx_numpy(self: fcecodec.Mesh, pid: int) -> buffer
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
     |      PSetPos(self: fcecodec.Mesh, pid: int, pos: List[float[3]]) -> None
     |  
     |  PSetPos_numpy(...)
     |      PSetPos_numpy(self: fcecodec.Mesh, pid: int, pos: numpy.ndarray[numpy.float32]) -> None
     |  
     |  PSetTriagsFlags(...)
     |      PSetTriagsFlags(self: fcecodec.Mesh, pid: int, arr: List[int]) -> None
     |      
     |      Expects (N, ) array for N triangles
     |  
     |  PSetTriagsFlags_numpy(...)
     |      PSetTriagsFlags_numpy(self: fcecodec.Mesh, pid: int, arr: numpy.ndarray[numpy.int32]) -> None
     |      
     |      Expects (N, ) numpy array for N triangles
     |  
     |  PSetTriagsTexcoords_numpy(...)
     |      PSetTriagsTexcoords_numpy(self: fcecodec.Mesh, pid: int, arr: numpy.ndarray[numpy.float32]) -> None
     |      
     |      arr: uuuvvv..., Expects (N*6, ) numpy array for N triangles.
     |  
     |  PSetTriagsTexpages(...)
     |      PSetTriagsTexpages(self: fcecodec.Mesh, pid: int, arr: List[int]) -> None
     |      
     |      Expects (N, ) array for N triangles
     |  
     |  PSetTriagsTexpages_numpy(...)
     |      PSetTriagsTexpages_numpy(self: fcecodec.Mesh, pid: int, arr: numpy.ndarray[numpy.int32]) -> None
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
     |  MVertsGetMap_idx2order_numpy
     |      Maps from global vert indexes (contained in triangles) to global vertex order.
     |  
     |  ----------------------------------------------------------------------
     |  Data descriptors defined here:
     |  
     |  MNumArts
     |      Usually equal to 1. Larger values enable multi-texture access for cop#.fce
     |  
     |  MVertsAnimation
     |      Returns (N, ) array for N vertices.
     |  
     |  MVertsAnimation_numpy
     |      Returns (N, ) numpy array for N vertices.
     |  
     |  MVertsDamgdNorms
     |      Returns (N*3, ) array for N vertices.
     |  
     |  MVertsDamgdNorms_numpy
     |      Returns (N*3, ) numpy array for N vertices.
     |  
     |  MVertsDamgdPos
     |      Local vertice positions. Returns (N*3, ) array for N vertices.
     |  
     |  MVertsDamgdPos_numpy
     |      Local vertice positions. Returns (N*3, ) numpy array for N vertices.
     |  
     |  MVertsNorms
     |      Returns (N*3, ) array for N vertices.
     |  
     |  MVertsNorms_numpy
     |      Returns (N*3, ) numpy array for N vertices.
     |  
     |  MVertsPos
     |      Local vertice positions. Returns (N*3, ) array for N vertices.
     |  
     |  MVertsPos_numpy
     |      Local vertice positions. Returns (N*3, ) numpy array for N vertices.
     |  
     |  ----------------------------------------------------------------------
     |  Static methods inherited from pybind11_builtins.pybind11_object:
     |  
     |  __new__(*args, **kwargs) from pybind11_builtins.pybind11_type
     |      Create and return a new object.  See help(type) for accurate signature.

FUNCTIONS
    PrintFceInfo(...) method of builtins.PyCapsule instance
        PrintFceInfo(buf: str) -> None
    
    ValidateFce(...) method of builtins.PyCapsule instance
        ValidateFce(buf: str) -> int
```
