/*
  fcelib.h
  fcecodec Copyright (C) 2021 and later Benjamin Futasz <https://github.com/bfut>

  You may not redistribute this program without its source code.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/**
  public-only interface. include this header in your project to use fcelib.
  written in C89, compiles in: C/C++, msvc|gcc|clang, 32|64-bit, little endian, win|linux|macos

  fcelib_fcetypes.h defines FCE structs and includes extensive FCE format documentation
  fcelib_types.h defines struct FcelibMesh

  usage:
    #include "fcelib.h"
    ...
    FcelibMesh mesh;
    if (!FCELIB_MeshInit(&mesh))  return EXIT_FAILURE;
    // workload
    FCELIB_MeshRelease(&mesh);
**/

#ifndef FCELIB_H_
#define FCELIB_H_

#define FCECVERS "1.15"

#ifndef SCL_DEBUG
#define SCL_DEBUG 0  /* 0: default, 1: dev console output, additional checks */
#endif

#if defined(SCL_DEBUG) && SCL_DEBUG > 0 && defined(FCECVERS_GREATER_THAN_2_0)
#include <assert.h>
#define SCL_assert assert
#else
/* #define SCL_printf(format, ...) { } */  /* C89-incompatible */
static void SCL_printf(const char *format, ...) { (void)format; }
#define SCL_assert(x)
#endif

#include <stdio.h>
#include <string.h>

#if defined(SCL_DEBUG) && SCL_DEBUG > 0
#include "./fcelib_diagnostics.h"
#endif
#include "./fcelib_fcetypes.h"
#include "./fcelib_io.h"
#include "./fcelib_op.h"
#include "./fcelib_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* mesh ------------------------------------------------------------------------------------------------------------- */

void (*FCELIB_MeshRelease)(FcelibMesh *mesh) = FCELIB_TYPES_MeshRelease;
FcelibMesh *(*FCELIB_MeshInit)(FcelibMesh *mesh) = FCELIB_TYPES_MeshInit;
void (*FCELIB_PrintMeshInfo)(const FcelibMesh *mesh) = FCELIB_TYPES_PrintMeshInfo;

/* mesh: operations ------------------------------------------------------------------------------------------------- */

int (*FCELIB_AddHelperPart)(FcelibMesh *mesh) = FCELIB_OP_AddHelperPart;
int (*FCELIB_CenterPart)(FcelibMesh *mesh, int pid) = FCELIB_OP_CenterPart;
int (*FCELIB_SetPartCenter)(FcelibMesh *mesh, int pid, const float new_center[3]) = FCELIB_OP_SetPartCenter;
int (*FCELIB_CopyPartToMesh)(FcelibMesh *mesh, FcelibMesh *mesh_src, int pid_src) = FCELIB_OP_CopyPartToMesh;
void (*FCELIB_DeletePart)(FcelibMesh *mesh, int pid) = FCELIB_OP_DeletePart;
int (*FCELIB_DeletePartTriags)(FcelibMesh *mesh, const int pid, const int *idxs, int idxs_len) = FCELIB_OP_DeletePartTriags;
int (*FCELIB_DeleteUnrefdVerts)(FcelibMesh *mesh) = FCELIB_OP_DeleteUnrefdVerts;
int (*FCELIB_MergePartsToNew)(FcelibMesh *mesh, int pid1, int pid2) = FCELIB_OP_MergePartsToNew;
/* void (*FCELIB_MeshSwapParts)(FcelibMesh *mesh, const int pid1, const int pid2) = FCELIB_OP_SwapParts; */
int (*FCELIB_MeshMoveUpPart)(FcelibMesh *mesh, const int pid) = FCELIB_OP_MoveUpPart;

/* util  ------------------------------------------------------------------------------------------------------------ */

int (*FCELIB_FceComputeSize)(const FcelibMesh *mesh, const int target_fce_version) = FCELIB_TYPES_FceComputeSize;
int (*FCELIB_GetFceVersion)(const void * const buf, const int bufsz) = FCELIB_FCETYPES_GetFceVersion;
void (*FCELIB_PrintFceInfo)(const void *buf, int bufsz) = FCELIB_FCETYPES_PrintHeaderFce;

int FCELIB_ValidateFce(const char *buf, const int fce_size)  /* DEPRECATED as of 1.15 */
{
  return FCELIB_FCETYPES_GetFceVersion(buf, fce_size) > 0;
}

/* i/o -------------------------------------------------------------------------------------------------------------- */

int (*FCELIB_DecodeFce)(FcelibMesh *mesh, const void *inbuf, int inbufsz) = FCELIB_IO_DecodeFce;
int (*FCELIB_EncodeFce3)(const FcelibMesh *mesh, unsigned char **outbuf, int outbufsz, int center_parts) = FCELIB_IO_EncodeFce3;

int FCELIB_EncodeFce4(const FcelibMesh *mesh, unsigned char **outbuf, int outbufsz, int center_parts)
{
  return FCELIB_IO_EncodeFce4(mesh, outbuf, outbufsz, center_parts, 0x00101014);
}

int FCELIB_EncodeFce4M(const FcelibMesh *mesh, unsigned char **outbuf, int outbufsz, int center_parts)
{
  return FCELIB_IO_EncodeFce4(mesh, outbuf, outbufsz, center_parts, 0x00101015);
}

int (*FCELIB_ExportObj)(const FcelibMesh *mesh,
                        const char *objpath, const char *mtlpath,
                        const char *texture_name,
                        int print_damage, int print_dummies,
                        int use_part_positions,
                        int print_part_positions,
                        int filter_triagflags_0xfff) = FCELIB_IO_ExportObj;

/* DEPRECATED from 2.0 */
int (*FCELIB_GeomDataToNewPart)(FcelibMesh *mesh,
                                int *vert_idxs, int vert_idxs_len,
                                float *vert_texcoords, int vert_texcoords_len,
                                float *vert_pos, int vert_pos_len,
                                float *normals, int normals_len) = FCELIB_IO_GeomDataToNewPart;

/* diagnostics (debug) ---------------------------------------------------------------------------------------------- */

#if defined(SCL_DEBUG) && SCL_DEBUG > 0
void (*FCELIB_PrintMeshParts)(const FcelibMesh *mesh) = FCELIB_DIAGNOSTICS_PrintMeshParts;
void (*FCELIB_PrintMeshTriangles)(const FcelibMesh *mesh) = FCELIB_DIAGNOSTICS_PrintMeshTriangles;
void (*FCELIB_PrintMeshVertices)(const FcelibMesh *mesh) = FCELIB_DIAGNOSTICS_PrintMeshVertices;
#else
#define FCELIB_PrintMeshParts(x)
#define FCELIB_PrintMeshTriangles(x)
#define FCELIB_PrintMeshVertices(x)
#endif

/* compatibility ---------------------------------------------------------------------------------------------------- */

#if !defined(SCL_DEBUG) || SCL_DEBUG != 0
#define FCELIB_FreeMesh(x) FCELIB_MeshRelease(x)  /* b/c through 2.99 */
#define FCELIB_InitMesh(x) FCELIB_MeshInit(x)  /* b/c through 2.99 */
#define FCELIB_MeshValidate(x) (int)1  /* b/c through 2.99 */
#define FCELIB_ValidateMesh(x) FCELIB_MeshValidate(x)  /* b/c through 2.99 */
#endif

/* service ---------------------------------------------------------------------------------------------------------- */

int (*FCELIB_GetInternalPartIdxByOrder)(const FcelibMesh *mesh, const int order) = FCELIB_TYPES_GetInternalPartIdxByOrder;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FCELIB_H_ */
