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
    bfut_MimicPartnamesAndPartsorder.py - mimic partnames and partorder of given source.fce

USAGE
    python "bfut_MimicPartnamesAndPartsorder.py" /path/to/source.fce /path/to/target.fce

    target.fce will be overwritten.
    This script works best for FCE files with either canonical FCE4 or FCE4M partnames.
    This script expects that both given FCE files each contain at least one part.

DESCRIPTION
    The following steps are executed
    1) every partname in target.fce that is not present in source.fce will be deleted in target.fce
    2) for every partname in source.fce that is not present in target.fce, a dummy part with that name will be added to target.fce
    3) sort parts in target.fce to the same order found in source.fce

    Note: If none of the partnames in source.fce are present in target.fce, all original parts will be deleted and replaced by dummies.

REQUIRES
    installing <https://github.com/bfut/fcecodec>
"""
import argparse
import pathlib

import fcecodec as fc

from bfut_mywrappers import *  # fcecodec/scripts/bfut_mywrappers.py

CONFIG = {
    "fce_version"  : "keep",  # output format version; expects "keep" or "3"|"4"|"4M" for FCE3, FCE4, FCE4M, respectively
}


# -------------------------------------- script functions
def PrintMeshParts_order(mesh, part_names_sorted):
    print("pid  IS                          SHOULD")
    for pid in range(mesh.MNumParts):
        print(f"{pid:<2} {mesh.PGetName(pid):<12} {part_names_sorted[pid]:<12}")

def AssertPartsOrder(mesh, part_names_sorted):
    for pid in range(mesh.MNumParts):
        if mesh.PGetName(pid) != part_names_sorted[pid]:
            PrintMeshParts_order(mesh, part_names_sorted)
            raise AssertionError (f"pid={pid} {mesh.PGetName(pid)} != {part_names_sorted[pid]}")


#
def main():
    # Parse command-line
    parser = argparse.ArgumentParser()
    parser.add_argument("path", nargs=2, help="file path")
    args = parser.parse_args()

    # Handle paths: mandatory source_path, mandatory target_path
    filepath_fce_input_source = pathlib.Path(args.path[0])
    filepath_fce_input = pathlib.Path(args.path[1])
    filepath_fce_output = filepath_fce_input

    # Process config / parameters
    if CONFIG["fce_version"] == "keep":
        fce_outversion = str(GetFceVersion(filepath_fce_input))
        if fce_outversion == "5":
            fce_outversion = "4M"
    else:
        fce_outversion = CONFIG["fce_version"]

    if GetFceVersion(filepath_fce_input_source) in ["3", 3]:
        print("Warning: source is in FCE3 format where partnames are irrelevant. result may be undesirable")

    # Load FCE
    mesh = fc.Mesh()
    mesh = LoadFce(mesh, filepath_fce_input)

    mesh_source = fc.Mesh()
    mesh_source = LoadFce(mesh_source, filepath_fce_input_source)

    if mesh.MNumParts < 1 or mesh_source.MNumParts < 1:
        print(f"source and target must have at least 1 part (src:{mesh.MNumParts}, trg:{mesh_source.MNumParts})")
        return

    # Mimic partnames and partorder
    partnames = GetMeshPartnames(mesh)
    partnames_source = GetMeshPartnames(mesh_source)
    # print(f"partnames=", partnames)
    # print(f"partnames_source=", partnames_source)

    print("Delete unnecessary parts (if any):")
    for idx in reversed(range(len(partnames))):
        pname = partnames[idx]
        if not pname in partnames_source:
            print(f"deleting part {idx} '{pname}'")
            mesh.OpDeletePart(idx)

    print("Add dummy parts for missing (if any):")
    for idx in reversed(range(len(partnames_source))):
        pname = partnames_source[idx]
        if not pname in partnames:
            new_pid = mesh.OpAddHelperPart(pname)
            print(f"adding dummy part {new_pid} '{pname}'")

    del partnames  # list outdated and no longer needed

    # Sort
    for target_pid in range(len(partnames_source)):
        pname = partnames_source[target_pid]
        current_pid = GetMeshPartnameIdx(mesh, pname)
        while current_pid > target_pid:
            current_pid = mesh.OpMovePart(current_pid)
    AssertPartsOrder(mesh, partnames_source)

    # Output FCE
    WriteFce(fce_outversion, mesh, filepath_fce_output, False)
    PrintFceInfo(filepath_fce_output)
    print(f"OUTPUT = {filepath_fce_output}", flush=True)


if __name__ == "__main__":
    main()
