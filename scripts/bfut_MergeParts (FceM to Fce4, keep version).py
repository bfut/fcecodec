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
#     claim that you wrote the original software. If you use this software
#     in a product, an acknowledgment in the product documentation would be
#     appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#     misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.
"""
    bfut_MergeParts (FceM to Fce4, keep version).py - parts documented in fcelib_fcetypes.h

HOW TO USE
    python "bfut_MergeParts (FceM to Fce4, keep version).py" [0|1 0|1 0|1|2] /path/to/model.fce

    Note: Script assumes that partnames conform to Fce4M

REQUIRES
    installing <https://github.com/bfut/fcecodec>
"""
import argparse
import pathlib
import sys

import fcecodec

CONFIG = {
    "fce_version"  : "keep",  # output format version; expects 'keep' or '3'|'4'|'4M' for FCE3, FCE4, FCE4M, respectively
    "center_parts" : 0,  # localize part vertice positions to part centroid, setting part position (expects 0|1)
    "chopped_roof" : "0",  # chopped roof parts or not (expects "0"|"1")
    "convertible" : "0",  # convertible or not, if "1" overrides chopped_roof (expects "0"|"1")
    "hood_scoop" : "0",  # no hood scoop, small hood scoop or big hood scoop (expects "0"|"1"|"2")
    "fuse_side_windows" : 0,  # if 1, fuse :Hswin and :Hswinchop to :Hbody
}

# Parse command-line
parser = argparse.ArgumentParser()
parser.add_argument("path", nargs="+", help="file path")
args = parser.parse_args()

i = 0
if not pathlib.Path(args.path[i]).is_file():
    i += 3
filepath_fce_input = pathlib.Path(args.path[i + 0])

if i + 1 >= len(args.path):
    filepath_fce_output = filepath_fce_input.parent / (filepath_fce_input.stem + "_out" + filepath_fce_input.suffix)
else:
    filepath_fce_output = pathlib.Path(args.path[i + 1])

# -------------------------------------- wrappers
def GetFceVersion(path):
    with open(path, "rb") as f:
        buf = f.read(0x2038)
        version = fcecodec.GetFceVersion(buf)
        assert version > 0
        return version

def PrintFceInfo(path):
    with open(path, "rb") as f:
        buf = f.read()
        fcecodec.PrintFceInfo(buf)
        assert fcecodec.ValidateFce(buf) == 1

def LoadFce(mesh, path):
    with open(path, "rb") as f:
        fce_buf = f.read()
    assert fcecodec.ValidateFce(fce_buf) == 1
    mesh.IoDecode(fce_buf)
    assert mesh.MValid() is True
    return mesh

def WriteFce(version, mesh, path, center_parts = 1):
    with open(path, "wb") as f:
        # print(version == "3", version == "4", version)
        if version == "3":
            buf = mesh.IoEncode_Fce3(center_parts)
        elif version == "4":
            buf = mesh.IoEncode_Fce4(center_parts)
        else:
            buf = mesh.IoEncode_Fce4M(center_parts)
        assert fcecodec.ValidateFce(buf) == 1
        f.write(buf)


#
def DeleteUnwantedParts(mesh, fce4m_fuse_map, chopped_roof, convertible, hood_scoop):
    to_be_deleted_parts = []

    if convertible == 1 and chopped_roof == 0:
        print("Convertible selected (hence no chopped roof)")
        fce4m_fuse_map.pop(":Hcagechop", None)
        fce4m_fuse_map.pop(":Hshieldchop", None)
        fce4m_fuse_map.pop(":Hswinchop", None)
        fce4m_fuse_map.pop(":Htop", None)
        fce4m_fuse_map.pop(":Htopchop", None)
        to_be_deleted_parts += [ ":Hcagechop", ":Hshieldchop", ":Hswinchop", ":Htop", ":Htopchop" ]
    elif convertible == 0 and chopped_roof == 0:
        print("Default roof selected")
        fce4m_fuse_map.pop(":Hcagechop", None)
        fce4m_fuse_map.pop(":Hshieldchop", None)
        fce4m_fuse_map.pop(":Hswinchop", None)
        fce4m_fuse_map.pop(":Htopchop", None)
        fce4m_fuse_map.pop(":Hconvertible", None)
        to_be_deleted_parts += [ ":Hcagechop", ":Hshieldchop", ":Hswinchop", ":Htopchop", ":Hconvertible" ]
    elif convertible == 0 and chopped_roof == 1:
        print("Chopped roof selected")
        fce4m_fuse_map.pop(":Hcage", None)
        fce4m_fuse_map.pop(":Hshield", None)
        fce4m_fuse_map.pop(":Hswin", None)
        fce4m_fuse_map.pop(":Htop", None)
        fce4m_fuse_map.pop(":Hconvertible", None)
        to_be_deleted_parts += [ ":Hcage", ":Hshield", ":Hswin", ":Htop", ":Hconvertible" ]
    elif chopped_roof == 1 and convertible == 1:
        raise ValueError("chopped_roof == 1 and convertible == 1")
    else:
        raise ValueError("chopped_roof or convertible, not in [0, 1]")

    if hood_scoop == 0:
        print("No hood scoop selected")
        fce4m_fuse_map.pop(":Hhoodhole", None)
        fce4m_fuse_map.pop(":Hscoopsmall", None)
        fce4m_fuse_map.pop(":Hscooplarge", None)
        to_be_deleted_parts += [ ":Hhoodhole", ":Hscoopsmall", ":Hscooplarge" ]
    elif hood_scoop == 1:
        print("Small hood scoop selected")
        fce4m_fuse_map.pop(":Hhood", None)
        fce4m_fuse_map.pop(":Hscooplarge", None)
        to_be_deleted_parts += [ ":Hhood", ":Hscooplarge" ]
    elif hood_scoop == 2:
        print("Big hood scoop selected")
        fce4m_fuse_map.pop(":Hhood", None)
        fce4m_fuse_map.pop(":Hscoopsmall", None)
        to_be_deleted_parts += [ ":Hhood", ":Hscoopsmall" ]
    else:
        raise ValueError("chopped_roof not in [0, 1, 2]")

    for pid in reversed(range(mesh.MNumParts)):
        if mesh.PGetName(pid) in to_be_deleted_parts:
            print(f"deleting part {mesh.PGetName(pid)}")
    return mesh, fce4m_fuse_map


def PrintMeshParts(mesh, fce4m_fuse_map):
    print(f"pid  PART NAME                        FUSE TO PART")
    for pid in range(mesh.MNumParts):
        print(f"{pid:<4} {mesh.PGetName(pid):<32} {fce4m_fuse_map.get(mesh.PGetName(pid), ''):<24}")

def GetMeshPartnames(mesh):
    partnames = []
    for pid in range(mesh.MNumParts):
        partnames += [ mesh.PGetName(pid) ]
    return partnames

def GetMeshPartnameIdx(mesh, partname):
    for pid in range(mesh.MNumParts):
        pn = mesh.PGetName(pid)
        if pn == partname:
            return pid
    return -1


def main():
    # Process configuration / parameters
    if CONFIG["fce_version"] == "keep":
        fce_outversion = str(GetFceVersion(filepath_fce_input))
        if fce_outversion == "5":
            fce_outversion = "4M"
    else:
        fce_outversion = CONFIG["fce_version"]

    if i > 1:
        chopped_roof_opt = {
            "0": 0,
            "1": 1,
            "chopped" : 1,
        }
        convertible_opt = {
            "0": 0,
            "1": 1,
            "convertible" : 1,
        }
        hood_scoop_opt = {
            "0": 0,
            "1": 1,
            "2": 2,
            "none" : 0,
            "small" : 1,
            "big" : 2,
        }
        if args.path[0] not in ["0", "1", "chopped"]:
            print(f"requires chopped_roof = 0|1 (received {args.path[0]})")
            sys.exit()
        chopped_roof = chopped_roof_opt[args.path[0]]
        if args.path[1] not in ["0", "1", "convertible"]:
            print(f"requires convertible = 0|1 (received {args.path[1]})")
            sys.exit()
        convertible = convertible_opt[args.path[1]]
        if convertible == 1:
            print("main: Convertible selected (hence no chopped roof)")
            chopped_roof = 0
        if args.path[2] not in ["0", "1", "2", "none", "small", "big"]:
            print(f"requires hood_scoop = 0|1|2 (received {args.path[2]})")
            sys.exit()
        hood_scoop = hood_scoop_opt[args.path[2]]
    else:
        chopped_roof = int(CONFIG["chopped_roof"])
        hood_scoop = int(CONFIG["hood_scoop"])

    # Load FCE
    mesh = fcecodec.Mesh()
    mesh = LoadFce(mesh, filepath_fce_input)

    if mesh.MNumParts > 1:
        fce4m_fuse_map = {
            ":Hbody": ":Hbody",
            ":Hconvertible": ":Hconvertible",
            # ":Hdashlight": ":Hdashlight",
            # ":Hfenderlight": ,  ?????????????????
            ":Hfirewall": ":Hinterior",
            # ":Hheadlight": ":Hheadlight",
            ":Hhood": ":Hbody",
            ":Hhoodhole": ":Hbody",
            ":Hinterior": ":Hinterior",
            # ":Hlbrake": ":Hlbrake",
            # ":Hrbrake": ":Hrbrake",
            # ":Hlmirror": ":Hlmirror",
            # ":Hrmirror": ":Hrmirror",
            ":Hscoopsmall": ":Hbody",
            ":Hscooplarge": ":Hbody",
            ":Hskirt": ":Hbody",
            ":Hskirtwell": ":Hbody",
            ":Hsteer": ":Hsteer",
            ":Htrans": ":Hinterior",
            ":Hwheelwell": ":Hbody",
            # ":Mwheelwell": ,  # useless
            ":Hcage": ":Hinterior",
            ":Hcagechop": ":Hinterior",
            ":Hshield": ":Hbody",
            ":Hshieldchop": ":Hbody",
            ":Hswin": ":Hbody",
            ":Hswinchop": ":Hbody",
            ":Htop": ":Htop",
            ":Htopchop": ":Htopchop",
        }

        mesh, fce4m_fuse_map = DeleteUnwantedParts(mesh, fce4m_fuse_map, chopped_roof, convertible, hood_scoop)
        print(f"fce4m_fuse_map={fce4m_fuse_map}")
        PrintMeshParts(mesh, fce4m_fuse_map)

        # mesh, fce4m_fuse_map = ApplyFuseMap(mesh, fce4m_fuse_map)
        print("Special case: fuse :Hbody parts such that window triangles have largest indexes")
        Hbody_order = [
            ":Hbody",
            ":Hhood",
            ":Hhoodhole",
            ":Hscoopsmall",
            ":Hscooplarge",
            ":Hskirt",
            ":Hskirtwell",
            ":Hwheelwell",
            ":Hshield",
            ":Hshieldchop",
            ":Hswin",
            ":Hswinchop",
        ]
        mesh_partnames = GetMeshPartnames(mesh)
        for idx in reversed(range(len(Hbody_order))):
            if Hbody_order[idx] not in fce4m_fuse_map or Hbody_order[idx] not in mesh_partnames:
                print("Hbody_order: delete", Hbody_order[idx])
                fce4m_fuse_map.pop(Hbody_order[idx], None)
                Hbody_order.pop(idx)

        print(f"Hbody_order={Hbody_order}")

        fce4m_fuse_map.pop(Hbody_order[0], None)

        if fce4m_fuse_map:
            delete_parts = []
            pid = GetMeshPartnameIdx(mesh, Hbody_order[0])
            delete_parts += [pid]
            new_pid = mesh.OpCopyPart(pid)
            if len(Hbody_order) > 1:
                delete_parts += [new_pid]

            for idx in range(1, len(Hbody_order)):
                fce4m_fuse_map.pop(Hbody_order[idx], None)

                pid = GetMeshPartnameIdx(mesh, Hbody_order[idx])
                new_pid = mesh.OpMergeParts(new_pid, pid)
                delete_parts += [pid, new_pid]
                print(f"idx={idx} partname={Hbody_order[idx]} pid={pid} new_pid={new_pid}")
            print(f"delete_parts={delete_parts}")
            delete_parts.pop()
            print(f"delete_parts={delete_parts}")
            PrintMeshParts(mesh, fce4m_fuse_map)
            mesh.PSetName(new_pid, ":HB")
            # cleanup
            for idx in reversed(range(mesh.MNumParts)):
                if idx in delete_parts:
                    print(f"deleting part {idx} '{mesh.PGetName(idx)}'")
                    mesh.OpDeletePart(idx)
        PrintMeshParts(mesh, fce4m_fuse_map)

        print(f"remaining fce4m_fuse_map={fce4m_fuse_map}")


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


    WriteFce(fce_outversion, mesh, filepath_fce_output, CONFIG["center_parts"])
    PrintFceInfo(filepath_fce_output)
    print("FILE =", filepath_fce_output, flush=True)



if __name__ == "__main__":
    main()
