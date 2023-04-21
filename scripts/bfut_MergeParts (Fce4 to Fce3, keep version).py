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

USAGE
    python "bfut_MergeParts (Fce4 to Fce3, keep version).py" /path/to/model.fce [/path/to/output.fce]
    python "bfut_MergeParts (Fce4 to Fce3, keep version).py" 0|1  0|1  /path/to/model.fce [/path/to/output.fce]

    Call script with 0 or 2 integer parameters.
    Assumes that partnames conform to FCE4.

    Call script with 0 or 2 integer parameters.
        arg0: 0|1 for No or Yes to delete or keep convertible part :OT
        arg1: 0|1 for "opaque"|"transparent" to change semi-transparency flag of all triangles, "opaque" removes all transparency flags

    Examples:
        0 1
        1 1
        0 0
        1 0

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
    "center_parts" : True,  # localize part vertice positions to part centroid, setting part position (expects 0|1)
    "convertible" : "0",  # if "0", delete :OT (top)
    "transparent_windows" : "1",  # default="1"; if "0", do not keep interior (:OC), high body window triangles are not semi-transparent
    "resize_factor" : 1.1,  # 1.1 and 1.2 give good results for vanilla FCE4/FCE4M models; unchanged if 1.0 or smaller than 0.1
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


# -------------------------------------- script functions
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


#
def main():
    # Parse command-line
    parser = argparse.ArgumentParser()
    parser.add_argument("path", nargs="+", help="file path")
    args = parser.parse_args()

    # Handle paths: 0 or 2 parameters, mandatory inpath, optional outpath
    arg_ofs = 0
    if not pathlib.Path(args.path[arg_ofs]).is_file():
        arg_ofs += 2
    filepath_fce_input = pathlib.Path(args.path[arg_ofs + 0])
    if len(args.path) < arg_ofs + 2:
        filepath_fce_output = filepath_fce_input.parent / (filepath_fce_input.stem + "_out" + filepath_fce_input.suffix)
    else:
        filepath_fce_output = pathlib.Path(args.path[arg_ofs + 1])

    # Process configuration / parameters
    if CONFIG["fce_version"] == "keep":
        fce_outversion = str(GetFceVersion(filepath_fce_input))
        if fce_outversion == "5":
            fce_outversion = "4M"
    else:
        fce_outversion = CONFIG["fce_version"]

    if arg_ofs > 1:
        convertible_opt = {
            "0": 0,
            "1": 1,
            "convertible" : 1,
        }
        transparent_windows_opt = {
            "0": 0,
            "1": 1,
            "opaque" : 0,
            "transparent" : 1,
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
        fce4_canonical_partnames = [
            # car.fce
            ":HB",
            ":MB",
            ":LB",
            ":TB",
            ":OT",
            ":OL",
            ":OS",
            ":OLB",
            ":ORB",
            ":OLM",
            ":ORM",
            ":OC",
            ":ODL",
            ":OH",
            ":OD",
            ":OND",
            ":HLFW",
            ":HRFW",
            ":HLMW",
            ":HRMW",
            ":HLRW",
            ":HRRW",
            ":MLFW",
            ":MRFW",
            ":MLMW",
            ":MRMW",
            ":MLRW",
            ":MRRW",
            # hel.fce
            "'body'",
            "'main'",
            "'tail'",
            ":LB",
            ":Lmain",
            ":Ltail",
        ]
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
        if len(Hbody_order) > 0:
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

        delete_parts = []
        if len(fce4_fuse_map) > 0:
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
        PrintMeshParts(mesh, fce4_fuse_map)

        print(f"remaining fce4_fuse_map={fce4_fuse_map}")

        # Cleanup: delete
        delete_parts = sorted(delete_parts, reverse=True)
        print(f"delete_parts={delete_parts}")
        assert delete_parts[0] < mesh.MNumParts
        for pid in reversed(range(mesh.MNumParts)):
            pn = mesh.PGetName(pid)
            if pid in delete_parts or not pn in fce4_canonical_partnames:
                print(f"deleting part {pid} '{pn}'")
                mesh.OpDeletePart(pid)

        # Optionally remove all semi-transparency
        if not transparent_windows:
            print("Selected opaque: remove semi-transparency, if window")
            pid = GetMeshPartnameIdx(mesh, ":HB")
            mesh = FlipTriangleFlag(mesh, pid, flag=0x8, condition=0x010, on=False)  # remove semi-transparency, if window

        # Resize
        resize_factor = float(CONFIG["resize_factor"])
        if resize_factor > 0.0 and abs(resize_factor) > 0.1:
            # for pid in reversed(range(mesh.MNumParts)):
            #     mesh.OpCenterPart(pid)
            #     # mesh.PSetPos(pid, [0, 0, 0])
            v = mesh.MVertsPos  # xyzxyzxyz...
            mesh.MVertsPos = v * resize_factor
            dv = mesh.MGetDummyPos()  # xyzxyzxyz...
            mesh.MSetDummyPos(dv * resize_factor)

    WriteFce(fce_outversion, mesh, filepath_fce_output, CONFIG["center_parts"])
    PrintFceInfo(filepath_fce_output)
    print(f"FILE = {filepath_fce_output}", flush=True)


if __name__ == "__main__":
    main()
