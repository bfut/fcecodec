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
    bfut_MergeParts (FceM to Fce4, keep version).py - parts documented in fcelib_fcetypes.h

USAGE
    python "bfut_MergeParts (FceM to Fce4, keep version).py" /path/to/model.fce [/path/to/output.fce]
    python "bfut_MergeParts (FceM to Fce4, keep version).py" 0|1  0|1  0|1|2  /path/to/model.fce [/path/to/output.fce]

    Assumes that partnames conform to Fce4M.

    Call script with 0 or 3 integer parameters.
        arg0: 0|1 for No or Yes to select "chopped_roof" TODO (reverts to normal roof if no chopped roof is available)
        arg1: 0|1 for No or Yes to select "convertible"  TODO (1 overrides chopped_roof parameter unless no convertible top is available)
        arg2: 0|1|2 for None or "small" or "big" to select "hood_scoop" size TODO (reverts to normal roof if no chopped roof is available)

    Examples:
        1 0 0 is part.fce with chopped roof and no hood scoop
        1 1 1 and 0 1 1, both yield part.fce with convertible top and small hood scoop

REQUIRES
    installing <https://github.com/bfut/fcecodec>
"""
import argparse
import pathlib
import sys

import fcecodec
import numpy as np

CONFIG = {
    "fce_version"  : "keep",  # output format version; expects "keep" or "3"|"4"|"4M" for FCE3, FCE4, FCE4M, respectively
    "center_parts" : False,  # localize part vertice positions to part centroid, setting part position (expects 0|1)
    "chopped_roof" : "0",  # chopped roof parts or not (expects "0"|"1")
    "convertible" : "0",  # convertible or not, if "1" overrides chopped_roof (expects "0"|"1")
    "hood_scoop" : "0",  # no hood scoop, small hood scoop or big hood scoop (expects "0"|"1"|"2")
    "merge_side_windows" : 0,  # if 1, merge :Hswin or :Hswinchop to :Hbody
    "delete_multi_texpage_triags" : True,  # default=True; delete triangles that have texpage > 0x00, will appear textureless in car.fce in FCE3 and FCE4
}


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

def WriteFce(version, mesh, path, center_parts=True, mesh_function=None):
    if mesh_function is not None:  # e.g., HiBody_ReorderTriagsTransparentToLast
        mesh = mesh_function(mesh, version)
    with open(path, "wb") as f:
        if version in ("3", 3):
            buf = mesh.IoEncode_Fce3(center_parts)
        elif version in ("4", 4):
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


# -------------------------------------- more functions
def FlipTriangleFlag(mesh: fcecodec.Mesh, pid: int, flag: int, on=False, condition: int=None, verbose=True):
    """
        Expects 'flag' is power of 2 (e.g., 2, 4, 8, ...)

        If 'condition' is not None, flip only where triangle_flag & 0x7FFFFFFF >= condition
    """
    assert flag == int(np.sqrt(flag)**2)
    if verbose:
        print(f"FlipTriangleFlag: pid={pid} flag={bin(flag)} on={on} condition={bin(condition)}")
    tflags = mesh.PGetTriagsFlags(pid)
    if not on:
        flag = ~flag
    if condition is None:
        tflags &= flag
    else:
        tflags = np.where(tflags & 0x7FFFFFFF >= condition, tflags & flag, tflags)
    mesh.PSetTriagsFlags(pid, tflags)
    return mesh

def DeleteUnwantedParts(mesh, fce4m_merge_map, chopped_roof, convertible, hood_scoop, side_windows: bool):
    to_be_deleted_parts = []

    if convertible == 1 and chopped_roof == 0:
        print("Convertible selected (hence no chopped roof, no cage)")
        to_be_deleted_parts = [
            ":Hcage",
            ":Hcagechop",
            ":Hshieldchop",
            # ":Hswin",
            ":Hswinchop",
            ":Htop",
            ":Htopchop"
        ]
    elif convertible == 0 and chopped_roof == 0:
        print("Default roof selected")
        to_be_deleted_parts = [
            ":Hcagechop",
            ":Hshieldchop",
            ":Hswinchop",
            ":Htopchop",
            ":Hconvertible"
        ]
    elif convertible == 0 and chopped_roof == 1:
        print("Chopped roof selected")
        to_be_deleted_parts = [
            ":Hcage",
            ":Hshield",
            ":Hswin",
            ":Htop",
            ":Hconvertible"
        ]
    elif chopped_roof == 1 and convertible == 1:
        raise ValueError("chopped_roof == 1 and convertible == 1")
    else:
        raise ValueError("chopped_roof or convertible, not in [0, 1]")

    if hood_scoop == 0:
        print("No hood scoop selected")
        to_be_deleted_parts += [
            ":Hhoodhole",
            ":Hscoopsmall",
            ":Hscooplarge"
        ]
    elif hood_scoop == 1:
        print("Small hood scoop selected")
        to_be_deleted_parts += [
            ":Hhood",
            ":Hscooplarge"
        ]
    elif hood_scoop == 2:
        print("Big hood scoop selected")
        to_be_deleted_parts += [
            ":Hhood",
            ":Hscoopsmall"
        ]
    else:
        raise ValueError("chopped_roof not in [0, 1, 2]")

    if side_windows == 1:
        print("Keep side-windows selected")
    elif side_windows == 0:
        print("No side-windows selected")
        to_be_deleted_parts += [
            ":Hswin",
            ":Hswinchop",
        ]

    to_be_deleted_parts = set(to_be_deleted_parts)

    for pn in to_be_deleted_parts:
        fce4m_merge_map.pop(pn, None)

    for pid in reversed(range(mesh.MNumParts)):
        pn = mesh.PGetName(pid)
        if pn in to_be_deleted_parts:
            print(f"to be deleted part {pn}")
    return mesh, fce4m_merge_map


def PrintMeshParts(mesh, fce4m_merge_map):
    print("pid  PART NAME                        MERGE TO PART")
    for pid in range(mesh.MNumParts):
        print(f"{pid:<4} {mesh.PGetName(pid):<32} {fce4m_merge_map.get(mesh.PGetName(pid), ''):<24}")


#
def main():
    # Parse command-line
    parser = argparse.ArgumentParser()
    parser.add_argument("path", nargs="+", help="file path")
    args = parser.parse_args()

    # Handle paths: 0 or 2 parameters, mandatory inpath, optional outpath
    arg_ofs = 0
    if not pathlib.Path(args.path[arg_ofs]).is_file():
        arg_ofs += 3
    filepath_fce_input = pathlib.Path(args.path[arg_ofs + 0])
    if len(args.path) < arg_ofs + 2:
        filepath_fce_output = filepath_fce_input.parent / (filepath_fce_input.stem + "_out" + filepath_fce_input.suffix)
    else:
        filepath_fce_output = pathlib.Path(args.path[arg_ofs + 1])
    if arg_ofs > 0:
        print(f"args={args}")


    # Process configuration / parameters
    if CONFIG["fce_version"] == "keep":
        fce_outversion = str(GetFceVersion(filepath_fce_input))
        if fce_outversion == "5":
            fce_outversion = "4M"
    else:
        fce_outversion = CONFIG["fce_version"]

    if arg_ofs > 1:
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
        convertible = int(CONFIG["convertible"])
        hood_scoop = int(CONFIG["hood_scoop"])

    # Load FCE
    mesh = fcecodec.Mesh()
    mesh = LoadFce(mesh, filepath_fce_input)

    if mesh.MNumParts > 1:
        # Add dummies from :PPLicense
        dmn = mesh.MGetDummyNames()
        pid = GetMeshPartnameIdx(mesh, ":PPlicense")
        if pid >= 0:
            dmvpos = mesh.PGetPos(pid)
            for nn in [ ":LICENSE", ":LICMED", ":LICLOW" ]:
                dmn += [nn]
                dv = mesh.MGetDummyPos()
                dv = np.append(dv, dmvpos)
                mesh.MSetDummyPos(dv)
            mesh.MSetDummyNames(dmn)

        fce4m_rename_map = {
            ":Hbody": ":HB",
            ":Hdashlight" : ":ODL",
            ":Hinterior": ":OC",
            ":Hheadlight": ":OL",
            ":Hconvertible": ":OT",
            ":Hsteer": ":OD",
            ":Hlbrake" : ":OLB",
            ":Hrbrake" : ":ORB",
            ":Hlmirror" : ":OLM",
            ":Hrmirror" : ":ORM",
        }
        fce4m_merge_map = {
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

        mesh, fce4m_merge_map = DeleteUnwantedParts(mesh, fce4m_merge_map, chopped_roof, convertible, hood_scoop, CONFIG["merge_side_windows"])
        print(f"fce4m_merge_map={fce4m_merge_map}")
        PrintMeshParts(mesh, fce4m_merge_map)


        # mesh = FlipTriangleFlag(mesh, GetMeshPartnameIdx(mesh, ":Hbody"), flag=0x8, on=True)
        # mesh = FlipTriangleFlag(mesh, GetMeshPartnameIdx(mesh, ":Hbody"), flag=0x2, on=False)

        # mesh, fce4m_merge_map = ApplyMergeMap(mesh, fce4m_merge_map)
        print("Special case: merge :Hbody parts such that semi-transparent triangles have largest indexes")
        Hbody_order = [
            ":Hbody",
            ":Hhood",
            ":Hhoodhole",
            ":Hscoopsmall",
            ":Hscooplarge",
            ":Hskirt",
            ":Hskirtwell",
            ":Hwheelwell",
            ":Htop",
            ":Htopchop",
            ":Hshield",
            ":Hshieldchop",
            ":Hswin",
            ":Hswinchop",
        ]
        mesh_partnames = GetMeshPartnames(mesh)
        for idx in reversed(range(len(Hbody_order))):
            if Hbody_order[idx] not in fce4m_merge_map or Hbody_order[idx] not in mesh_partnames:
                print("Hbody_order: delete", Hbody_order[idx])
                fce4m_merge_map.pop(Hbody_order[idx], None)
                Hbody_order.pop(idx)

        print(f"Hbody_order={Hbody_order}")

        fce4m_merge_map.pop(Hbody_order[0], None)

        if len(fce4m_merge_map) > 0:
            delete_parts = []
            pid = GetMeshPartnameIdx(mesh, Hbody_order[0])
            delete_parts += [pid]
            new_pid = mesh.OpCopyPart(pid)
            if len(Hbody_order) > 1:
                delete_parts += [new_pid]

            for idx in range(1, len(Hbody_order)):
                fce4m_merge_map.pop(Hbody_order[idx], None)

                pid = GetMeshPartnameIdx(mesh, Hbody_order[idx])
                new_pid = mesh.OpMergeParts(new_pid, pid)
                delete_parts += [pid, new_pid]
                print(f"idx={idx} partname={Hbody_order[idx]} pid={pid} new_pid={new_pid}")
            print(f"delete_parts={delete_parts}")
            delete_parts.pop()  # last part is our target and must be kept
            print(f"delete_parts={delete_parts}")
            # PrintMeshParts(mesh, fce4m_merge_map)
            # print(f"rename {mesh.PGetName(new_pid)} to :HB")
            mesh.PSetName(new_pid, ":Hbody")
            PrintMeshParts(mesh, fce4m_merge_map)

            print(f"remaining fce4m_merge_map={fce4m_merge_map}")
            def ApplyMergeMap_Not_HB(mesh: fcecodec.Mesh, fce4m_merge_map: dict, delete_parts: list):
                """
                    For non-hi body merging, triangle order is not significant
                """
                target_parts = set(fce4m_merge_map.values())
                print(target_parts)
                for target_part in target_parts:
                    original_pid = GetMeshPartnameIdx(mesh, target_part)
                    if original_pid < 0:
                        continue
                    for pname, target in fce4m_merge_map.items():
                        if target == target_part:
                            pid = GetMeshPartnameIdx(mesh, pname)
                            if pid < 0:
                                continue
                            new_pid = GetMeshPartnameIdx(mesh, target)
                            if pid != new_pid:
                                delete_parts += [original_pid]
                                new_pid = mesh.OpMergeParts(new_pid, pid)
                                delete_parts += [pid, new_pid]
                                print(f"merged partname={pname} target={target} pid={pid} new_pid={new_pid}")
                                print(f"delete_parts={delete_parts} + [{pid}, {new_pid}]")
                    mesh.PSetName(new_pid, target_part)
                    delete_parts.pop()  # last part is our target and must be kept
                    print(f"delete_parts={delete_parts}")
                return delete_parts, {}
            delete_parts, fce4m_merge_map = ApplyMergeMap_Not_HB(mesh, fce4m_merge_map, delete_parts)
            PrintMeshParts(mesh, fce4m_merge_map)

            # Cleanup: delete and rename
            delete_parts = sorted(set(delete_parts), reverse=True)
            print(f"delete_parts={delete_parts}")
            assert delete_parts[0] < mesh.MNumParts
            for pid in reversed(range(mesh.MNumParts)):
                if pid in delete_parts:
                    print(f"deleting part {pid} '{mesh.PGetName(pid)}'")
                    mesh.OpDeletePart(pid)
                else:
                    tmp = fce4m_rename_map.get(mesh.PGetName(pid))
                    if not tmp is None:
                        print(f"rename part {pid} '{mesh.PGetName(pid)}' to '{tmp}'")
                        mesh.PSetName(pid, tmp)


        if CONFIG["delete_multi_texpage_triags"]:
            # Delete triangles with texpage > 0x0 (texture-less for car.fce)
            for pid in reversed(range(mesh.MNumParts)):
                texp = mesh.PGetTriagsTexpages(pid)
                x = np.argwhere(texp > 0x0)
                assert mesh.OpDeletePartTriags(pid, x)
            assert mesh.OpDelUnrefdVerts()

    WriteFce(fce_outversion, mesh, filepath_fce_output, CONFIG["center_parts"])
    PrintFceInfo(filepath_fce_output)
    print(f"FILE = {filepath_fce_output}", flush=True)


if __name__ == "__main__":
    main()
