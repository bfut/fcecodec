# fcecodec - Python extension module
This file describes installation, and usage of fcecodec as Python extension
module.

## Installation
Requires Python 3.7+ and the following dependencies

        python -m pip install --upgrade wheel setuptools pybind11

Install `fcecodec`

        git clone https://github.com/bfut/fcecodec.git
        cd fcecodec/python
        python setup.py install

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
     |  CenterPart(...)
     |      CenterPart(self: fcecodec.Mesh, pid: int) -> bool
     |      
     |      Center specified part pos to local centroid.
     |  
     |  Decode(...)
     |      Decode(self: fcecodec.Mesh, arg0: str) -> None
     |  
     |  DelPartTriags(...)
     |      DelPartTriags(self: fcecodec.Mesh, pid: int, idxs: List[int]) -> bool
     |  
     |  DelUnrefdVerts(...)
     |      DelUnrefdVerts(self: fcecodec.Mesh) -> bool
     |      
     |      Delete vertices that are not referenced by any triangle.
     |  
     |  DeletePart(...)
     |      DeletePart(self: fcecodec.Mesh, pid: int) -> bool
     |  
     |  Encode_Fce3(...)
     |      Encode_Fce3(self: fcecodec.Mesh, center_parts: bool = True) -> bytes
     |  
     |  Encode_Fce4(...)
     |      Encode_Fce4(self: fcecodec.Mesh, center_parts: bool = True) -> bytes
     |  
     |  Encode_Fce4M(...)
     |      Encode_Fce4M(self: fcecodec.Mesh, center_parts: bool = True) -> bytes
     |  
     |  ExportObj(...)
     |      ExportObj(self: fcecodec.Mesh, objpath: str, mtlpath: str, texname: str, print_damage: int = 0, print_dummies: int = 0) -> None
     |  
     |  GeomDataToNewPart(...)
     |      GeomDataToNewPart(self: fcecodec.Mesh, vert_idxs: numpy.ndarray[numpy.int32], vert_texcoords: numpy.ndarray[numpy.float32], vert_pos: numpy.ndarray[numpy.float32], normals: numpy.ndarray[numpy.float32]) -> int
     |  
     |  GetColors(...)
     |      GetColors(self: fcecodec.Mesh) -> buffer
     |  
     |  GetDummyNames(...)
     |      GetDummyNames(self: fcecodec.Mesh) -> List[str]
     |  
     |  GetDummyPos(...)
     |      GetDummyPos(self: fcecodec.Mesh) -> buffer
     |  
     |  GetPartName(...)
     |      GetPartName(self: fcecodec.Mesh, pid: int) -> str
     |  
     |  GetPartPos(...)
     |      GetPartPos(self: fcecodec.Mesh, pid: int) -> List[float[3]]
     |  
     |  GetTriagsFlags(...)
     |      GetTriagsFlags(self: fcecodec.Mesh, arg0: int) -> List[int]
     |  
     |  GetTriagsFlags_numpy(...)
     |      GetTriagsFlags_numpy(self: fcecodec.Mesh, arg0: int) -> buffer
     |  
     |  GetTriagsTexpages(...)
     |      GetTriagsTexpages(self: fcecodec.Mesh, arg0: int) -> List[int]
     |  
     |  GetTriagsTexpages_numpy(...)
     |      GetTriagsTexpages_numpy(self: fcecodec.Mesh, arg0: int) -> buffer
     |  
     |  GetTriagsVidx(...)
     |      GetTriagsVidx(self: fcecodec.Mesh, arg0: int) -> List[int]
     |      
     |      Returns (N*3, ) array of global vert indexes for N triangles.
     |  
     |  GetTriagsVidx_numpy(...)
     |      GetTriagsVidx_numpy(self: fcecodec.Mesh, arg0: int) -> buffer
     |      
     |      Returns (N*3, ) numpy array of global vert indexes for N triangles.
     |  
     |  GetVertsMap_idx2order(...)
     |      GetVertsMap_idx2order(self: fcecodec.Mesh) -> List[int]
     |      
     |      Maps from global vert indexes (contained in triangles) to global vertex order.
     |  
     |  GetVertsMap_idx2order_numpy(...)
     |      GetVertsMap_idx2order_numpy(self: fcecodec.Mesh) -> buffer
     |      
     |      Maps from global vert indexes (contained in triangles) to global vertex order.
     |  
     |  Info(...)
     |      Info(self: fcecodec.Mesh) -> None
     |  
     |  InsertPart(...)
     |      InsertPart(self: fcecodec.Mesh, mesh_src: fcecodec.Mesh, pid_src: int) -> int
     |      
     |      Insert (copy) specified part from mesh_src. Returns new part index.
     |  
     |  MergeParts(...)
     |      MergeParts(self: fcecodec.Mesh, pid1: int, pid2: int) -> int
     |      
     |      Returns new part index.
     |  
     |  MovePart(...)
     |      MovePart(self: fcecodec.Mesh, pid: int) -> int
     |      
     |      Move up specified part towards order 0. Returns new part index.
     |  
     |  PNumTriags(...)
     |      PNumTriags(self: fcecodec.Mesh, pid: int) -> int
     |  
     |  PNumVerts(...)
     |      PNumVerts(self: fcecodec.Mesh, pid: int) -> int
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
     |  SetColors(...)
     |      SetColors(self: fcecodec.Mesh, colors: numpy.ndarray[numpy.uint8]) -> None
     |      
     |      Expects shape=(N, 4, 4)
     |  
     |  SetDummyNames(...)
     |      SetDummyNames(self: fcecodec.Mesh, names: List[str]) -> None
     |  
     |  SetDummyPos(...)
     |      SetDummyPos(self: fcecodec.Mesh, positions: numpy.ndarray[numpy.float32]) -> None
     |      
     |      Expects shape (N*3, ) for N dummies
     |  
     |  SetPartName(...)
     |      SetPartName(self: fcecodec.Mesh, pid: int, name: str) -> None
     |  
     |  SetPartPos(...)
     |      SetPartPos(self: fcecodec.Mesh, pid: int, pos: List[float[3]]) -> None
     |  
     |  SetTriagsFlags(...)
     |      SetTriagsFlags(self: fcecodec.Mesh, arg0: int, arg1: List[int]) -> None
     |      
     |      Expects (N, ) array for N triangles
     |  
     |  SetTriagsFlags_numpy(...)
     |      SetTriagsFlags_numpy(self: fcecodec.Mesh, arg0: int, arg1: numpy.ndarray[numpy.int32]) -> None
     |      
     |      Expects (N, ) numpy array for N triangles
     |  
     |  SetTriagsTexpages(...)
     |      SetTriagsTexpages(self: fcecodec.Mesh, arg0: int, arg1: List[int]) -> None
     |      
     |      Expects (N, ) array for N triangles
     |  
     |  SetTriagsTexpages_numpy(...)
     |      SetTriagsTexpages_numpy(self: fcecodec.Mesh, arg0: int, arg1: numpy.ndarray[numpy.int32]) -> None
     |      
     |      Expects (N, ) numpy array for N triangles
     |  
     |  Valid(...)
     |      Valid(self: fcecodec.Mesh) -> bool
     |  
     |  __init__(...)
     |      __init__(self: fcecodec.Mesh) -> None
     |  
     |  ----------------------------------------------------------------------
     |  Readonly properties defined here:
     |  
     |  num_parts
     |  
     |  num_triags
     |  
     |  num_verts
     |  
     |  ----------------------------------------------------------------------
     |  Data descriptors defined here:
     |  
     |  DamgdVertsNorms
     |      Returns (N*3, ) array for N vertices.
     |  
     |  DamgdVertsNorms_numpy
     |      Returns (N*3, ) numpy array for N vertices.
     |  
     |  DamgdVertsPos
     |      Local vertice positions. Returns (N*3, ) array for N vertices.
     |  
     |  DamgdVertsPos_numpy
     |      Local vertice positions. Returns (N*3, ) numpy array for N vertices.
     |  
     |  VertsAnimation
     |      Returns (N, ) array for N vertices.
     |  
     |  VertsAnimation_numpy
     |      Returns (N, ) numpy array for N vertices.
     |  
     |  VertsNorms
     |      Returns (N*3, ) array for N vertices.
     |  
     |  VertsNorms_numpy
     |      Returns (N*3, ) numpy array for N vertices.
     |  
     |  VertsPos
     |      Local vertice positions. Returns (N*3, ) array for N vertices.
     |  
     |  VertsPos_numpy
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
