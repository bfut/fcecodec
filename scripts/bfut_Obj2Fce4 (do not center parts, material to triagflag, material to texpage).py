# Copyright (C) 2021 and later Benjamin Futasz <https://github.com/bfut>
#
# This software is provided 'as-is', without any express or implied
# warranty.  In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.
"""
    bfut_Obj2Fce4 (do not center parts, material to triagflag, material to texpage).py

DESCRIPTION
    import OBJ file using tinyobjloader, export as FCE file using fcecodec

    make sure the OBJ has valid links to its MTL materials file
    all faces must be triangles

HOW TO USE
    python "bfut_Obj2Fce4 (do not center parts, material to triagflag, material to texpage).py" /path/to/model.obj

TUTORIAL
    check <doc_Obj2Fce.md> in fcecodec/scripts/

REQUIRES
    installing fcecodec <https://github.com/bfut/fcecodec>
    installing numpy <https://numpy.org/install>
    installing tinyobjloader <https://github.com/tinyobjloader/tinyobjloader/tree/master/python>
"""
import argparse
import pathlib
import re
import sys

import fcecodec
import numpy as np
import tinyobjloader

CONFIG = {
    "fce_version"        : "4",  # output format version; expects "keep" or "3"|"4"|"4M" for FCE3, FCE4, FCE4M, respectively
    "center_parts"       : 0,  # localize part vertice positions to part centroid, setting part position (expects 0|1)
    "material2texpage"   : 1,  # maps OBJ face materials to FCE texpages (expects 0|1)
    "material2triagflag" : 1,  # maps OBJ face materials to FCE triangles flag (expects 0|1)
}

# Parse command-line
parser = argparse.ArgumentParser()
parser.add_argument("path", nargs="+", help="file path")
args = parser.parse_args()

filepath_obj_input = pathlib.Path(args.path[0])
output_path_stem = filepath_obj_input.parent / filepath_obj_input.stem
filepath_fce_output = output_path_stem.with_suffix(".fce")


# -------------------------------------- fcecodec wrappers
def PrintFceInfo(path):
    with open(path, "rb") as f:
        buf = f.read()
        fcecodec.PrintFceInfo(buf)
        assert fcecodec.ValidateFce(buf) == 1

def WriteFce(version, mesh, path, center_parts=1, mesh_function=None):
    if mesh_function is not None:  # e.g., HiBody_ReorderTriagsTransparentToLast
        mesh = mesh_function(mesh, version)
    with open(path, "wb") as f:
        if version == "3":
            buf = mesh.IoEncode_Fce3(center_parts)
        elif version == "4":
            buf = mesh.IoEncode_Fce4(center_parts)
        else:
            buf = mesh.IoEncode_Fce4M(center_parts)
        assert fcecodec.ValidateFce(buf) == 1
        f.write(buf)

def GetMeshPartnames(mesh):
    return [mesh.PGetName(pid) for pid in range(mesh.MNumParts)]

def GetPartGlobalOrderVidxs(mesh, pid):
    map_verts = mesh.MVertsGetMap_idx2order
    part_vidxs = mesh.PGetTriagsVidx(pid)
    for i in range(part_vidxs.shape[0]):
        # print(part_vidxs[i], map_verts[part_vidxs[i]])
        part_vidxs[i] = map_verts[part_vidxs[i]]
    return part_vidxs


# -------------------------------------- tinyobjloader wrappers
def LoadObj(filename):
    """src: https://github.com/tinyobjloader/tinyobjloader/blob/master/python/sample.py"""
    reader = tinyobjloader.ObjReader()
    config = tinyobjloader.ObjReaderConfig()
    config.triangulate = False
    ret = reader.ParseFromFile(str(filename), config)
    if ret is False:
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
              f"faces={int(shape.mesh.numpy_indices().shape[0] / 3)}")

def GetShapeNames(reader):
    shapenames = []
    shapes = reader.GetShapes()
    for i in range(len(shapes)):
        shapenames += [shapes[i].name]
    return shapenames

def GetShapeFaces(reader, vertices, normals, texcoords, shapename):
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

    print(shape.name, f"faces={int(s_faces.shape[0] / 3)}")
    print(shape.name, f"normals={int(normals_idxs.shape[0])}")
    print(shape.name, f"texcoords={int(texcoord_idxs.shape[0])}")

    # cannot use np.unique(), as shape may have unreferenced verts
    # example: mcf1/car.viv->car.fce :HB
    vert_selection = np.arange(np.amin(s_faces), np.amax(s_faces) + 1)
    s_verts = vertices.reshape(-1, 3)[ vert_selection ].flatten()

    # Get normals (use vert positions, if no normals for shape)
    # obj: number of verts and normals may differ; fce: each vert has a normal
    print("GetShapeFaces: Get normals")
    norm_selection = np.empty(vert_selection.shape[0], dtype=int)
    map_v_t = np.copy(vert_selection)
    print(f"GetShapeFaces: for i in range{map_v_t.shape[0]}")
    for i in range(map_v_t.shape[0]):
        argwhere = np.argwhere(s_faces == map_v_t[i])
        if len(argwhere) == 0:
            map_v_t[i] = -1
        else:
            map_v_t[i] = argwhere[0]
    print(f"GetShapeFaces: for i in range{map_v_t.shape[0]}")
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

    print("GetShapeFaces: uvuvuv... -> uuuvvv...")
    s_texcs = np.empty(s_NumFaces * 6)
    for i in range(s_NumFaces):
        for j in range(3):
            s_texcs[i*6 + 0*3 + j] = texcoords[texcoord_idxs[i*3 + j] * 2 + 0]
            s_texcs[i*6 + 1*3 + j] = texcoords[texcoord_idxs[i*3 + j] * 2 + 1]

    s_matls = shape.mesh.numpy_material_ids()

    return s_faces, s_verts, s_norms, s_texcs, s_matls


# -------------------------------------- more wrappers
def LocalizeVertIdxs(faces):
    return faces - np.amin(faces)

def GetFlagFromTags(tags):
    flag = 0x0
    for t in tags:
        # if t == "FDEF":
        #     flag += 0x000
        if t == "FMAT":
            flag += 0x001
        elif t == "FHIC":
            flag += 0x002
        elif t == "FNOC":
            flag += 0x004
        elif t == "FSET":
            flag += 0x008
        elif t == "FUN1":
            flag += 0x010
        elif t == "FALW":
            flag += 0x020
        elif t == "FFRW":
            flag += 0x040
        elif t == "FLEW":
            flag += 0x080
        elif t == "FBAW":
            flag += 0x100
        elif t == "FRIW":
            flag += 0x200
        elif t == "FBRW":
            flag += 0x400
        elif t == "FUN2":
            flag += 0x800
    return flag

def GetTexPageFromTags(tags):
    txp = 0x0
    r = re.compile("T[0-9]", re.IGNORECASE)
    for t in tags:
        if r.match(t) is not None:
            try:
                txp = int(t[1:])
            except ValueError:
                print(f"Cannot convert tag {t} to texpage")
    return txp

def ShapeToPart(reader,
                mesh, objverts, objnorms, objtexcoords, request_shapename,
                material2texpage, material2triagflag):
    s_faces, s_verts, s_norms, s_texcs, s_matls = GetShapeFaces(reader,
        objverts, objnorms, objtexcoords, request_shapename)
    print(f"faces:{int(s_faces.shape[0] / 3)}")
    print(f"vert idx range: [{np.amin(s_faces)},{np.amax(s_faces)}]")
    print(f"vertices:{int(s_verts.shape[0] / 3)}")
    print(f"normals:{s_norms.shape[0]}")
    print(f"texcoords:{s_texcs.shape[0]}->{int(s_texcs.shape[0] / 6)}")

    # print(s_faces)
    s_faces = LocalizeVertIdxs(s_faces)

    s_verts[2::3] = -s_verts[2::3]  # flip sign in Z-coordinate
    s_norms[2::3] = -s_norms[2::3]  # flip sign in Z-coordinate
    mesh.IoGeomDataToNewPart(s_faces, s_texcs, s_verts, s_norms)
    mesh.PSetName(mesh.MNumParts - 1, request_shapename)  # shapename to partname

    # map faces material IDs to triangles texpages
    if material2texpage == 1:
        print("mapping faces material names to triangles texpages...")
        num_arts_warning = False
        materials = reader.GetMaterials()
        texps = mesh.PGetTriagsTexpages(mesh.MNumParts - 1)
        for i in range(texps.shape[0]):
            if materials[s_matls[i]].name[:2] == "0x":
                texps[i] = int(materials[s_matls[i]].name[2:], base=16)
            else:
                tags = materials[s_matls[i]].name.split("_")
                texps[i] = GetTexPageFromTags(tags)
            if texps[i] > 0:
                num_arts_warning = True
        # print(type(texps), texps.dtype, texps.shape)
        if num_arts_warning:
            print("Warning: texpage greater than zero is present. FCE3/FCE4 require amending Mesh.NumArts value")
        mesh.PSetTriagsTexpages(mesh.MNumParts - 1, texps)

    # map faces material names to triangles flags iff
    # all material names are integer hex values (strings of the form '0xiii')
    if material2triagflag == 1:
        print("mapping faces material names to triangles flags...")
        materials = reader.GetMaterials()
        tflags = mesh.PGetTriagsFlags(mesh.MNumParts - 1)

        # if material name is hex value, map straight to triag flag
        # if it isn't, treat as string of tags
        for i in range(tflags.shape[0]):
            try:
                # print(materials[s_matls[i]].name, int(materials[s_matls[i]].name[2:], base=16))
                tflags[i] = int(materials[s_matls[i]].name[2:], base=16)
            except ValueError:
                tags = materials[s_matls[i]].name.split("_")
                tflags[i] = GetFlagFromTags(tags)
        mesh.PSetTriagsFlags(mesh.MNumParts - 1, tflags)

        for i in range(len(materials)):
            tmp = materials[i].name
            # print(tmp)
            if tmp[:2] == "0x":
                try:
                    # print(tmp, "->", int(tmp[2:], base=16), f"0x{hex(int(tmp[2:], base=16))}")
                    val = int(tmp[2:], base=16)
                    del val
                except ValueError:
                    print(f"Cannot map faces material name to triangles flags ('{materials[i].name}' is not hex value) 1")
                    break

    return mesh

def CopyDamagePartsVertsToPartsVerts(mesh):
    """
    Copy verts/norms of DAMAGE_<partname> to damaged verts/norms of <partname>
    """
    damgd_pids = []
    part_names = np.array(GetMeshPartnames(mesh), dtype="U64")
    mesh.PrintInfo()
    for damgd_pid in range(mesh.MNumParts):
        if part_names[damgd_pid][:7] == "DAMAGE_":
            pid = np.argwhere(part_names == part_names[damgd_pid][7:])
            # print(damgd_pid, part_names[damgd_pid], pid, len(pid))
            if len(pid) < 1:
                continue
            pid = pid[0][0]
            print(f"copy verts/norms of {part_names[damgd_pid]} to damaged verts/norms of {part_names[pid]}")
            damgd_part_vidxs = GetPartGlobalOrderVidxs(mesh, damgd_pid)
            part_vidxs = GetPartGlobalOrderVidxs(mesh, pid)

            # cannot use np.unique(), as shape may have unreferenced verts
            # example: mcf1/car.viv->car.fce :HB
            # requires that OBJ parts verts are ordered in non-overlapping
            # ranges and that verts 0 and last, resp., are referenced
            damgd_part_vidxs = np.arange(np.amin(damgd_part_vidxs), np.amax(damgd_part_vidxs) + 1)
            part_vidxs = np.arange(np.amin(part_vidxs), np.amax(part_vidxs) + 1)

            dn = mesh.MVertsDamgdNorms.reshape((-1, 3))
            dv = mesh.MVertsDamgdPos.reshape((-1, 3))
            dn[part_vidxs] = mesh.MVertsNorms.reshape((-1, 3))[damgd_part_vidxs]
            dv[part_vidxs] = mesh.MVertsPos.reshape((-1, 3))[damgd_part_vidxs]
            mesh.MVertsDamgdNorms = dn.flatten()
            mesh.MVertsDamgdPos = dv.flatten()
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
            print(f"convert part {i} '{mesh.PGetName(i)}' to dummy {mesh.PGetName(i)[9:]}")
            pids += [i]
            dms += [mesh.PGetName(i)[9:]]
            mesh.OpCenterPart(i)
            dms_pos = np.append(dms_pos, mesh.PGetPos(i))
    for i in reversed(pids):
        mesh.OpDeletePart(i)
    print(pids, dms, dms_pos)
    mesh.MSetDummyNames(dms)
    mesh.MSetDummyPos(dms_pos)
    return mesh

def SetAnimatedVerts(mesh):
    """
    Set <partname> verts movable iff contained in ANIMATED_##_<partname> cuboid
    hull, where # is digit
    """
    print("SetAnimatedVerts(mesh):")
    vpos = mesh.MVertsPos.reshape((-1, 3))
    animation_flags = mesh.MVertsAnimation
    anim_pids = []
    part_names = GetMeshPartnames(mesh)
    r = re.compile("ANIMATED_[0-9][0-9]_", re.IGNORECASE)
    for i, part_name in zip(range(mesh.MNumParts), part_names):
        part_anim_pids = []
        if r.match(part_name[:12]) is None:
            print(i, part_name)
            for j, anim_name in zip(range(mesh.MNumParts), part_names):
                if anim_name[12:] == part_name and r.match(anim_name[:12]) is not None:
                    anim_pids += [j]
                    part_anim_pids += [j]
                    print("  ", j, anim_name)
            print("animation flag maps:", part_anim_pids)
            if len(part_anim_pids) > 0:
                part_vidxs = GetPartGlobalOrderVidxs(mesh, i)
                # cannot use np.unique(), as shape may have unreferenced verts
                # example: mcf1/car.viv->car.fce :HB
                # requires that OBJ parts verts are ordered in non-overlapping
                # ranges and that verts 0 and last, resp., are referenced
                part_vidxs = np.arange(np.amin(part_vidxs), np.amax(part_vidxs) + 1)
                part_animation_flags = animation_flags[part_vidxs]
                part_vpos = vpos[part_vidxs]
                part_animation_flags[:] = 0x4
                print(part_vidxs.shape, part_animation_flags.shape)
                for j in part_anim_pids:
                    part_anim_vidxs = GetPartGlobalOrderVidxs(mesh, j)
                    part_anim_vidxs = np.arange(np.amin(part_anim_vidxs), np.amax(part_anim_vidxs) + 1)
                    anim_vpos = vpos[part_anim_vidxs]
                    print(anim_vpos.shape, np.amin(anim_vpos, axis=0), np.amax(anim_vpos, axis=0))
                    cuboid_min = np.amin(anim_vpos, axis=0)
                    cuboid_max = np.amax(anim_vpos, axis=0)
                    for n in range(part_vpos.shape[0]):
                        # part_vpos is ndarray, but make static analysis happy
                        if False not in np.array(part_vpos[n] > cuboid_min) \
                        and False not in np.array(part_vpos[n] < cuboid_max):
                            part_animation_flags[n] = 0x0
                            print(n, part_vpos[n], part_animation_flags[n])
                    animation_flags[part_vidxs] = part_animation_flags
                print(np.unique(animation_flags[part_vidxs]))
    print(anim_pids, np.unique(animation_flags))
    mesh.MVertsAnimation = animation_flags
    for i in sorted(anim_pids, reverse=True):
        mesh.OpDeletePart(i)
    return mesh

def CenterParts(mesh):
    """
    Center part <partname> either to centroid, or if present to centroid of part POSITION_<partname>
    """
    pos_pids = []
    part_names = GetMeshPartnames(mesh)
    pos_parts = {}
    for name, i in zip(part_names, range(mesh.MNumParts)):
        if name[:9] == "POSITION_":
            pos_parts[name[9:]] = i
    print(pos_parts)
    for pid in range(mesh.MNumParts):
        if part_names[pid][:9] != "POSITION_":
            if part_names[pid] not in pos_parts:  # POSITION_<partname> not available
                print(f"center {part_names[pid]} to local centroid")
                mesh.OpCenterPart(pid)
            else:                                 # POSITION_<partname> is available
                pos_pid = pos_parts[part_names[pid]]
                print(f"center {part_names[pid]} to centroid of {part_names[pos_pid]}")
                pos_pids += [pos_pid]
                mesh.OpCenterPart(pos_pid)
                mesh.OpSetPartCenter(pid, mesh.PGetPos(pos_pid))
    for i in sorted(pos_pids, reverse=True):
        mesh.OpDeletePart(i)
    return mesh

#
def main():
    if CONFIG["fce_version"] not in ["3", "4", "4m", "4M"]:
        print(f"requires fce_version = '3'|'4'|'4M' (received {CONFIG['fce_version']})")
        sys.exit()
    if CONFIG["center_parts"] not in [0, 1]:
        print(f"requires center_parts = 0|1 (received {CONFIG['center_parts']})")
        sys.exit()
    if CONFIG["material2texpage"] not in [0, 1]:
        print(f"requires material2texpage = 0|1 (received {CONFIG['material2texpage']})")
        sys.exit()
    if CONFIG["material2triagflag"] not in [0, 1]:
        print(f"requires material2triagflag = 0|1 (received {CONFIG['material2triagflag']})")
        sys.exit()

    # Import OBJ
    reader = LoadObj(filepath_obj_input)
    attrib = reader.GetAttrib()
    print(f"attrib.vertices = {len(attrib.vertices)}, {int(len(attrib.vertices) / 3)}")
    print(f"attrib.normals = {len(attrib.normals)}")
    print(f"attrib.texcoords = {len(attrib.texcoords)}")
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
    mesh = SetAnimatedVerts(mesh)
    if CONFIG["center_parts"] == 1:
        mesh = CenterParts(mesh)

    # Write FCE
    # mesh.PrintInfo()
    WriteFce(CONFIG["fce_version"], mesh, filepath_fce_output, center_parts=0)
    print(flush=True)
    PrintFceInfo(filepath_fce_output)
    print(f"filepath_fce_output={filepath_fce_output}")

if __name__ == "__main__":
    main()

