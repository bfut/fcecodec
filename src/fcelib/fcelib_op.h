/*
  fcelib_op.h
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
  implements mesh operations
 **/


#ifndef FCELIB_OP_H
#define FCELIB_OP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fcelib_fcetypes.h"
#include "fcelib_types.h"

#ifdef __cplusplus
namespace fcelib {
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* mesh --------------------------------------------------------------------- */

/* Center specified part around local centroid */
int FCELIB_OP_CenterPart(FcelibMesh *mesh, const int idx)
{
  int retv = 0;
  int internal_idx;
  FcelibPart *part;
  tVector centroid;

  for (;;)
  {
    internal_idx = FCELIB_TYPES_GetInternalPartIdxByOrder(mesh, idx);
    if (internal_idx < 0)
    {
      fprintf(stderr, "CenterPart: Invalid index (internal_idx)\n");
      break;
    }

    part = mesh->parts[ mesh->hdr.Parts[internal_idx] ];
    FCELIB_TYPES_GetPartLocalCentroid(mesh, part, &centroid);
    FCELIB_TYPES_ResetPartPos(mesh, part, centroid);

    retv = 1;
    break;
  }

  return retv;
}

/* Returns mesh_rcv new part index (order) on success, -1 on failure.
   Allows mesh_rcv == mesh_src. */
int FCELIB_OP_CopyPartToMesh(FcelibMesh *mesh_rcv,
                             FcelibMesh *mesh_src, const int idx_src)
{
  int retv = -1;
  int i;
  int j;
  int n;
  int vidx_1st;
  int tidx_1st;
  int *old_global_to_new_global_idxs = NULL;  /* maps vert ref's in copied triags */
  int internal_idx_rcv;
  int internal_idx_src;
  FcelibPart *part_rcv;
  FcelibPart *part_src;

  if (!FCELIB_TYPES_ValidateMesh(*mesh_rcv))
    return -1;
  if (!FCELIB_TYPES_ValidateMesh(*mesh_src))
    return -1;

  internal_idx_src = FCELIB_TYPES_GetInternalPartIdxByOrder(mesh_src, idx_src);
  if (internal_idx_src < 0)
  {
    fprintf(stderr, "CopyPartToMesh: Invalid index (internal_idx_src)\n");
    return -1;
  }

  for (;;)
  {
    // Lengthen part index map only if necessary
    if (!mesh_rcv->hdr.Parts)
    {
      if (!FCELIB_TYPES_AddParts(mesh_rcv, 1))
      {
        retv = -1;
        break;
      }
    }
    if (mesh_rcv->hdr.Parts[mesh_rcv->parts_len - 1] >= 0)
    {
      if (!FCELIB_TYPES_AddParts(mesh_rcv, 1))
      {
        retv = -1;
        break;
      }
    }

    /* Get first unused part index in receiving mesh */
    internal_idx_rcv = mesh_rcv->parts_len - 1;
    while ((mesh_rcv->hdr.Parts[internal_idx_rcv] < 0) && (internal_idx_rcv >= 0))
      --internal_idx_rcv;
    ++internal_idx_rcv;
    retv = internal_idx_rcv;

    tidx_1st = FCELIB_TYPES_GetFirstUnusedGlobalTriangleIdx(mesh_rcv);
    vidx_1st = FCELIB_TYPES_GetFirstUnusedGlobalVertexIdx(mesh_rcv);

    /* Add part */
//    mesh_rcv->hdr.Parts[internal_idx_rcv] = mesh_rcv->hdr.NumParts;
    mesh_rcv->hdr.Parts[internal_idx_rcv] = FCELIB_MISC_ArrMax(mesh_rcv->hdr.Parts, mesh_rcv->parts_len) + 1;
    part_rcv = (FcelibPart *)malloc(sizeof(*part_rcv));
    if (!part_rcv)
    {
      fprintf(stderr, "CopyPartToMesh: Cannot allocate memory (part_rcv)\n");
      retv = -1;
      break;
    }
    memset(part_rcv, 0, sizeof(*part_rcv));
    mesh_rcv->parts[ mesh_rcv->hdr.Parts[internal_idx_rcv] ] = part_rcv;

    ++mesh_rcv->hdr.NumParts;

    /* Get source part */
    part_src = mesh_src->parts[ mesh_src->hdr.Parts[internal_idx_src] ];
    if (!part_src)
    {
      fprintf(stderr, "CopyPartToMesh: Unexpected NULL (part_src)\n");
      retv = -1;
      break;
    }

    /* Copy */
    sprintf(part_rcv->PartName, "%s", part_src->PartName);
    memcpy(&part_rcv->PartPos.x, &part_src->PartPos.x, sizeof(float));
    memcpy(&part_rcv->PartPos.y, &part_src->PartPos.y, sizeof(float));
    memcpy(&part_rcv->PartPos.z, &part_src->PartPos.z, sizeof(float));

    part_rcv->PNumVertices = part_src->PNumVertices;
    part_rcv->PNumTriangles = part_src->PNumTriangles;

    if (!FCELIB_TYPES_AddVertices2(mesh_rcv, part_rcv, part_rcv->PNumVertices))
    {
      retv = -1;
      break;
    }

    if (!FCELIB_TYPES_AddTriangles2(mesh_rcv, part_rcv, part_rcv->PNumTriangles))
    {
      retv = -1;
      break;
    }

    /* Copy vertices */
    old_global_to_new_global_idxs = (int *)malloc((size_t)mesh_src->vertices_len * sizeof(*old_global_to_new_global_idxs));
    if (!old_global_to_new_global_idxs)
    {
      fprintf(stderr, "CopyPartToMesh: Cannot allocate memory (map)\n");
      retv = -1;
      break;
    }
    memset(old_global_to_new_global_idxs, -1, (size_t)mesh_src->vertices_len * sizeof(*old_global_to_new_global_idxs));

    // i - vert index in source, j - vert index in receiver
    for (i = 0, j = 0; (i < part_src->pvertices_len) && (j < part_src->PNumVertices); ++i)
    {
      if (part_src->PVertices[i] < 0)
        continue;

      mesh_rcv->vertices[vidx_1st + j] = (FcelibVertex *)malloc(sizeof(FcelibVertex));
      if (!mesh_rcv->vertices[vidx_1st + j])
      {
        fprintf(stderr, "CopyPartToMesh: Cannot allocate memory (vert)\n");
        retv = -1;
        break;
      }
      part_rcv->PVertices[j] = vidx_1st + j;
      FCELIB_TYPES_CpyVert(mesh_rcv->vertices[vidx_1st + j], mesh_src->vertices[ part_src->PVertices[i] ]);
      old_global_to_new_global_idxs[part_src->PVertices[i]] = vidx_1st + j;

      ++j;
    }
    mesh_rcv->hdr.NumVertices += part_rcv->PNumVertices;
    if (retv < 0)
      break;

    /* Copy triangles */
    // i - triag index in source, j - triag index in receiver
    for (i = 0, j = 0; (i < part_src->ptriangles_len) && (j < part_src->PNumTriangles); ++i)
    {
      if (part_src->PTriangles[i] < 0)
        continue;

      mesh_rcv->triangles[tidx_1st + j] = (FcelibTriangle *)malloc(sizeof(FcelibTriangle));
      if (!mesh_rcv->triangles[tidx_1st + j])
      {
        fprintf(stderr, "CopyPartToMesh: Cannot allocate memory (triag)\n");
        retv = -1;
        break;
      }
      part_rcv->PTriangles[j] = tidx_1st + j;
      FCELIB_TYPES_CpyTriag(mesh_rcv->triangles[tidx_1st + j], mesh_src->triangles[ part_src->PTriangles[i] ]);

      for (n = 0; n < 3; ++n)
        mesh_rcv->triangles[tidx_1st + j]->vidx[n] = old_global_to_new_global_idxs[mesh_rcv->triangles[tidx_1st + j]->vidx[n]];

      ++j;
    }
    if (retv < 0)
      break;
    mesh_rcv->hdr.NumTriangles += part_rcv->PNumTriangles;

    retv = FCELIB_TYPES_GetOrderByInternalPartIdx(mesh_rcv, mesh_rcv->hdr.Parts[internal_idx_rcv]);
    if (retv < 0)
    {
      fprintf(stderr, "CopyPartToMesh: Cannot get new part idx\n");
      break;
    }

    break;
  }  /* for (;;) */

  if (old_global_to_new_global_idxs)
    free(old_global_to_new_global_idxs);

  return retv;
}

int FCELIB_OP_DeletePart(FcelibMesh *mesh, const int idx)
{
  int i;
  int j;
  FcelibPart *part;

  if (!FCELIB_TYPES_ValidateMesh(*mesh))
    return 0;
  j = FCELIB_TYPES_GetInternalPartIdxByOrder(mesh, idx);
  if (j < 0)
    return 0;
  part = mesh->parts[ mesh->hdr.Parts[j] ];

  for (i = 0; i < part->PNumVertices; ++i)
  {
    free(mesh->vertices[ part->PVertices[i] ]);
    mesh->vertices[ part->PVertices[i] ] = NULL;
  }
  free(part->PVertices);

  for (i = 0; i < part->PNumTriangles; ++i)
  {
    free(mesh->triangles[ part->PTriangles[i] ]);
    mesh->triangles[ part->PTriangles[i] ] = NULL;
  }
  free(part->PTriangles);

  mesh->hdr.NumVertices -= part->PNumVertices;
  mesh->hdr.NumTriangles -= part->PNumTriangles;
  --mesh->hdr.NumParts;
  free(part);
  mesh->parts[ mesh->hdr.Parts[j] ] = NULL;
  mesh->hdr.Parts[j] = -1;

  return 1;
}

/* Delete part triangles by order. */
int FCELIB_OP_DeletePartTriags(FcelibMesh *mesh, const int pidx,
                               const int *idxs, const int idxs_len)
{
  int retv = 0;
  int i;
  int internal_idx;
  FcelibPart *part;
  int *ptr;
  int *sptr;
  int search_len = idxs_len;
  int *map = NULL;

  for (;;)
  {
    if (idxs_len < 1)
    {
      retv = 1;
      break;
    }
    if (!idxs)
    {
      fprintf(stderr, "DeletePartTriags: Unexpected NULL (idxs)\n");
      break;
    }

    if (!FCELIB_TYPES_ValidateMesh(*mesh))
    {
      fprintf(stderr, "DeletePartTriags: invalid mesh\n");
      break;
    }

    internal_idx = FCELIB_TYPES_GetInternalPartIdxByOrder(mesh, pidx);
    if (internal_idx < 0)
    {
      fprintf(stderr, "DeletePartTriags: Invalid index (internal_idx)\n");
      break;
    }
    part = mesh->parts[ mesh->hdr.Parts[internal_idx] ];

    map = (int *)malloc((size_t)idxs_len * sizeof(*map));
    if (!map)
    {
      fprintf(stderr, "DeletePartTriags: Cannot allocate memory (map)\n");
      break;
    }

    memcpy(map, idxs, (size_t)idxs_len * sizeof(*map));
    qsort(map, (size_t)idxs_len, sizeof(*map), FCELIB_MISC_CompareInts);
    if (map[0] < 0 || map[idxs_len - 1] > part->PNumTriangles)
    {
      fprintf(stderr, "DeletePartTriags: Triangle index out of range (idxs)\n");
      break;
    }

    ptr = NULL;
    sptr = map;
    for (i = 0; i < part->ptriangles_len && search_len > 0; ++i)
    {
      if (part->PTriangles[i] < 0)
        continue;
      ptr = (int *)bsearch(&i, sptr, (size_t)search_len, sizeof(*map), FCELIB_MISC_CompareInts);
      if (!ptr)
        continue;
      free(mesh->triangles[ part->PTriangles[i] ]);
      mesh->triangles[ part->PTriangles[i] ] = NULL;
      part->PTriangles[i] = -1;

      ++sptr;
      --search_len;
      ptr = NULL;
    }
    part->PNumTriangles -= idxs_len;
    mesh->hdr.NumTriangles -= idxs_len;
    free(map);

    retv = 1;
    break;
  }  /* for (;;) */

  return retv;
}

/* Deletes vertices that are not referenced by any triangles. */
int FCELIB_OP_DeleteUnrefdVerts(FcelibMesh *mesh)
{
  int i;
  int j;
  int k;
  FcelibPart *part;
  int *map;

  if (!FCELIB_TYPES_ValidateMesh(*mesh))
    return 0;

  map = (int *)malloc((size_t)mesh->vertices_len * sizeof(*map));
  if(!map)
  {
    fprintf(stderr, "DeleteUnrefdVerts: Cannot allocate memory (map)\n");
    return 0;
  }
  memset(map, 0, (size_t)mesh->vertices_len * sizeof(*map));

  for (i = 0; i < mesh->parts_len; ++i)
  {
    if (mesh->hdr.Parts[i] < 0)
      continue;
    part = mesh->parts[ mesh->hdr.Parts[i] ];

    /* Mark referenced verts */
    for (j = 0; j < part->ptriangles_len; ++j)
    {
      if (part->PTriangles[j] < 0)
        continue;

      for (k = 0; k < 3; ++k)
        map[ mesh->triangles[ part->PTriangles[j] ]->vidx[k] ] = 1;
    }

    /* Delete existing, unreferenced verts */
    for (j = 0; j < part->pvertices_len; ++j)
    {
      if (part->PVertices[j] < 0 || map[ part->PVertices[j] ] == 1)
        continue;
      free(mesh->vertices[ part->PVertices[j] ]);
      mesh->vertices[ part->PVertices[j] ] = NULL;
      part->PVertices[j] = -1;
      --part->PNumVertices;
      --mesh->hdr.NumVertices;
    }
  }

  free(map);
  return 1;
}

/* Returns new part index (order) on success, -1 on failure. */
int FCELIB_OP_MergePartsToNew(FcelibMesh *mesh, const int pid1, const int pid2)
{
  int new_pid = -1;
  int idx1;
  int idx2;
  int i;
  int j;
  int n;
  int vidx_1st;
  int tidx_1st;
  int *old_global_to_new_global_idxs = NULL;  /* maps vert ref's in copied triags */
  FcelibPart *part_dest;
  FcelibPart *part_src1;
  FcelibPart *part_src2;

#if FCECVERBOSE == 1
  fprintf(stdout, "MergePartsToNew:\n");
#endif

  if (pid1 == pid2)
  {
    fprintf(stderr, "MergePartsToNew: Cannot merge part with itself\n");
    return -1;
  }

  if (!FCELIB_TYPES_ValidateMesh(*mesh))
    return -1;

#if FCECVERBOSE == 1
  fprintf(stdout, "validation done...\n");
#endif

  idx1 = FCELIB_TYPES_GetInternalPartIdxByOrder(mesh, pid1);
  idx2 = FCELIB_TYPES_GetInternalPartIdxByOrder(mesh, pid2);
  if (idx1 < 0)
  {
    fprintf(stderr, "MergePartsToNew: Invalid index (idx1)\n");
    return -1;
  }
  if (idx2 < 0)
  {
    fprintf(stderr, "MergePartsToNew: Invalid index (idx2)\n");
    return -1;
  }
  part_src1 = mesh->parts[mesh->hdr.Parts[idx1]];
  part_src2 = mesh->parts[mesh->hdr.Parts[idx2]];
#if FCECVERBOSE == 1
    fprintf(stdout, "internal part indexes... (%d, %d)\n", idx1, idx2);
    fprintf(stdout, "%s + %s\n", part_src1->PartName, part_src2->PartName);
#endif

  for (;;)
  {
    if (mesh->hdr.Parts[mesh->parts_len - 1] >= 0)
    {
      if (!FCELIB_TYPES_AddParts(mesh, 1))
      {
        new_pid = -1;
        break;
      }
    }

    /* Get first unused index */
    i = mesh->parts_len - 1;
    while (mesh->hdr.Parts[i] < 0 && i >= 0)
      --i;
    ++i;
    new_pid = i;

    tidx_1st = FCELIB_TYPES_GetFirstUnusedGlobalTriangleIdx(mesh);
    vidx_1st = FCELIB_TYPES_GetFirstUnusedGlobalVertexIdx(mesh);
#if FCECVERBOSE == 1
    fprintf(stdout, "first unused indexes... (t %d, v %d)\n", tidx_1st, vidx_1st);
#endif

    /* Add part */
    mesh->hdr.Parts[new_pid] = FCELIB_MISC_ArrMax(mesh->hdr.Parts, mesh->parts_len) + 1;
    part_dest = (FcelibPart *)malloc(sizeof(*part_dest));
    if (!part_dest)
    {
      fprintf(stderr, "MergePartsToNew: Cannot allocate memory (part)\n");
      new_pid = -1;
      break;
    }
    memset(part_dest, 0, sizeof(*part_dest));
    part_dest->pvertices_len = 0;
    part_dest->ptriangles_len = 0;

    mesh->parts[ mesh->hdr.Parts[new_pid] ] = part_dest;
#if FCECVERBOSE == 1
    fprintf(stdout, "created new part... (order %d, internal %d)\n", new_pid, mesh->hdr.Parts[new_pid]);
#endif

    sprintf(part_dest->PartName, "%d_%d", idx1, idx2);
    part_dest->PartPos.x = 0.0f;
    part_dest->PartPos.y = 0.0f;
    part_dest->PartPos.z = 0.0f;

    part_dest->PNumVertices = part_src1->PNumVertices + part_src2->PNumVertices;
    part_dest->PNumTriangles = part_src1->PNumTriangles + part_src2->PNumTriangles;

    ++mesh->hdr.NumParts;


    /* Copy vertices */
#if FCECVERBOSE == 1
    fprintf(stdout, "add vertices? (excess: %d)\n", mesh->vertices_len - (vidx_1st + part_dest->PNumVertices));
#endif
    if (mesh->vertices_len < vidx_1st + part_dest->PNumVertices)
    {
      if (!FCELIB_TYPES_AddVertices2(mesh, part_dest, part_dest->PNumVertices))
      {
        new_pid = -1;
        break;
      }
    }

    old_global_to_new_global_idxs = (int *)malloc((size_t)mesh->vertices_len * sizeof(int));
    if (!old_global_to_new_global_idxs)
    {
      fprintf(stderr, "MergePartsToNew: Cannot allocate memory (map)\n");
      new_pid = -1;
      break;
    }
    memset(old_global_to_new_global_idxs, -1, (size_t)mesh->vertices_len * sizeof(int));

    // i - vert index in source, j - vert index in receiver
    for (i = 0, j = 0; i < part_src1->pvertices_len && j < part_src1->PNumVertices; ++i)
    {
      if (part_src1->PVertices[i] < 0)
        continue;

      mesh->vertices[vidx_1st + j] = (FcelibVertex *)malloc(sizeof(FcelibVertex));
      if (!mesh->vertices[vidx_1st + j])
      {
        fprintf(stderr, "MergePartsToNew: Cannot allocate memory (vert1)\n");
        new_pid = -1;
        break;
      }
      part_dest->PVertices[j] = vidx_1st + j;
      FCELIB_TYPES_CpyVert(mesh->vertices[vidx_1st + j], mesh->vertices[ part_src1->PVertices[i] ]);
      FCELIB_TYPES_VertAddPosition(mesh->vertices[vidx_1st + j], &part_src1->PartPos);
      old_global_to_new_global_idxs[part_src1->PVertices[i]] = vidx_1st + j;

      ++j;
    }
    if (new_pid < 0)
      break;
    for (i = 0, j = part_src1->PNumVertices; i < part_src2->pvertices_len && j - part_src1->PNumVertices < part_src2->PNumVertices; ++i)
    {
      if (part_src2->PVertices[i] < 0)
        continue;

      mesh->vertices[vidx_1st + j] = (FcelibVertex *)malloc(sizeof(FcelibVertex));
      if (!mesh->vertices[vidx_1st + j])
      {
        fprintf(stderr, "MergePartsToNew: Cannot allocate memory (vert2)\n");
        new_pid = -1;
        break;
      }
      part_dest->PVertices[j] = vidx_1st + j;
      FCELIB_TYPES_CpyVert(mesh->vertices[vidx_1st + j], mesh->vertices[ part_src2->PVertices[i] ]);
      FCELIB_TYPES_VertAddPosition(mesh->vertices[vidx_1st + j], &part_src2->PartPos);
      old_global_to_new_global_idxs[ part_src2->PVertices[i] ] = vidx_1st + j;

      ++j;
    }
    mesh->hdr.NumVertices += part_dest->PNumVertices;
    if (new_pid < 0)
      break;

    /* Copy triangles */
#if FCECVERBOSE == 1
    fprintf(stdout, "add triangles? (excess: %d)\n", mesh->triangles_len - (tidx_1st + part_dest->PNumTriangles));
#endif
    if (mesh->triangles_len < tidx_1st + part_dest->PNumTriangles)
    {
      if (!FCELIB_TYPES_AddTriangles2(mesh, part_dest, part_dest->PNumTriangles))
      {
        new_pid = -1;
        break;
      }
    }

    for (i = 0, j = 0; i < part_src1->ptriangles_len && j < part_src1->PNumTriangles; ++i)
    {
      if (part_src1->PTriangles[i] < 0)
        continue;
      mesh->triangles[tidx_1st + j] = (FcelibTriangle *)malloc(sizeof(FcelibTriangle));
      if (!mesh->triangles[tidx_1st + j])
      {
        fprintf(stderr, "MergePartsToNew: Cannot allocate memory (triag1)\n");
        new_pid = -1;
        break;
      }
      part_dest->PTriangles[j] = tidx_1st + j;
      FCELIB_TYPES_CpyTriag(mesh->triangles[tidx_1st + j], mesh->triangles[ part_src1->PTriangles[i] ]);

      for (n = 0; n < 3; ++n)
        mesh->triangles[tidx_1st + j]->vidx[n] = old_global_to_new_global_idxs[mesh->triangles[tidx_1st + j]->vidx[n]];

      ++j;
    }
    if (new_pid < 0)
      break;
    for (i = 0, j = part_src1->PNumTriangles; i < part_src2->ptriangles_len && j - part_src1->PNumTriangles < part_src2->PNumTriangles; ++i)
    {
      if (part_src2->PTriangles[i] < 0)
        continue;

      mesh->triangles[tidx_1st + j] = (FcelibTriangle *)malloc(sizeof(FcelibTriangle));
      if (!mesh->triangles[tidx_1st + j])
      {
        fprintf(stderr, "MergePartsToNew: Cannot allocate memory (triag2)\n");
        new_pid = -1;
        break;
      }
      part_dest->PTriangles[j] = tidx_1st + j;
      FCELIB_TYPES_CpyTriag(mesh->triangles[tidx_1st + j], mesh->triangles[ part_src2->PTriangles[i] ]);
      for (n = 0; n < 3; ++n)
        mesh->triangles[tidx_1st + j]->vidx[n] = old_global_to_new_global_idxs[mesh->triangles[tidx_1st + j]->vidx[n]];

      ++j;
    }
    mesh->hdr.NumTriangles += part_dest->PNumTriangles;

    new_pid = FCELIB_TYPES_GetOrderByInternalPartIdx(mesh, new_pid);
    if (new_pid < 0)
    {
      fprintf(stderr, "MergePartsToNew: Cannot get new part idx\n");
      break;
    }

    break;
  }  /* for (;;) */

  if (old_global_to_new_global_idxs)
    free(old_global_to_new_global_idxs);

#if FCECVERBOSE == 1
    fprintf(stdout, "return (new part order = %d)\n", new_pid);
#endif
  return new_pid;
}

/* Move up part in order ('up' means towards idx=0)
     If it's the first part, do nothing.
     If not, switch with previous part.
   Returns new index on success, -1 on failure.  */
int FCELIB_OP_MoveUpPart(FcelibMesh *mesh, const int idx)
{
  int internal_index_idx;
  int internal_index_j;

  if (!FCELIB_TYPES_ValidateMesh(*mesh))
    return -1;

  internal_index_idx = FCELIB_TYPES_GetInternalPartIdxByOrder(mesh, idx);
  internal_index_j = FCELIB_TYPES_GetInternalPartIdxByOrder(mesh, idx - 1);

  if (internal_index_idx < 0)
  {
    fprintf(stderr, "MoveUpPart: part %d does not exist\n", idx);
    return -1;
  }

  /* Do nothing, if 'idx' is the first part */
  if (internal_index_j < 0)
    return idx;

  /* Switch indexes */
  {
    const int tmp = mesh->hdr.Parts[internal_index_idx];
    mesh->hdr.Parts[internal_index_idx] = mesh->hdr.Parts[internal_index_j];
    mesh->hdr.Parts[internal_index_j] = tmp;
  }

  return idx - 1;
}

#ifdef __cplusplus
}  /* extern "C" */
#endif

#ifdef __cplusplus
}  /* namespace fcelib */
#endif

#endif  /* FCELIB_OP_H */