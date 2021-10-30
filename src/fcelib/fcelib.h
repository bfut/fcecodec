/*
  fcelib.h
  fcecodec Copyright (C) 2021 Benjamin Futasz <https://github.com/bfut>

  You may not redistribute this program without its source code.

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
  library interface

  FCE-specific documentation can be found in fcelib_fcetypes.h
 **/

#ifndef FCELIB_H
#define FCELIB_H

#include <stdio.h>
#include <string.h>

#define FCECVERS "0.52"

#include "fcelib_io.h"
#include "fcelib_op.h"
#include "fcelib_types.h"

#ifdef __cplusplus
namespace fcelib {
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* source: fshtool.c, D. Auroux */
int FCELIB_SanityTest(void)
{
  int retv = 1;
  int x = 0;

  *((char *)(&x)) = 1;
  if (x != 1)
  {
    fprintf(stderr, "expects little-endian architecture\n");
    retv = 0;
  }

  if (sizeof(int) != 4)
  {
    fprintf(stderr, "expects 32-bit int\n");
    retv = 0;
  }

  if (sizeof(short) != 2)
  {
    fprintf(stderr, "expects 16-bit short\n");
    retv = 0;
  }

  if (sizeof(char) != 1)
  {
    fprintf(stderr, "expects 8-bit char\n");
    retv = 0;
  }

  return retv;
}


/* mesh --------------------------------------------------------------------- */

void FCELIB_InitMesh(FcelibMesh *mesh)
{
  FCELIB_TYPES_InitMesh(mesh);
}

void FCELIB_FreeMesh(FcelibMesh *mesh)
{
  FCELIB_TYPES_FreeMesh(mesh);
}

void FCELIB_PrintMeshInfo(FcelibMesh mesh)
{
  FCELIB_TYPES_PrintMeshInfo(mesh);
}

void FCELIB_PrintMeshParts(FcelibMesh mesh)
{
  FCELIB_TYPES_PrintMeshParts(mesh);
}

void FCELIB_PrintMeshTriangles(FcelibMesh mesh)
{
  FCELIB_TYPES_PrintMeshTriangles(mesh);
}

void FCELIB_PrintMeshVertices(FcelibMesh mesh)
{
  FCELIB_TYPES_PrintMeshVertices(mesh);
}

int FCELIB_ValidateMesh(FcelibMesh mesh)
{
  return FCELIB_TYPES_ValidateMesh(mesh);
}


/* mesh: operations --------------------------------------------------------- */

int FCELIB_CenterPart(FcelibMesh *mesh, const int idx)
{
  return FCELIB_OP_CenterPart(mesh, idx);
}

int FCELIB_CopyPartToMesh(FcelibMesh *mesh_rcv,
                          FcelibMesh *mesh_src, const int idx)
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

void FCELIB_PrintFceInfo(const int fce_size, const void *hdr)
{
  int version;
  if (fce_size < 0x1F04) return;
  memcpy(&version, hdr, (size_t)4);
  switch (version)
  {
    case 0x00101014: case 0x00101015:
      if (fce_size < 0x2038) return;
      FCELIB_FCETYPES_PrintHeaderFce4(fce_size, hdr);
      break;
    default:
      FCELIB_FCETYPES_PrintHeaderFce3(fce_size, hdr);
      break;
  }
}


/* i/o ---------------------------------------------------------------------- */

int FCELIB_ValidateFce(const void *buf, const int length)
{
  int version;
  if (length < 0x1F04) return 0;
  memcpy(&version, buf, (size_t)4);
  switch (version)
  {
    case 0x00101014: case 0x00101015:
      if (length < 0x2038) return 0;
      return FCELIB_FCETYPES_Fce4ValidateHeader(buf, length);
    default:
      return FCELIB_FCETYPES_Fce3ValidateHeader(buf, length);
  }
}

int FCELIB_DecodeFce(const void *buf, int buf_size, FcelibMesh *mesh)
{
  return FCELIB_IO_DecodeFce(buf, buf_size, mesh);
}

int FCELIB_ExportObj(FcelibMesh *mesh,
                     const char *objpath, const char *mtlpath,
                     const char *texture_name,
                     const int print_damage, const int print_dummies)
{
  return FCELIB_IO_ExportObj(mesh, objpath, mtlpath, texture_name,
                             print_damage, print_dummies);
}

int FCELIB_EncodeFce3_Fopen(FcelibMesh *mesh, const char *fcepath,
                            const int center_parts)
{
  return FCELIB_IO_EncodeFce3_Fopen(mesh, fcepath, center_parts);
}

int FCELIB_EncodeFce3(unsigned char **buf, const int buf_size,
                      FcelibMesh *mesh, const int center_parts)
{
  return FCELIB_IO_EncodeFce3(buf, buf_size, mesh, center_parts);
}

int FCELIB_EncodeFce4(unsigned char **buf, const int buf_size,
                      FcelibMesh *mesh, const int center_parts)
{
  return FCELIB_IO_EncodeFce4(buf, buf_size, mesh, center_parts, 0x00101014);
}

int FCELIB_EncodeFce4M(unsigned char **buf, const int buf_size,
                      FcelibMesh *mesh, const int center_parts)
{
  return FCELIB_IO_EncodeFce4(buf, buf_size, mesh, center_parts, 0x00101015);
}

int FCELIB_GeomDataToNewPart(FcelibMesh *mesh,
                             int *vert_idxs, const int vert_idxs_len,
                            //  int *triangles_flags, const int triangles_flags_len,
                             float *vert_texcoords, const int vert_texcoords_len,
                             float *vert_pos, const int vert_pos_len,
                             float *normals, const int normals_len)
{
  return FCELIB_IO_GeomDataToNewPart(mesh,
                                     vert_idxs, vert_idxs_len,
                                    //  triangles_flags, triangles_flags_len,
                                     vert_texcoords, vert_texcoords_len,
                                     vert_pos, vert_pos_len,
                                     normals, normals_len);  // TODO: test
}

/* service (assumes valid FcelibMesh) --------------------------------------- */

int FCELIB_GetInternalPartIdxByOrder(FcelibMesh *mesh, const int idx)
{
  return FCELIB_TYPES_GetInternalPartIdxByOrder(mesh, idx);
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#ifdef __cplusplus
} /* namespace fcelib */
#endif

#endif /* FCELIB_H */