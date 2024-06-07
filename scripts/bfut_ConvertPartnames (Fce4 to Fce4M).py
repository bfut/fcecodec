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
    bfut_ConvertPartnames (Fce4 to Fce4M).py - convert partnames from FCE4 to FCE4M

USAGE
    python "bfut_ConvertPartnames (Fce4 to Fce4M).py" /path/to/model.fce [/path/to/output.fce]

    When converting to FCE4M, priority search for dummies :LICENSE, etc. and create :PPlicense at that position

REQUIRES
    installing fcecodec <https://github.com/bfut/fcecodec>
"""
import argparse
import pathlib

import fcecodec as fc

from bfut_mywrappers import *  # fcecodec/scripts/bfut_mywrappers.py

CONFIG = {
    "fce_version" : "keep",  # output format version; expects "keep" or "3"|"4"|"4M" for FCE3, FCE4, FCE4M, respectively
    "script_version" : "4M", # "34"|"4M"
}

# Parse command-line
parser = argparse.ArgumentParser()
parser.add_argument("path", nargs="+", help="file path")
args = parser.parse_args()

# Handle paths: mandatory inpath, optional outpath
filepath_fce_input = pathlib.Path(args.path[0])
if len(args.path) < 2:
    filepath_fce_output = filepath_fce_input.parent / (filepath_fce_input.stem + "_out" + filepath_fce_input.suffix)
else:
    filepath_fce_output = pathlib.Path(args.path[1])

#
def Partnames3to4_car(mesh):
    # car.fce
    # NB1: front wheel order is different for high body/medium body
    pnames_map = [
        ":HB",  # high body
        ":HLFW",  # left front wheel
        ":HRFW",  # right front wheel
        ":HLRW",  # left rear wheel
        ":HRRW",  # right rear wheel
        ":MB",  # medium body
        ":MRFW",  # medium r front wheel
        ":MLFW",  # medium l front wheel
        ":MRRW",  # medium r rear wheel
        ":MLRW",  # medium l rear wheel
        ":LB",  # small body
        ":TB",  # tiny body
        ":OL",  # high headlights
    ]
    for pid in range(min(len(pnames_map), mesh.MNumParts)):
        mesh.PSetName(pid, pnames_map[pid])
        print(f"renaming part {pid} -> '{pnames_map[pid]}'")
    return mesh

def Partnames4to4M_car(mesh):
    # car.fce
    # FCE4M loads meshes for wheels, drivers, and enhanced parts from central files.
    pnames_map = {
        ":HB": ":Hbody",
        # ":MB": ,
        # ":LB": ,
        # ":TB": ,
        ":OT": ":Hconvertible",
        ":OL": ":Hheadlight",
        ":OS": ":PPspoiler",
        ":OLB": ":Hlbrake",
        ":ORB": ":Hrbrake",
        ":OLM": ":Hlmirror",
        ":ORM": ":Hrmirror",
        ":OC": ":Hinterior",
        ":ODL": ":Hdashlight",
        # ":OH": ,
        ":OD": ":PPdriver",
        # ":OND": ,
        ":HLFW": ":PPLFwheel",
        ":HRFW": ":PPRFwheel",
        # ":HLMW": ,
        # ":HRMW": ,
        ":HLRW": ":PPLRwheel",
        ":HRRW": ":PPRRwheel",
        # ":MLFW": ,
        # ":MRFW": ,
        # ":MLMW": ,
        # ":MRMW": ,
        # ":MLRW": ,
        # ":MRRW": ,
    }
    for pid in range(mesh.MNumParts):
        pname = mesh.PGetName(pid)
        new_pname = pnames_map.get(pname, None)
        if not new_pname is None:
            mesh.PSetName(pid, new_pname)
            print(f"renaming part {pid} '{pname}' -> '{new_pname}'")
    return mesh

def AddPartAtPositionForLicenseDummy(mesh):
    """
        If license dummy exists, add FCE4M ":PPlicense"-part  at dummy position
        Do nothing, if ":PPlicense" already exists.
    """
    if GetMeshPartnameIdx(mesh, ":PPlicense") < 0:
        # lic_list = [":LICENSE", ":LICENSE_EURO", ":LICMED", ":LICLOW"]
        lic_list = [":LICENS", ":LICMED", ":LICLOW"]
        dms_names = mesh.MGetDummyNames()
        dms_names = [name[:7] for name in dms_names]
        for lic in lic_list:
            if lic in dms_names:
                didx = dms_names.index(lic)
                dms_pos = mesh.MGetDummyPos()[3*didx:3*didx + 3]
                new_pid = mesh.OpAddHelperPart(":PPlicense", dms_pos)
                print(f"adding dummy part {new_pid} ':PPlicense'")
                break
    return mesh


#
def main():
    if CONFIG["fce_version"] == "keep":
        fce_outversion = str(GetFceVersion(filepath_fce_input))
        if fce_outversion == "5":
            fce_outversion = "4M"
    else:
        fce_outversion = CONFIG["fce_version"]
    if CONFIG["script_version"] not in ["34", "4M"]:
        raise ValueError('invalid script_version config (expects "34"|"4M")')

    # Load FCE
    mesh = fc.Mesh()
    mesh = LoadFce(mesh, filepath_fce_input)

    if mesh.MNumParts < 1:
        print("ConvertPartnames: FCE must have at least 1 part.")
        return

    # Convert partnames
    if CONFIG["script_version"] == "34":
        mesh = Partnames3to4_car(mesh)
    elif CONFIG["script_version"] == "4M":
        mesh = Partnames4to4M_car(mesh)
        mesh = AddPartAtPositionForLicenseDummy(mesh)

    # Write FCE
    WriteFce(fce_outversion, mesh, filepath_fce_output)
    PrintFceInfo(filepath_fce_output)
    print(f"OUTPUT = {filepath_fce_output}", flush=True)

if __name__ == "__main__":
    main()

