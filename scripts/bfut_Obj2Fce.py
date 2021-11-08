"""
  bfut_Obj2Fce.py - import OBJ file using tinyobjloader, export as FCE file using fcecodec

REQUIRES:
  installing <https://github.com/bfut/fcecodec>
  installing numpy
  installing <https://github.com/tinyobjloader/tinyobjloader>

DEFAULT PARAMETERS:
  fce_version=4 : expects 3|4|'4M' for FCE3, FCE4, FCE4M, respectively
  center_parts=1 : localize part vertice positions to part centroid, setting part position (expects 0|1)
  material2texpage=0 : maps OBJ face materials to FCE texpages (expects 0|1)
  material2triagflag=1 : maps OBJ face materials to FCE triangles flag (expects 0|1)

LICENSE:
  This file is distributed under: CC BY-SA 4.0
      <https://creativecommons.org/licenses/by-sa/4.0/>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.
"""
import argparse
import pathlib
import sys
import tinyobjloader
import numpy as np

script_path = pathlib.Path(__file__).parent

# Look for local build, if not installed
try:
    import fcecodec
except ModuleNotFoundError:
    import sys
    p = pathlib.Path(script_path / "../python/build")
    # print(p)
    for x in p.glob("**"):
        sys.path.append(str(x.resolve()))
    import fcecodec

# Parse command (or print module help)
parser = argparse.ArgumentParser()
parser.add_argument("cmd", nargs=1, help="path")
args = parser.parse_args()

filepath_obj_input = pathlib.Path(args.cmd[0])
output_path_stem = pathlib.Path(filepath_obj_input.parent / filepath_obj_input.stem)
filepath_fce_output = output_path_stem.with_suffix(".fce")


# -------------------------------------- parameters
fce_version = 3
center_parts = 1
material2texpage = 0
material2triagflag = 1


# -------------------------------------- fcecodec wrappers
def PrintFceInfo(path):
    with open(path, "rb") as f:
        # print("PrintFceInfo(", path, ")")
        buf = f.read()
        fcecodec.PrintFceInfo(buf)
        assert(fcecodec.ValidateFce( buf ) == 1)

def WriteFce(version, mesh, path, center_parts = 1):
    with open(path, "wb") as f:
        if version == 3:
            buf = mesh.IoEncode_Fce3(center_parts)
        elif version == 4:
            buf = mesh.IoEncode_Fce4(center_parts)
        else:
            buf = mesh.IoEncode_Fce4M(center_parts)
        assert(fcecodec.ValidateFce(buf) == 1)
        f.write(buf)


# -------------------------------------- tinyobjloader wrappers
def LoadObj(filename):
    # src: https://github.com/tinyobjloader/tinyobjloader/blob/master/python/sample.py
    reader = tinyobjloader.ObjReader()
    config = tinyobjloader.ObjReaderConfig()
    config.triangulate = False
    ret = reader.ParseFromFile(str(filename), config)
    if ret == False:
        print("Failed to load : ", filename)
        print("Warn:", reader.Warning())
        print("Err:", reader.Error())
        sys.exit(-1)
    if reader.Warning():
        print("Warn:", reader.Warning())
    return reader

def GetVerts(reader):  # xyzxyzxyz
    attrib = reader.GetAttrib()
    arr = attrib.numpy_vertices()
    # print(arr)
    print(type(attrib.numpy_vertices()), arr.shape, arr.ndim)
    return arr

def GetNormals(reader):  # xyzxyzxyz
    attrib = reader.GetAttrib()
    arr = np.array(attrib.normals)
    print(type(arr), arr.shape, arr.ndim)
    return arr

def GetTexcoords(reader):  # uvuvuv
    attrib = reader.GetAttrib()
    arr = np.array(attrib.texcoords)
    print(type(arr), arr.shape, arr.ndim)
    return arr

def PrintShapes(reader):
    shapes = reader.GetShapes()
    print("Num shapes: ", len(shapes))
    for shape in shapes:
        print(shape.name,
              "faces={}".format(int(shape.mesh.numpy_indices().shape[0] / 3)))

def GetShapeNames(reader):
    shapenames = []
    shapes = reader.GetShapes()
    for i in range(len(shapes)):
        shapenames += [shapes[i].name]
    return shapenames

def GetShapeFaces(reader, vertices, normals, texcoords, shapename, material2texpage, material2triagflag):
    shape = None
    shapes = reader.GetShapes()
    for s in shapes:
        if s.name == shapename:
            shape = s
            break
    if shape is None:
        print("GetShapeFaces: cannot find specified shapename", shapename)
        return None
    s_NumFaces = int(shape.mesh.numpy_indices()[0::3].shape[0] / 3)

    s_faces = shape.mesh.numpy_indices()[0::3]
    normals_idxs = shape.mesh.numpy_indices()[1::3]
    texcoord_idxs = shape.mesh.numpy_indices()[2::3]

    print(shape.name, "faces={}".format(int(s_faces.shape[0] / 3)))
    print(shape.name, "normals={}".format(int(normals_idxs.shape[0])))
    print(shape.name, "texcoords={}".format(int(texcoord_idxs.shape[0])))

    s_verts = vertices.reshape(-1, 3)[ np.unique(s_faces) ].flatten()
    # Get normals (use vert positions, if no normals for shape)
    if np.amax(s_faces) <= int(normals.shape[0] / 3):
        s_norms = normals.reshape(-1, 3)[ np.unique(s_faces) ].flatten()  # normals[normals_idxs]
    else:
        s_norms = np.copy(s_verts)

    # uvuvuv... -> uuuvvv...
    s_texcs = np.empty(s_NumFaces * 6)
    for i in range( s_NumFaces ):
        for j in range(3):
            s_texcs[i * 6 + 0 * 3 + j] = texcoords[ texcoord_idxs[i * 3 + j] * 2 + 0 ]
            s_texcs[i * 6 + 1 * 3 + j] = texcoords[ texcoord_idxs[i * 3 + j] * 2 + 1 ]

    s_matls = shape.mesh.numpy_material_ids()

    return s_faces, s_verts, s_norms, s_texcs, s_matls


# -------------------------------------- fcecodec wrappers (continued)
def LocalizeVertIdxs(faces):
  return faces - np.amin(faces)

def ShapeToPart(reader,
                mesh, objverts, objnorms, objtexcoords, request_shapename,
                material2texpage, material2triagflag):
    s_faces, s_verts, s_norms, s_texcs, s_matls = GetShapeFaces(reader,
        objverts, objnorms, objtexcoords, request_shapename, material2texpage, material2triagflag)
    print("faces:", int(s_faces.shape[0] / 3))
    print("vert idx range: [", np.amin(s_faces), ",", np.amax(s_faces), "]")
    print("vertices:", int(s_verts.shape[0] / 3))
    print("normals:", s_norms.shape[0])
    print("texcoords:", s_texcs.shape[0], "->", int(s_texcs.shape[0] / 6))

    # print(s_faces)
    s_faces = LocalizeVertIdxs(s_faces)

    s_verts[2::3] = -s_verts[2::3]  # flip sign in Z-coordinate
    mesh.IoGeomDataToNewPart(s_faces, s_texcs, s_verts, s_norms)
    mesh.PSetName(mesh.MNumParts - 1, request_shapename)  # shapename to partname

    # map faces material IDs to triangles texpages
    if material2texpage == 1:
        texps = mesh.PGetTriagsTexpages_numpy(mesh.MNumParts - 1)
        for i in range(texps.shape[0]):
            texps[i] = s_matls[i]
        # print(type(texps), texps.dtype, texps.shape)
        mesh.PSetTriagsTexpages_numpy(mesh.MNumParts - 1, texps)

    # map faces material names to triangles flags iff
    # all material names are integer hex values (strings of the form '0xiii')
    if material2triagflag == 1:
        materials = reader.GetMaterials()
        map = True
        for i in range(len(materials)):
            tmp = materials[i].name
            # print(tmp)
            if tmp[:2] == '0x':
                try:
                    # print(tmp, "->", int(tmp[2:], base=16), "0x{}".format(hex(int(tmp[2:], base=16))))
                    val = int(tmp[2:], base=16)
                except ValueError:
                    print("Cannot map faces material names to triangles flags ('{0}' is not hex value) 1".format(materials[i].name))
                    map = False
                    break
            else:
                print("Cannot map faces material names to triangles flags ('{0}' is not hex value) 2".format(materials[i].name))
                map = False
                break
        if map:
            print("mapping faces material names to triangles flags...")
            tflags = mesh.PGetTriagsFlags_numpy(mesh.MNumParts - 1)
            for i in range(tflags.shape[0]):
                # print(materials[s_matls[i]].name, int(materials[s_matls[i]].name[2:], base=16))
                tflags[i] = int(materials[s_matls[i]].name[2:], base=16)
            mesh.PSetTriagsFlags_numpy(mesh.MNumParts - 1, tflags)

    return mesh


# -------------------------------------- workload
if fce_version not in [3, 4, '4m', '4M']:
    print("requires fce_version = 3|4|'4m' (received", fce_version, ")")
    quit()
if center_parts not in [0, 1]:
    print("requires center_parts = 0|1 (received", center_parts, ")")
    quit()
if material2texpage not in [0, 1]:
    print("requires material2texpage = 0|1 (received", material2texpage, ")")
    quit()
if material2triagflag not in [0, 1]:
    print("requires material2triagflag = 0|1 (received", material2triagflag, ")")
    quit()

# import OBJ
reader = LoadObj(filepath_obj_input)
attrib = reader.GetAttrib()
print("attrib.vertices = ", len(attrib.vertices), int(len(attrib.vertices) / 3))
print("attrib.normals = ", len(attrib.normals))
print("attrib.texcoords = ", len(attrib.texcoords))

# transform geometric data
objverts = GetVerts(reader)
objnorms = GetNormals(reader)
objtexcoords = GetTexcoords(reader)
# PrintShapes(reader)
shapenames = GetShapeNames(reader)

# export FCE
mesh = fcecodec.Mesh()
for i in range(len(shapenames)):
    print("s_name", shapenames[i])
    mesh = ShapeToPart(reader,
                       mesh, objverts, objnorms, objtexcoords, shapenames[i],
                       material2texpage, material2triagflag)
# mesh.PrintInfo()
WriteFce(fce_version, mesh, filepath_fce_output, center_parts)
PrintFceInfo(filepath_fce_output)