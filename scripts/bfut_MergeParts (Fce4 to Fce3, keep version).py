# Copyright (C) 2022 and later Benjamin Futasz <https://github.com/bfut>
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
    bfut_MergeParts (Fce4 to Fce3, keep version).py - parts documented in fcelib_fcetypes.h

HOW TO USE
    python "bfut_MergeParts (Fce4 to Fce3, keep version).py" [0|1 0|1] /path/to/model.fce

    Note: Script assumes that partnames conform to Fce4

REQUIRES
    installing <https://github.com/bfut/fcecodec>
"""
import argparse
import pathlib
import sys

import fcecodec

CONFIG = {
    "fce_version"  : "keep",  # output format version; expects "keep" or "3"|"4"|"4M" for FCE3, FCE4, FCE4M, respectively
    "center_parts" : 0,  # localize part vertice positions to part centroid, setting part position (expects 0|1)
    "convertible" : "0",  # if "0", do not keep top (:OT)
    "transparent_windows" : "0",  # if "0", do not keep interior (:OC), set all triangles to opaque
}

# Parse command-line
parser = argparse.ArgumentParser()
parser.add_argument("path", nargs="+", help="file path")
args = parser.parse_args()

i = 0
if not pathlib.Path(args.path[i]).is_file():
    i += 2
filepath_fce_input = pathlib.Path(args.path[i + 0])

if i + 1 >= len(args.path):
    filepath_fce_output = filepath_fce_input.parent / (filepath_fce_input.stem + "_out" + filepath_fce_input.suffix)
else:
    filepath_fce_output = pathlib.Path(args.path[i + 1])

# -------------------------------------- wrappers
def GetFceVersion(path):
    with open(path, "rb") as f:
        version = fcecodec.GetFceVersion(f.read(0x2038))
        assert version > 0
        return version

def PrintFceInfo(path):
    with open(path, "rb") as f:
        buf = f.read()
        fcecodec.PrintFceInfo(buf)
        assert fcecodec.ValidateFce(buf) == 1

def LoadFce(mesh, path):
    with open(path, "rb") as f:
        mesh.IoDecode(f.read())
        assert mesh.MValid() is True
        return mesh

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

def GetMeshPartnameIdx(mesh, partname):
    for pid in range(mesh.MNumParts):
        if mesh.PGetName(pid) == partname:
            return pid
    print(f"GetMeshPartnameIdx: Warning: cannot find \"{partname}\"")
    return -1


#
def DeleteUnwantedParts4(mesh, fce_part_fuse_map, convertible, transparent_windows):
    to_be_deleted_parts = []

    if transparent_windows == 0:
        print("No transparent triangles selected (hence no interior)")
        to_be_deleted_parts += [ ":OC", ":ODL", ":OH", ":OD", ":OND" ]
        for pn in to_be_deleted_parts:
            fce_part_fuse_map.pop(pn, None)
    elif transparent_windows != 1:
        raise ValueError("transparent_windows, not in [0, 1]")

    if convertible == 0:
        print("Delete top (:OT) selected")
        to_be_deleted_parts += [ ":OT" ]
        fce_part_fuse_map.pop(":OT", None)
    elif convertible != 1:
        raise ValueError("convertible, not in [0, 1]")

    for pid in reversed(range(mesh.MNumParts)):
        if mesh.PGetName(pid) in to_be_deleted_parts:
            print(f"deleting part {mesh.PGetName(pid)}")
    return mesh, fce_part_fuse_map


def PrintMeshParts(mesh, fuse_map):
    print("pid  PART NAME                        FUSE TO PART")
    for pid in range(mesh.MNumParts):
        print(f"{pid:<4} {mesh.PGetName(pid):<32} {fuse_map.get(mesh.PGetName(pid), ''):<24}")

# def AssertPartsOrder(mesh, part_names_sorted):
#     for pid in range(mesh.MNumParts):
#         if mesh.PGetName(pid) != part_names_sorted[pid]:
#             PrintMeshParts(mesh, part_names_sorted)
#             raise AssertionError (f"pid={pid} {mesh.PGetName(pid)} != {part_names_sorted[pid]}")

def RemoveSemiTransparencyTriagFlagFromWindows(mesh, pid):
    flags = mesh.PGetTriagsFlags(pid)
    for j in range(flags.shape[0]):
        if flags[j] & 0x7F0 > 0x010:  # only windows to opaque
            # print(f"{hex(flags[i])} -> {hex(flags[i] & 0xFFFFFFF7)}")
            flags[j] = flags[j] & 0xFFFFFFF7
    mesh.PSetTriagsFlags(pid, flags)


def main():
    # Process configuration / parameters
    if CONFIG["fce_version"] == "keep":
        fce_outversion = str(GetFceVersion(filepath_fce_input))
        if fce_outversion == "5":
            fce_outversion = "4M"
    else:
        fce_outversion = CONFIG["fce_version"]

    print("len", len(args.path) )

    print(args.path[1])

    if i > 1:
        convertible_opt = {
            "0": 0,
            "1": 1,
            "convertible" : 1,
        }
        transparent_windows_opt = {
            "0": 0,
            "1": 1,
            "opaque" : 0,
            "transp" : 1,
        }
        if args.path[0] not in ["0", "1", "convertible"]:
            print(f"requires convertible = 0|1 (received {args.path[0]})")
            sys.exit()
        convertible = convertible_opt[args.path[0]]
        if args.path[1] not in ["0", "1", "opaque", "transp"]:
            print(f"""requires transparent_windows in ["0", "1", "opaque", "transp"] (received {args.path[1]})""")
            sys.exit()
        transparent_windows = transparent_windows_opt[args.path[1]]
    else:
        convertible = int(CONFIG["convertible"])
        transparent_windows = int(CONFIG["transparent_windows"])

    # Load FCE
    mesh = fcecodec.Mesh()
    mesh = LoadFce(mesh, filepath_fce_input)

    if mesh.MNumParts > 1:
        fce4_fuse_map = {
            ":HB": ":HB",
            ":OT": ":HB",
            # ":OL": ":OL",
            ":OS": ":HB",
            ":OLB": ":HB",
            ":ORB": ":HB",
            ":OLM": ":HB",
            ":ORM": ":HB",
            ":OC": ":HB",
            ":OH": ":HB",
            ":OD": ":HB",
        }

        mesh, fce4_fuse_map = DeleteUnwantedParts4(mesh, fce4_fuse_map, convertible, transparent_windows)
        print(f"fce4_fuse_map={fce4_fuse_map}")
        PrintMeshParts(mesh, fce4_fuse_map)

        # mesh, fce4_fuse_map = ApplyFuseMap(mesh, fce4_fuse_map)
        print("Special case: fuse :HB parts such that window triangles have largest indexes")
        Hbody_order = [
            ":OS",                # spoiler
            ":OC", ":OH", ":OD",   # interior + driver
            ":OLB", ":ORB",         # front disc brakes
            ":ORM", ":OLM",        # side mirrors
            ":HB",                  # hi body
            ":OT",                 # hardtop / convertible top
        ]
        mesh_partnames = GetMeshPartnames(mesh)
        for idx in reversed(range(len(Hbody_order))):
            if Hbody_order[idx] not in fce4_fuse_map or Hbody_order[idx] not in mesh_partnames:
                print("Hbody_order: delete", Hbody_order[idx])
                fce4_fuse_map.pop(Hbody_order[idx], None)

                pid = GetMeshPartnameIdx(mesh, Hbody_order[idx])
                if pid >= 0:
                    print(f"delete part from mesh: {Hbody_order[idx]} ({pid})")
                    mesh.OpDeletePart(pid)
                Hbody_order.pop(idx)


        print(f"Hbody_order={Hbody_order}")

        fce4_fuse_map.pop(Hbody_order[0], None)
        print(f"remaining fce4_fuse_map={fce4_fuse_map}")

        if fce4_fuse_map:
            delete_parts = []
            pid = GetMeshPartnameIdx(mesh, Hbody_order[0])
            delete_parts += [pid]
            new_pid = mesh.OpCopyPart(pid)
            if len(Hbody_order) > 1:
                delete_parts += [new_pid]

            for idx in range(1, len(Hbody_order)):
                fce4_fuse_map.pop(Hbody_order[idx], None)

                pid = GetMeshPartnameIdx(mesh, Hbody_order[idx])
                new_pid = mesh.OpMergeParts(new_pid, pid)
                delete_parts += [pid, new_pid]
                print(f"idx={idx} partname={Hbody_order[idx]} pid={pid} new_pid={new_pid}")
            print(f"delete_parts={delete_parts}")
            delete_parts.pop()
            print(f"delete_parts={delete_parts}")
            PrintMeshParts(mesh, fce4_fuse_map)
            print(f"renaming part {new_pid} {mesh.PGetName(new_pid)} to :HB")
            mesh.PSetName(new_pid, ":HB")
            # cleanup
            for idx in reversed(range(mesh.MNumParts)):
                if idx in delete_parts:
                    print(f"deleting part {idx} '{mesh.PGetName(idx)}'")
                    mesh.OpDeletePart(idx)
        PrintMeshParts(mesh, fce4_fuse_map)

        print(f"remaining fce4_fuse_map={fce4_fuse_map}")

        if not transparent_windows:
            print("RemoveSemiTransparencyTriagFlagFromWindows()")
            pid = GetMeshPartnameIdx(mesh, ":HB")
            RemoveSemiTransparencyTriagFlagFromWindows(mesh, pid)


        if 0:

            part_names = []
            for pid in reversed(range(mesh.MNumParts)):
                part_names += [mesh.PGetName(pid)]
            part_names_sorted = sorted(part_names, key=lambda x: priority_dic.get(x, 64))

            for target_idx in range(0, len(part_names_sorted)):
                pname = part_names_sorted[target_idx]
                current_idx = GetMeshPartnameIdx(mesh, pname)
                # print(f" {pname} {current_idx} -> {target_idx}")
                while current_idx > target_idx:
                    current_idx = mesh.OpMovePart(current_idx)
            AssertPartsOrder(mesh, part_names_sorted)
            # PrintMeshParts(mesh, part_names_sorted)


    WriteFce(fce_outversion, mesh, filepath_fce_output, CONFIG["center_parts"],
             mesh_function=None)
    PrintFceInfo(filepath_fce_output)
    print(f"FILE = {filepath_fce_output}", flush=True)


if __name__ == "__main__":
    main()
