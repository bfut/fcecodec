# fcecodec - Python extension module
This file describes installation, and usage of fcecodec as Python extension
module.

## Installation
Requires Python 3.7+

       git clone https://github.com/bfut/fcecodec.git
       cd fcecodec/python
       python setup.py install

To uninstall, delete the module. Its path can be printed with

       python -c "import fcecodec; print(fcecodec)"

## Usage
```
Help on module fcecodec:

NAME
    fcecodec - FCE decoder/encoder

CLASSES
    pybind11_builtins.pybind11_object(builtins.object)
        Mesh
        Part
    
    class Mesh(pybind11_builtins.pybind11_object)
     |  Method resolution order:
     |      Mesh
     |      pybind11_builtins.pybind11_object
     |      builtins.object
     |  
     |  Methods defined here:
     |  
     |  __init__(...)
     |      __init__(*args, **kwargs)
     |      Overloaded function.
     |      
     |      1. __init__(self: fcecodec.Mesh) -> None
     |      
     |      2. __init__(self: fcecodec.Mesh, mesh_src: fcecodec.Mesh, part_idx: int) -> None
     |      
     |       Copy specified part from mesh_src to new mesh.
     |  
     |  center_part(...)
     |      center_part(self: fcecodec.Mesh, idx: int) -> bool
     |      
     |      Center specified part pos to local centroid.
     |  
     |  copy_part(...)
     |      copy_part(self: fcecodec.Mesh, mesh_src: fcecodec.Mesh, part_idx: int) -> int
     |      
     |      Copy specified part from mesh_src. Returns new part index.
     |  
     |  decode_fce(...)
     |      decode_fce(self: fcecodec.Mesh, buf: str) -> bool
     |  
     |  del_part(...)
     |      del_part(self: fcecodec.Mesh, idx: int) -> bool
     |  
     |  del_unrefd_verts(...)
     |      del_unrefd_verts(self: fcecodec.Mesh) -> bool
     |      
     |      Delete vertices that are not referenced by any triangle.
     |  
     |  encode_fce3(...)
     |      encode_fce3(self: fcecodec.Mesh, fcepath: str, center_parts: bool = True) -> bool
     |  
     |  encode_fce4(...)
     |      encode_fce4(self: fcecodec.Mesh, fcepath: str, center_parts: bool = True) -> bool
     |  
     |  export_obj(...)
     |      export_obj(self: fcecodec.Mesh, objpath: str, mtlpath: str, texname: str, print_damage: int = 0, print_dummies: int = 0) -> bool
     |  
     |  get_colors(...)
     |      get_colors(self: fcecodec.Mesh) -> buffer
     |  
     |  get_dmgd_verts_norms(...)
     |      get_dmgd_verts_norms(self: fcecodec.Mesh) -> buffer
     |      
     |      Damage model vertice normals. Returns (3*N, ) array for N vertices.
     |  
     |  get_dmgd_verts_pos(...)
     |      get_dmgd_verts_pos(self: fcecodec.Mesh) -> buffer
     |      
     |      Damage model local vertice positions. Returns (3*N, ) array for N vertices.
     |  
     |  get_dummy_names(...)
     |      get_dummy_names(self: fcecodec.Mesh) -> List[str]
     |  
     |  get_dummy_pos(...)
     |      get_dummy_pos(self: fcecodec.Mesh) -> buffer
     |  
     |  get_parts(...)
     |      get_parts(self: fcecodec.Mesh) -> List[fcecodec.Part]
     |  
     |  get_verts_anim(...)
     |      get_verts_anim(self: fcecodec.Mesh) -> buffer
     |      
     |      Vertice animation flag. Returns (N, ) array for N vertices.
     |  
     |  get_verts_map_idx2order(...)
     |      get_verts_map_idx2order(self: fcecodec.Mesh) -> buffer
     |      
     |      Triangles contain global vert indexes. Via those, this vector maps to global vertex order.
     |  
     |  get_verts_norms(...)
     |      get_verts_norms(self: fcecodec.Mesh) -> buffer
     |      
     |      Vertice normals. Returns (3*N, ) array for N vertices.
     |  
     |  get_verts_pos(...)
     |      get_verts_pos(self: fcecodec.Mesh) -> buffer
     |      
     |      Local vertice positions. Returns (3*N, ) array for N vertices.
     |  
     |  info(...)
     |      info(self: fcecodec.Mesh) -> None
     |      
     |      Print stats to console.
     |  
     |  merge_parts(...)
     |      merge_parts(self: fcecodec.Mesh, idx1: int, idx2: int) -> int
     |      
     |      Returns new part index.
     |  
     |  move_part(...)
     |      move_part(self: fcecodec.Mesh, idx: int) -> int
     |      
     |      Move up specified part towards order 0. Returns new part index.
     |  
     |  num_parts(...)
     |      num_parts(self: fcecodec.Mesh) -> int
     |  
     |  num_triags(...)
     |      num_triags(self: fcecodec.Mesh) -> int
     |  
     |  num_verts(...)
     |      num_verts(self: fcecodec.Mesh) -> int
     |  
     |  part_at(...)
     |      part_at(self: fcecodec.Mesh, idx: int) -> fcecodec.Part
     |      
     |      DEPRECATED
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
     |  set_verts_anim(...)
     |      set_verts_anim(self: fcecodec.Mesh, data: numpy.ndarray[numpy.int32]) -> None
     |      
     |      Expects (N, ) array where N = Part.num_verts()
     |  
     |  valid(...)
     |      valid(self: fcecodec.Mesh) -> bool
     |  
     |  ----------------------------------------------------------------------
     |  Static methods inherited from pybind11_builtins.pybind11_object:
     |  
     |  __new__(*args, **kwargs) from pybind11_builtins.pybind11_type
     |      Create and return a new object.  See help(type) for accurate signature.
    
    class Part(pybind11_builtins.pybind11_object)
     |  Method resolution order:
     |      Part
     |      pybind11_builtins.pybind11_object
     |      builtins.object
     |  
     |  Methods defined here:
     |  
     |  __init__(...)
     |      __init__(self: fcecodec.Part) -> None
     |  
     |  get_name(...)
     |      get_name(self: fcecodec.Part) -> str
     |  
     |  get_pos(...)
     |      get_pos(self: fcecodec.Part) -> buffer
     |  
     |  get_triags_flags(...)
     |      get_triags_flags(self: fcecodec.Part) -> buffer
     |  
     |  get_triags_texpages(...)
     |      get_triags_texpages(self: fcecodec.Part) -> buffer
     |  
     |  get_triags_vidx(...)
     |      get_triags_vidx(self: fcecodec.Part) -> buffer
     |      
     |      Returns (N, 3) array of global vert indexes for N triangles.
     |  
     |  num_triags(...)
     |      num_triags(self: fcecodec.Part) -> int
     |  
     |  num_verts(...)
     |      num_verts(self: fcecodec.Part) -> int
     |  
     |  set_name(...)
     |      set_name(self: fcecodec.Part, name: str) -> None
     |  
     |  set_pos(...)
     |      set_pos(self: fcecodec.Part, position: numpy.ndarray[numpy.float32]) -> None
     |  
     |  set_triags_flags(...)
     |      set_triags_flags(self: fcecodec.Part, flags: numpy.ndarray[numpy.int32]) -> None
     |      
     |      Expects (N, ) array where N = Part.num_triags()
     |  
     |  set_triags_texpages(...)
     |      set_triags_texpages(self: fcecodec.Part, texpages: numpy.ndarray[numpy.int32]) -> None
     |      
     |      Expects (N, ) array where N = Part.num_triags()
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
        print_fce_info(fcepath: str) -> None
```
