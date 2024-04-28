/*
  fcelib.h
  fcecodec Copyright (C) 2021-2024 Benjamin Futasz <https://github.com/bfut>

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
  high level library interface: library users should only use functions from this header externally,
    whereas functions from here are never used internally

  fcelib_types.h defines library structs
  fcelib_fcetypes.h defines FCE structs and includes extensive FCE format documentation

  "TODO:" in comments marks nitpicks
**/

#ifndef FCELIB_H_
#define FCELIB_H_

#include <stdio.h>
#include <string.h>

#define FCECVERS "1.6"
#ifndef FCECVERBOSE
#define FCECVERBOSE 0  /* >=1 for verbose console output */
/* #define FCEC_STATE */  /* feature branch */
#endif

#include "./fcelib_io.h"
#include "./fcelib_op.h"
#include "./fcelib_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* mesh --------------------------------------------------------------------- */

void FCELIB_FreeMesh(FcelibMesh *mesh)
{
  if (mesh->release == &FCELIB_TYPES_FreeMesh)
    mesh->release(mesh);
}

FcelibMesh *FCELIB_InitMesh(FcelibMesh *mesh)
{
  return FCELIB_TYPES_InitMesh(mesh);
}

void FCELIB_PrintMeshInfo(const FcelibMesh *mesh)
{
  FCELIB_TYPES_PrintMeshInfo(mesh);
}

void FCELIB_PrintMeshParts(const FcelibMesh *mesh)
{
  FCELIB_TYPES_PrintMeshParts(mesh);
}

void FCELIB_PrintMeshTriangles(const FcelibMesh *mesh)
{
  FCELIB_TYPES_PrintMeshTriangles(mesh);
}

void FCELIB_PrintMeshVertices(const FcelibMesh *mesh)
{
  FCELIB_TYPES_PrintMeshVertices(mesh);
}

int FCELIB_ValidateMesh(const FcelibMesh *mesh)
{
  return FCELIB_TYPES_ValidateMesh(mesh);
}

/* mesh: operations --------------------------------------------------------- */

int FCELIB_AddHelperPart(FcelibMesh *mesh)
{
  return FCELIB_OP_AddHelperPart(mesh);
}

int FCELIB_CenterPart(FcelibMesh *mesh, const int idx)
{
  return FCELIB_OP_CenterPart(mesh, idx);
}

int FCELIB_SetPartCenter(FcelibMesh *mesh, const int pidx,
                         const float new_center[3])
{
  return FCELIB_OP_SetPartCenter(mesh, pidx, new_center);
}

int FCELIB_CopyPartToMesh(FcelibMesh *mesh_rcv, FcelibMesh *mesh_src,
                          const int idx)
{
  return FCELIB_OP_CopyPartToMesh(mesh_rcv, mesh_src, idx);
}

int FCELIB_DeletePart(FcelibMesh *mesh, const int idx)
{
  return FCELIB_OP_DeletePart(mesh, idx);
}

int FCELIB_DeletePartTriags(FcelibMesh *mesh, const int pidx,
                            const int *idxs, const int idxs_len)
{
  return FCELIB_OP_DeletePartTriags(mesh, pidx, idxs, idxs_len);
}

int FCELIB_DeleteUnrefdVerts(FcelibMesh *mesh)
{
  return FCELIB_OP_DeleteUnrefdVerts(mesh);
}

int FCELIB_MergePartsToNew(FcelibMesh *mesh, const int idx1, const int idx2)
{
  return FCELIB_OP_MergePartsToNew(mesh, idx1, idx2);
}

int FCELIB_MeshMoveUpPart(FcelibMesh *mesh, const int idx)
{
  return FCELIB_OP_MoveUpPart(mesh, idx);
}

/* tools -------------------------------------------------------------------- */

/* Returns size in bytes. target_fce_version: 3 (FCE3), 4 (FCE4), 5 (FCE4M) */
int FCELIB_FceComputeSize(const FcelibMesh *mesh, const int target_fce_version)
{
  switch (target_fce_version)
  {
    case 4:
      return FCELIB_FCETYPES_Fce4ComputeSize(0x00101014, mesh->hdr.NumVertices,
                                             mesh->hdr.NumTriangles);
      break;
    case 5:
      return FCELIB_FCETYPES_Fce4ComputeSize(0x00101015, mesh->hdr.NumVertices,
                                             mesh->hdr.NumTriangles);
      break;
    default:
      return FCELIB_FCETYPES_Fce3ComputeSize(mesh->hdr.NumVertices,
                                             mesh->hdr.NumTriangles);
      break;
  }
}

int FCELIB_GetFceVersion(const void *buf, const int length)
{
  return FCELIB_FCETYPES_GetFceVersion(buf, length);
}

void FCELIB_PrintFceInfo(const int fce_size, const void *hdr)
{
  switch (FCELIB_FCETYPES_GetFceVersion(hdr, fce_size))
  {
    case 4: case 5:
      FCELIB_FCETYPES_PrintHeaderFce4(hdr, fce_size);
      break;
    case -3: case -4: case -5:
      break;
    default:
      FCELIB_FCETYPES_PrintHeaderFce3(hdr, fce_size);
      break;
  }
}

/* i/o ---------------------------------------------------------------------- */

int FCELIB_ValidateFce(const void *buf, const int length)
{
  switch (FCELIB_FCETYPES_GetFceVersion(buf, length))
  {
    case 4: case 5:
    {
      const FceHeader4 hdr = FCELIB_FCETYPES_GetFceHeader4((const unsigned char *)buf);
      return FCELIB_FCETYPES_Fce4ValidateHeader(length, buf, &hdr);
    }
    case -3: case -4: case -5:
      return 0;
    default:
    {
      const FceHeader3 hdr = FCELIB_FCETYPES_GetFceHeader3((const unsigned char *)buf);
      return FCELIB_FCETYPES_Fce3ValidateHeader(length, buf, &hdr);
    }
  }
}

int FCELIB_DecodeFce(const void *buf, int buf_size, FcelibMesh *mesh)
{
  return FCELIB_IO_DecodeFce(mesh, (const unsigned char *)buf, buf_size);
}

int FCELIB_ExportObj(const FcelibMesh *mesh,
                     const char *objpath, const char *mtlpath,
                     const char *texture_name,
                     const int print_damage, const int print_dummies,
                     const int use_part_positions,
                     const int print_part_positions)
{
  return FCELIB_IO_ExportObj(mesh, objpath, mtlpath, texture_name,
                             print_damage, print_dummies,
                             use_part_positions, print_part_positions);
}

int FCELIB_EncodeFce3(unsigned char **buf, const int buf_size,
                      FcelibMesh *mesh, const int center_parts)
{
  return FCELIB_IO_EncodeFce3(mesh, buf, buf_size, center_parts);
}

int FCELIB_EncodeFce4(unsigned char **buf, const int buf_size,
                      FcelibMesh *mesh, const int center_parts)
{
  return FCELIB_IO_EncodeFce4(mesh, buf, buf_size, center_parts, 0x00101014);
}

int FCELIB_EncodeFce4M(unsigned char **buf, const int buf_size,
                       FcelibMesh *mesh, const int center_parts)
{
  return FCELIB_IO_EncodeFce4(mesh, buf, buf_size, center_parts, 0x00101015);
}

int FCELIB_GeomDataToNewPart(FcelibMesh *mesh,
                             int *vert_idxs, const int vert_idxs_len,
                             float *vert_texcoords, const int vert_texcoords_len,
                             float *vert_pos, const int vert_pos_len,
                             float *normals, const int normals_len)
{
  return FCELIB_IO_GeomDataToNewPart(mesh,
                                     vert_idxs, vert_idxs_len,
                                     vert_texcoords, vert_texcoords_len,
                                     vert_pos, vert_pos_len,
                                     normals, normals_len);
}

/* service ------------------------------------------------------------------ */

int FCELIB_GetInternalPartIdxByOrder(const FcelibMesh *mesh, const int idx)
{
  return FCELIB_TYPES_GetInternalPartIdxByOrder(mesh, idx);
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FCELIB_H_ */
