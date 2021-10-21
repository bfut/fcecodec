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
     |  DelPartTriags(...)
     |      DelPartTriags(self: fcecodec.Mesh, pid: int, idxs: List[int]) -> bool
     |  
     |  DelUnrefdVerts(...)
     |      DelUnrefdVerts(self: fcecodec.Mesh) -> bool
     |      
     |      Delete vertices that are not referenced by any triangle.
     |  
     |  GetPartName(...)
     |      GetPartName(self: fcecodec.Mesh, arg0: int) -> str
     |  
     |  GetPartPos(...)
     |      GetPartPos(self: fcecodec.Mesh, arg0: int) -> List[float[3]]
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
     |  InsertPart(...)
     |      InsertPart(self: fcecodec.Mesh, mesh_src: fcecodec.Mesh, part_idx: int) -> int
     |      
     |      Insert (copy) specified part from mesh_src. Returns new part index.
     |  
     |  PNumTriags(...)
     |      PNumTriags(self: fcecodec.Mesh, arg0: int) -> int
     |  
     |  PNumVerts(...)
     |      PNumVerts(self: fcecodec.Mesh, arg0: int) -> int
     |  
     |  SetPartName(...)
     |      SetPartName(self: fcecodec.Mesh, arg0: int, arg1: str) -> None
     |  
     |  SetPartPos(...)
     |      SetPartPos(self: fcecodec.Mesh, arg0: int, arg1: List[float[3]]) -> None
     |  
     |  __init__(...)
     |      __init__(self: fcecodec.Mesh) -> None
     |  
     |  center_part(...)
     |      center_part(self: fcecodec.Mesh, pid: int) -> bool
     |      
     |      Center specified part pos to local centroid.
     |  
     |  decode(...)
     |      decode(self: fcecodec.Mesh, arg0: str) -> None
     |  
     |  del_part(...)
     |      del_part(self: fcecodec.Mesh, pid: int) -> bool
     |  
     |  encode_fce3(...)
     |      encode_fce3(self: fcecodec.Mesh, center_parts: bool = True) -> bytes
     |  
     |  encode_fce4(...)
     |      encode_fce4(self: fcecodec.Mesh, center_parts: bool = True) -> bytes
     |  
     |  encode_fce4m(...)
     |      encode_fce4m(self: fcecodec.Mesh, center_parts: bool = True) -> bytes
     |  
     |  export_obj(...)
     |      export_obj(self: fcecodec.Mesh, objpath: str, mtlpath: str, texname: str, print_damage: int = 0, print_dummies: int = 0) -> None
     |  
     |  get_colors(...)
     |      get_colors(self: fcecodec.Mesh) -> buffer
     |  
     |  get_dummy_names(...)
     |      get_dummy_names(self: fcecodec.Mesh) -> List[str]
     |  
     |  get_dummy_pos(...)
     |      get_dummy_pos(self: fcecodec.Mesh) -> buffer
     |  
     |  info(...)
     |      info(self: fcecodec.Mesh) -> None
     |  
     |  merge_parts(...)
     |      merge_parts(self: fcecodec.Mesh, pid1: int, pid2: int) -> int
     |      
     |      Returns new part index.
     |  
     |  move_part(...)
     |      move_part(self: fcecodec.Mesh, pid: int) -> int
     |      
     |      Move up specified part towards order 0. Returns new part index.
     |  
     |  print_parts(...)
     |      print_parts(self: fcecodec.Mesh) -> None
     |  
     |  print_triags(...)
     |      print_triags(self: fcecodec.Mesh) -> None
     |  
     |  print_verts(...)
     |      print_verts(self: fcecodec.Mesh) -> None
     |  
     |  set_colors(...)
     |      set_colors(self: fcecodec.Mesh, integers: numpy.ndarray[numpy.uint8]) -> None
     |      
     |      Expects shape=(N, 4, 4)
     |  
     |  set_dummy_names(...)
     |      set_dummy_names(self: fcecodec.Mesh, strings: List[str]) -> None
     |  
     |  set_dummy_pos(...)
     |      set_dummy_pos(self: fcecodec.Mesh, floats: numpy.ndarray[numpy.float32]) -> None
     |      
     |      Expects shape (N*3, ) for N dummies
     |  
     |  valid(...)
     |      valid(self: fcecodec.Mesh) -> bool
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
    fce_valid(...) method of builtins.PyCapsule instance
        fce_valid(buf: str) -> int
    
    print_fce_info(...) method of builtins.PyCapsule instance
        print_fce_info(buf: str) -> None
```
