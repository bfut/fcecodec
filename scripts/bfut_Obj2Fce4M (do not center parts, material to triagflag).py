"""
    bfut_Obj2Fce4M (do not center parts, material to triagflag).py

DESCRIPTION
    import OBJ file using tinyobjloader, export as FCE file using fcecodec
    all faces must be triangles

REQUIRES
    installing fcecodec <https://github.com/bfut/fcecodec>
    installing numpy <https://numpy.org/install>
    installing tinyobjloader <https://github.com/tinyobjloader/tinyobjloader/tree/master/python>

LICENSE
    Copyright (C) 2021 and later Benjamin Futasz <https://github.com/bfut>

    This software is provided 'as-is', without any express or implied
    warranty.  In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
        claim that you wrote the original software. If you use this software
        in a product, an acknowledgment in the product documentation would be
        appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be
        misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.
"""
CONFIG = {
    "fce_version"        : '4M',  # output format version; expects 'keep version' or '3'|'4'|'4M' for FCE3, FCE4, FCE4M, respectively
    "center_parts"       : 0,  # localize part vertice positions to part centroid, setting part position (expects 0|1)
    "material2texpage"   : 0,  # maps OBJ face materials to FCE texpages (expects 0|1)
    "material2triagflag" : 1,  # maps OBJ face materials to FCE triangles flag (expects 0|1)
}
import argparse
import pathlib
import re
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


# -------------------------------------- fcecodec wrappers
def PrintFceInfo(path):
    with open(path, "rb") as f:
        # print("PrintFceInfo(", path, ")")
        buf = f.read()
        fcecodec.PrintFceInfo(buf)
        assert(fcecodec.ValidateFce( buf ) == 1)

def WriteFce(version, mesh, path, center_parts = 1):
    with open(path, "wb") as f:
        if version == '3':
            buf = mesh.IoEncode_Fce3(center_parts)
        elif version == '4':
            buf = mesh.IoEncode_Fce4(center_parts)
        else:
            buf = mesh.IoEncode_Fce4M(center_parts)
        assert(fcecodec.ValidateFce(buf) == 1)
        f.write(buf)

def GetPartNames(mesh):
    # part_names = []
    # for i in range(mesh.MNumParts):
    #     part_names += mesh.PGetName(i)
    part_names = np.empty(shape=(mesh.MNumParts, ), dtype='U64')
    for i in range(mesh.MNumParts):
        part_names[i] = mesh.PGetName(i)
        i += 1
    return part_names

def GetPartGlobalOrderVidxs(mesh, pid):
    map = mesh.MVertsGetMap_idx2order_numpy
    part_vidxs = mesh.PGetTriagsVidx_numpy(pid)
    for i in range(part_vidxs.shape[0]):
        # print(part_vidxs[i], map[part_vidxs[i]])
        part_vidxs[i] = map[part_vidxs[i]]
    return part_vidxs


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

    # cannot use np.unique(), as shape may have unreferenced verts
    # example: mcf1/car.viv->car.fce :HB
    vert_selection = np.arange(np.amin(s_faces), np.amax(s_faces) + 1)
    s_verts = vertices.reshape(-1, 3)[ vert_selection ].flatten()

    # Get normals (use vert positions, if no normals for shape)
    # obj: number of verts and normals may differ; fce: each vert has a normal
    norm_selection = np.empty(vert_selection.shape[0], dtype=int)
    map_v_t = np.copy(vert_selection)
    for i in range(map_v_t.shape[0]):
        argwhere = np.argwhere(s_faces == map_v_t[i])
        if len(argwhere) == 0:
            map_v_t[i] = -1
        else:
            map_v_t[i] = argwhere[0]
    for i in range(map_v_t.shape[0]):
        if map_v_t[i] < 0:
            norm_selection[i] = np.copy(vert_selection[i])
        else:
            norm_selection[i] = np.copy(normals_idxs[map_v_t[i]])
    if np.amax(s_faces) <= int(normals.shape[0] / 3):
        print("norm_selection")
        s_norms = normals.reshape(-1, 3)[ norm_selection ].flatten()  # normals[normals_idxs]
    else:
        print("shape has no normals... use vert positions as normals")
        s_norms = np.copy(s_verts)

    # uvuvuv... -> uuuvvv...
    s_texcs = np.empty(s_NumFaces * 6)
    for i in range( s_NumFaces ):
        for j in range(3):
            s_texcs[i * 6 + 0 * 3 + j] = texcoords[texcoord_idxs[i * 3 + j] * 2 + 0]
            s_texcs[i * 6 + 1 * 3 + j] = texcoords[texcoord_idxs[i * 3 + j] * 2 + 1]

    s_matls = shape.mesh.numpy_material_ids()

    return s_faces, s_verts, s_norms, s_texcs, s_matls


# -------------------------------------- more wrappers
def LocalizeVertIdxs(faces):
  return faces - np.amin(faces)

def GetFlagFromTags(tags):
    flag = 0x0
    for t in tags:
        # if t == "FDEF" : flag += 0x000
        if t == "FMAT" : flag += 0x001
        elif t == "FHIC" : flag += 0x002
        elif t == "FNOC" : flag += 0x004
        elif t == "FSET" : flag += 0x008
        elif t == "FUN1" : flag += 0x010
        elif t == "FALW" : flag += 0x020
        elif t == "FFRW" : flag += 0x040
        elif t == "FLEW" : flag += 0x080
        elif t == "FBAW" : flag += 0x100
        elif t == "FRIW" : flag += 0x200
        elif t == "FBRW" : flag += 0x400
        elif t == "FUN2" : flag += 0x800
    return flag

def GetTexPageFromTags(tags):
    txp = 0x0
    for t in tags:
        if t[:1] == "T":
            try:
                txp = int(t[1:])
            except ValueError:
                print("Cannot convert tag {} to texpage".format(t))
    return txp

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

    s_verts[2::3] = - s_verts[2::3]  # flip sign in Z-coordinate
    s_norms[2::3] = - s_norms[2::3]  # flip sign in Z-coordinate
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
        print("mapping faces material names to triangles flags...")
        materials = reader.GetMaterials()
        tflags = mesh.PGetTriagsFlags_numpy(mesh.MNumParts - 1)

        # if material name is hex value, map straight to triag flag
        # if it isn't, treat as string of tags
        for i in range(tflags.shape[0]):
            try:
                # print(materials[s_matls[i]].name, int(materials[s_matls[i]].name[2:], base=16))
                tflags[i] = int(materials[s_matls[i]].name[2:], base=16)
            except ValueError:
                # print(s_matls[i], materials[s_matls[i]].name)
                tags = materials[s_matls[i]].name.split('_')
                tflags[i] = GetFlagFromTags(tags)
        mesh.PSetTriagsFlags_numpy(mesh.MNumParts - 1, tflags)

        for i in range(len(materials)):
            tmp = materials[i].name
            # print(tmp)
            if tmp[:2] == '0x':
                try:
                    # print(tmp, "->", int(tmp[2:], base=16), "0x{}".format(hex(int(tmp[2:], base=16))))
                    val = int(tmp[2:], base=16)
                except ValueError:
                    print("Cannot map faces material name to triangles flags ('{0}' is not hex value) 1".format(materials[i].name))
                    map = False
                    break

    return mesh

def CopyDamagePartsVertsToPartsVerts(mesh):
    """
    Copy verts/norms of DAMAGE_<partname> to damaged verts/norms of <partname>
    """
    damgd_pids = []
    part_names = GetPartNames(mesh)
    mesh.PrintInfo()
    for damgd_pid in range(mesh.MNumParts):
        if part_names[damgd_pid][:7] == "DAMAGE_":
            pid = np.argwhere(part_names == part_names[damgd_pid][7:])
            # print(damgd_pid, part_names[damgd_pid], pid, pid.__len__())
            if pid.__len__() < 1:
                continue
            pid = pid[0][0]
            print("copy verts/norms of {0} to damaged verts/norms of {1}".format(part_names[damgd_pid], part_names[pid]))
            damgd_part_vidxs = GetPartGlobalOrderVidxs(mesh, damgd_pid)
            part_vidxs = GetPartGlobalOrderVidxs(mesh, pid)

            # cannot use np.unique(), as shape may have unreferenced verts
            # example: mcf1/car.viv->car.fce :HB
            # requires that OBJ parts verts are ordered in non-overlapping
            # ranges and that verts 0 and last, resp., are referenced
            damgd_part_vidxs = np.arange(np.amin(damgd_part_vidxs), np.amax(damgd_part_vidxs) + 1)
            part_vidxs = np.arange(np.amin(part_vidxs), np.amax(part_vidxs) + 1)

            dn = mesh.MVertsDamgdNorms_numpy.reshape((-1, 3))
            dv = mesh.MVertsDamgdPos_numpy.reshape((-1, 3))
            dn[part_vidxs] = mesh.MVertsNorms_numpy.reshape((-1, 3))[damgd_part_vidxs]
            dv[part_vidxs] = mesh.MVertsPos_numpy.reshape((-1, 3))[damgd_part_vidxs]
            mesh.MVertsDamgdNorms_numpy = dn.flatten()
            mesh.MVertsDamgdPos_numpy = dv.flatten()
            damgd_pids += [damgd_pid]
    for i in sorted(damgd_pids, reverse=True):
        mesh.OpDeletePart(i)
    print(damgd_pids)
    return mesh

def PartsToDummies(mesh):
    """
    From shapes named DUMMY_##_<dummyname> create dummies at centroids.
    """
    pids = []
    dms = []
    dms_pos = np.empty(0)
    r = re.compile("DUMMY_[0-9][0-9]_", re.IGNORECASE)
    for i in range(mesh.MNumParts):
        if r.match(mesh.PGetName(i)[:9]) is not None:
            print("convert part {0} '{1}' to dummy {2}".format(i, mesh.PGetName(i), mesh.PGetName(i)[9:]))
            pids += [i]
            dms += [mesh.PGetName(i)[9:]]
            mesh.OpCenterPart(i)
            dms_pos = np.append(dms_pos, mesh.PGetPos_numpy(i))
    for i in reversed(pids):
        mesh.OpDeletePart(i)
    print(pids, dms, dms_pos)
    mesh.MSetDummyNames(dms)
    mesh.MSetDummyPos_numpy(dms_pos)
    return mesh

def CenterParts(mesh):
    """
    Center part <partname> either to centroid, or if present to centroid of part POSITION_<partname>
    """
    pos_pids = []
    part_names = GetPartNames(mesh)
    pos_parts = dict()
    for name, i in zip(part_names, range(mesh.MNumParts)):
        if name[:9] == "POSITION_":
            pos_parts[name[9:]] = i
    print(pos_parts)
    for pid in range(mesh.MNumParts):
        if part_names[pid][:9] != "POSITION_":
            if part_names[pid] not in pos_parts:  # POSITION_<partname> not available
                print("center {0} to local centroid".format(part_names[pid]))
                mesh.OpCenterPart(pid)
            else:                                 # POSITION_<partname> is available
                pos_pid = pos_parts[part_names[pid]]
                print("center {0} to centroid of {1}".format(part_names[pid], part_names[pos_pid]))
                pos_pids += [pos_pid]
                mesh.OpCenterPart(pos_pid)
                mesh.OpSetPartCenter_numpy(pid, mesh.PGetPos_numpy(pos_pid))
    for i in sorted(pos_pids, reverse=True):
        mesh.OpDeletePart(i)
    return mesh

# -------------------------------------- workload
if CONFIG["fce_version"] not in ['3', '4', '4m', '4M']:
    print("requires fce_version = '3'|'4'|'4M' (received {})".format(CONFIG["fce_version"]))
    quit()
if CONFIG["center_parts"] not in [0, 1]:
    print("requires center_parts = 0|1 (received", CONFIG["center_parts"], ")")
    quit()
if CONFIG["material2texpage"] not in [0, 1]:
    print("requires material2texpage = 0|1 (received", CONFIG["material2texpage"], ")")
    quit()
if CONFIG["material2triagflag"] not in [0, 1]:
    print("requires material2triagflag = 0|1 (received", CONFIG["material2triagflag"], ")")
    quit()

# Import OBJ
reader = LoadObj(filepath_obj_input)
attrib = reader.GetAttrib()
print("attrib.vertices = ", len(attrib.vertices), int(len(attrib.vertices) / 3))
print("attrib.normals = ", len(attrib.normals))
print("attrib.texcoords = ", len(attrib.texcoords))
objverts = GetVerts(reader)
objnorms = GetNormals(reader)
objtexcoords = GetTexcoords(reader)
PrintShapes(reader)
shapenames = GetShapeNames(reader)

# Transform geometric data, load as fcecodec.Mesh
mesh = fcecodec.Mesh()
for i in range(len(shapenames)):
    print("s_name", shapenames[i])
    mesh = ShapeToPart(reader,
                       mesh, objverts, objnorms, objtexcoords, shapenames[i],
                       CONFIG["material2texpage"], CONFIG["material2triagflag"])
mesh = CopyDamagePartsVertsToPartsVerts(mesh)
mesh = PartsToDummies(mesh)
if CONFIG["center_parts"] == 1:
    mesh = CenterParts(mesh)

# Write FCE
# mesh.PrintInfo()
WriteFce(CONFIG["fce_version"], mesh, filepath_fce_output, center_parts=0)
print(flush=True)
PrintFceInfo(filepath_fce_output)