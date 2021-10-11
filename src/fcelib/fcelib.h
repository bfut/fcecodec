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

#define FCECVERS "0.11"

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


/* mesh -------------------------------------------------------------------- */

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


/* mesh: operations -------------------------------------------------------- */

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

int FCELIB_DeleteTriags(FcelibMesh *mesh, int *idxs, const int idxs_len);  // TODO: implement

#if 0  // replaced by FCELIB_DeleteTriags()
int FCELIB_DeleteUnmappedTriags(FcelibMesh *mesh, int keep_tex)
{
  return FCELIB_OP_DeleteUnmappedTriags(mesh, keep_tex);
}
#endif

int FCELIB_DeleteUnrefdVerts(FcelibMesh *mesh)
{
  return FCELIB_OP_RemoveUnrefdVerts(mesh);
}

int FCELIB_MergePartsToNew(FcelibMesh *mesh, int idx1, int idx2)
{
  return FCELIB_OP_MergePartsToNew(mesh, idx1, idx2);
}

int FCELIB_MeshMoveUpPart(FcelibMesh *mesh, const int idx)
{
  return FCELIB_OP_MoveUpPart(mesh, idx);
}

#if 0
int FCELIB_MeshRenamePart(FcelibMesh *mesh, const int idx, const char *name)
{
  return FCELIB_OP_RenamePart(mesh, idx, name);
}
#endif


/* tools ------------------------------------------------------------------- */

void FCELIB_PrintFceInfo(const int fce_size, void *hdr)
{
  int version;
  memcpy(&version, hdr, (size_t)4);
  switch (version)
  {
    case 0x00101014: case 0x00101015:
      FCELIB_FCETYPES_PrintHeaderFce4(fce_size, hdr);
      break;
    default:
      FCELIB_FCETYPES_PrintHeaderFce3(fce_size, hdr);
      break;
  }
}


/* io ---------------------------------------------------------------------- */

int FCELIB_GetFileBufRead(unsigned char **buf, int *length, const char *path)
{
  return FCELIB_IO_GetFileBufRead(buf, length, path);
}

int FCELIB_FreeFileBuf(void *buf, int length)
{
  return FCELIB_IO_FreeFileBuf(buf, length);
}

int FCELIB_ValidateFce(const void *buf, const int length)
{
  int version;
  memcpy(&version, buf, (size_t)4);
  switch (version)
  {
    case 0x00101014: case 0x00101015:
      if (length < 0x2038) return 0;
      return FCELIB_FCETYPES_Fce4ValidateHeader(buf, length);
    default:
      if (length < 0x1F04) return 0;
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

int FCELIB_EncodeFce3(FcelibMesh *mesh, const char *fcepath, const int center_parts)
{
  return FCELIB_IO_EncodeFce3(mesh, fcepath, center_parts);
}

int FCELIB_EncodeFce4(FcelibMesh *mesh, const char *fcepath, const int center_parts)
{
  return FCELIB_IO_EncodeFce4(mesh, fcepath);  // TODO: implement
}

int FCELIB_GeomDataToNewPart(FcelibMesh *mesh,
                             float **triangles, const int triangles_len,
                             float **triangle_flags, const int triangles_flags_len,
                             float **vertices, const int vertices_len,
                             float **normals, const int normals_len,
                             float **texcoords, const int texcoords_len);  // TODO:implement

#ifdef __cplusplus
} /* extern "C" */
#endif

#ifdef __cplusplus
} /* namespace fcelib */
#endif

#endif /* FCELIB_H */
