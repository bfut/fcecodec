/*
  fcelib_op.h
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
  implements mesh operations
**/

#ifndef FCELIB_OP_H_
#define FCELIB_OP_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./fcelib_fcetypes.h"
#include "./fcelib_io.h"  /* FCELIB_IO_GeomDataToNewPart */
#include "./fcelib_types.h"
#include "./fcelib_util.h"  /* kTrianglesDiamond, kVertDiamond */

/* mesh --------------------------------------------------------------------- */

/*
  Auxiliary operation.
  Returns new part index (order) on success, -1 on failure.
*/
int FCELIB_OP_AddHelperPart(FcelibMesh *mesh)
{
  int i;
  int vert_idxs[8 * 3];
  float vert_texcoords[2 * 8 * 3];
  float vert_pos[6 * 3];

  memcpy(vert_idxs, kTrianglesDiamond, sizeof(kTrianglesDiamond));
  for (i = 0; i < 8 * 3; ++i)
    --vert_idxs[i];
  memset(vert_texcoords, 0, sizeof(vert_texcoords));
  memcpy(vert_pos, kVertDiamond, sizeof(kVertDiamond));
  for (i = 0; i < 6 * 3; ++i)
    vert_pos[i] *= 0.1f;

  return FCELIB_IO_GeomDataToNewPart(mesh,
                                     vert_idxs, 8 * 3,
                                     vert_texcoords, 2 * 8 * 3,
                                     vert_pos, 6 * 3,
                                     vert_pos, 6 * 3);
}

/*
  Center specified part around local centroid.
  Does not move part w.r.t. to global coordinates
*/
int FCELIB_OP_CenterPart(FcelibMesh *mesh, int pid)
{
  int retv = 0;
  int internal_pid;
  FcelibPart *part;
  tVector centroid;

  for (;;)
  {
    internal_pid = FCELIB_TYPES_GetInternalPartIdxByOrder(mesh, pid);
    if (internal_pid < 0)
    {
      fprintf(stderr, "CenterPart: Invalid index (internal_pid)\n");
      break;
    }

    part = mesh->parts[ mesh->hdr.Parts[internal_pid] ];
    FCELIB_TYPES_GetPartCentroid(mesh, part, &centroid);
    FCELIB_TYPES_ResetPartCenter(mesh, part, centroid);

    retv = 1;
    break;
  }

  return retv;
}

/*
  Center specified part around new_center.
  Does not move part w.r.t. to global coordinates
*/
int FCELIB_OP_SetPartCenter(FcelibMesh *mesh, int pid, const float new_center[3])
{
  int retv = 0;
  int internal_pid;
  FcelibPart *part;
  tVector temp;

  for (;;)
  {
    internal_pid = FCELIB_TYPES_GetInternalPartIdxByOrder(mesh, pid);
    if (internal_pid < 0)
    {
      fprintf(stderr, "SetPartCenter: Invalid index (internal_idx)\n");
      break;
    }

    part = mesh->parts[ mesh->hdr.Parts[internal_pid] ];
    memcpy(&temp.x, &new_center[0], sizeof(temp.x));
    memcpy(&temp.y, &new_center[1], sizeof(temp.y));
    memcpy(&temp.z, &new_center[2], sizeof(temp.z));
    FCELIB_TYPES_ResetPartCenter(mesh, part, temp);

    retv = 1;
    break;
  }

  return retv;
}

/*
  Returns mesh new part index (order) on success, -1 on failure.
  Allows (mesh == mesh_src)
*/
int FCELIB_OP_CopyPartToMesh(FcelibMesh *mesh, FcelibMesh *mesh_src, int pid_src)
{
  int pid_new = -1;
  int internal_pid_new;
  int internal_pid_src;
  int i;
  int j;
  int n;
  int vidx_1st;
  int tidx_1st;
  int *old_global_to_new_global_idxs = NULL;  /* maps vert ref's in copied triags */
  FcelibPart *part_new;
  FcelibPart *part_src;

  internal_pid_src = FCELIB_TYPES_GetInternalPartIdxByOrder(mesh_src, pid_src);
  if (internal_pid_src < 0)
  {
    fprintf(stderr, "CopyPartToMesh: Invalid index (internal_pid_src)\n");
    return -1;
  }

  for (;;)
  {
    /* Lengthen part index map only if necessary */
    if (!mesh->hdr.Parts)
    {
      if (!FCELIB_TYPES_AddParts(mesh, 1))
      {
        internal_pid_new = -1;
        break;
      }
    }
    if (mesh->hdr.Parts[mesh->parts_len - 1] >= 0)
    {
      if (!FCELIB_TYPES_AddParts(mesh, 1))
      {
        internal_pid_new = -1;
        break;
      }
    }

    /* Get first unused part index in receiving mesh */
    internal_pid_new = FCELIB_TYPES_GetFirstUnusedGlobalPartIdx(mesh);
    tidx_1st = FCELIB_TYPES_GetFirstUnusedGlobalTriangleIdx(mesh);
    vidx_1st = FCELIB_TYPES_GetFirstUnusedGlobalVertexIdx(mesh);

    /* Add part */
    mesh->hdr.Parts[internal_pid_new] = FCELIB_UTIL_ArrMax(mesh->hdr.Parts, mesh->parts_len) + 1;
    part_new = (FcelibPart *)malloc(sizeof(*part_new));
    if (!part_new)
    {
      fprintf(stderr, "CopyPartToMesh: Cannot allocate memory (part_new)\n");
      internal_pid_new = -1;
      break;
    }
    memset(part_new, 0, sizeof(*part_new));
    part_new->pvertices_len = 0;
    part_new->ptriangles_len = 0;
    mesh->parts[ mesh->hdr.Parts[internal_pid_new] ] = part_new;

    ++mesh->hdr.NumParts;

    /* Get source part */
    part_src = mesh_src->parts[ mesh_src->hdr.Parts[internal_pid_src] ];
    if (!part_src)
    {
      fprintf(stderr, "CopyPartToMesh: Unexpected NULL (part_src)\n");
      internal_pid_new = -1;
      break;
    }

    /* Copy */
    sprintf(part_new->PartName, "%s", part_src->PartName);
    memcpy(&part_new->PartPos.x, &part_src->PartPos.x, sizeof(float));
    memcpy(&part_new->PartPos.y, &part_src->PartPos.y, sizeof(float));
    memcpy(&part_new->PartPos.z, &part_src->PartPos.z, sizeof(float));

    part_new->PNumVertices = part_src->PNumVertices;
    part_new->PNumTriangles = part_src->PNumTriangles;

    /* Copy vertices */
    if (!FCELIB_TYPES_AddVerticesToPart(part_new, part_new->PNumVertices))
    {
      internal_pid_new = -1;
      break;
    }
    if (mesh->vertices_len < vidx_1st + part_new->PNumVertices)
    {
      if (!FCELIB_TYPES_AddVerticesToMesh(mesh, vidx_1st + part_new->PNumVertices - mesh->vertices_len))
      {
        internal_pid_new = -1;
        break;
      }
    }
    mesh->hdr.NumVertices += part_new->PNumVertices;

    old_global_to_new_global_idxs = (int *)malloc(mesh_src->vertices_len * sizeof(*old_global_to_new_global_idxs));
    if (!old_global_to_new_global_idxs)
    {
      fprintf(stderr, "CopyPartToMesh: Cannot allocate memory (map)\n");
      internal_pid_new = -1;
      break;
    }
    memset(old_global_to_new_global_idxs, 0xFF, mesh_src->vertices_len * sizeof(*old_global_to_new_global_idxs));

    /* i - vert index in source, j - vert index in receiver */
    for (i = 0, j = 0; (i < part_src->pvertices_len) && (j < part_src->PNumVertices); ++i)
    {
      if (part_src->PVertices[i] < 0)
        continue;

      mesh->vertices[vidx_1st + j] = (FcelibVertex *)malloc(sizeof(**mesh->vertices));
      if (!mesh->vertices[vidx_1st + j])
      {
        fprintf(stderr, "CopyPartToMesh: Cannot allocate memory (vert)\n");
        internal_pid_new = -1;
        break;
      }
      part_new->PVertices[j] = vidx_1st + j;
      FCELIB_TYPES_CpyVert(mesh->vertices[vidx_1st + j], mesh_src->vertices[ part_src->PVertices[i] ]);
      old_global_to_new_global_idxs[part_src->PVertices[i]] = vidx_1st + j;

      ++j;
    }
    if (internal_pid_new < 0)
      break;

    /* Copy triangles */
    if (!FCELIB_TYPES_AddTrianglesToPart(part_new, part_new->PNumTriangles))
    {
      internal_pid_new = -1;
      break;
    }
    if (mesh->triangles_len < tidx_1st + part_new->PNumTriangles)
    {
      if (!FCELIB_TYPES_AddTrianglesToMesh(mesh, tidx_1st + part_new->PNumTriangles - mesh->triangles_len))
      {
        internal_pid_new = -1;
        break;
      }
    }
    mesh->hdr.NumTriangles += part_new->PNumTriangles;

    /* i - triag index in source, j - triag index in receiver */
    for (i = 0, j = 0; (i < part_src->ptriangles_len) && (j < part_src->PNumTriangles); ++i)
    {
      if (part_src->PTriangles[i] < 0)
        continue;

      mesh->triangles[tidx_1st + j] = (FcelibTriangle *)malloc(sizeof(**mesh->triangles));
      if (!mesh->triangles[tidx_1st + j])
      {
        fprintf(stderr, "CopyPartToMesh: Cannot allocate memory (triag)\n");
        internal_pid_new = -1;
        break;
      }
      part_new->PTriangles[j] = tidx_1st + j;
      FCELIB_TYPES_CpyTriag(mesh->triangles[tidx_1st + j], mesh_src->triangles[ part_src->PTriangles[i] ]);

      for (n = 0; n < 3; ++n)
        mesh->triangles[tidx_1st + j]->vidx[n] = old_global_to_new_global_idxs[mesh->triangles[tidx_1st + j]->vidx[n]];

      ++j;
    }
    if (internal_pid_new < 0)
      break;

    pid_new = FCELIB_TYPES_GetOrderByInternalPartIdx(mesh, mesh->hdr.Parts[internal_pid_new]);
    if (pid_new < 0)
    {
      fprintf(stderr, "CopyPartToMesh: Cannot get new part idx\n");
      break;
    }

    break;
  }  /* for (;;) */

  free(old_global_to_new_global_idxs);

  return pid_new;
}

void FCELIB_OP_DeletePart(FcelibMesh *mesh, int pid)
{
  int i;
  int internal_pid;
  FcelibPart *part;

  for (;;)
  {
    internal_pid = FCELIB_TYPES_GetInternalPartIdxByOrder(mesh, pid);
    if (internal_pid < 0)
    {
      fprintf(stderr, "DeletePart: Invalid index (internal_pid)\n");
      break;
    }
    part = mesh->parts[ mesh->hdr.Parts[internal_pid] ];

    for (i = 0; i < part->pvertices_len; ++i)
    {
      if (part->PVertices[i] < 0)
        continue;
      free(mesh->vertices[ part->PVertices[i] ]);
      mesh->vertices[ part->PVertices[i] ] = NULL;
    }
    free(part->PVertices);

    for (i = 0; i < part->ptriangles_len; ++i)
    {
      if (part->PTriangles[i] < 0)
        continue;
      free(mesh->triangles[ part->PTriangles[i] ]);
      mesh->triangles[ part->PTriangles[i] ] = NULL;
    }
    free(part->PTriangles);

    mesh->hdr.NumVertices -= part->PNumVertices;
    mesh->hdr.NumTriangles -= part->PNumTriangles;
    --mesh->hdr.NumParts;
    free(part);
    mesh->parts[ mesh->hdr.Parts[internal_pid] ] = NULL;
    mesh->hdr.Parts[internal_pid] = -1;

    break;
  }  /* for (;;) */
}

/* Delete part triangles by order. */
int FCELIB_OP_DeletePartTriags(FcelibMesh *mesh, const int pid, const int *idxs, int idxs_len)
{
  int retv = 0;
  int i;
  int internal_pid;
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

    internal_pid = FCELIB_TYPES_GetInternalPartIdxByOrder(mesh, pid);
    if (internal_pid < 0)
    {
      fprintf(stderr, "DeletePartTriags: Invalid index (internal_pid)\n");
      break;
    }
    part = mesh->parts[ mesh->hdr.Parts[internal_pid] ];

    map = (int *)malloc(idxs_len * sizeof(*map));
    if (!map)
    {
      fprintf(stderr, "DeletePartTriags: Cannot allocate memory (map)\n");
      break;
    }

    memcpy(map, idxs, idxs_len * sizeof(*map));
    qsort(map, idxs_len, sizeof(*map), FCELIB_UTIL_CompareInts);
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
      ptr = (int *)bsearch(&i, sptr, search_len, sizeof(*map), FCELIB_UTIL_CompareInts);
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

  map = (int *)malloc(mesh->vertices_len * sizeof(*map));
  if (!map)
  {
    fprintf(stderr, "DeleteUnrefdVerts: Cannot allocate memory (map)\n");
    return 0;
  }
  memset(map, 0, mesh->vertices_len * sizeof(*map));

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
int FCELIB_OP_MergePartsToNew(FcelibMesh *mesh, int pid1, int pid2)
{
  int pid_new = -1;
  int internal_pid_new = -1;
  int internal_pid1;
  int internal_pid2;
  int i;
  int j;
  int n;
  int vidx_1st;
  int tidx_1st;
  int *old_global_to_new_global_idxs = NULL;  /* maps vert ref's in copied triags */
  FcelibPart *part_new;
  FcelibPart *part_src1;
  FcelibPart *part_src2;

  if (pid1 == pid2)
  {
    fprintf(stderr, "MergePartsToNew: Cannot merge part with itself\n");
    return -1;
  }

  internal_pid1 = FCELIB_TYPES_GetInternalPartIdxByOrder(mesh, pid1);
  internal_pid2 = FCELIB_TYPES_GetInternalPartIdxByOrder(mesh, pid2);
  if (internal_pid1 < 0)
  {
    fprintf(stderr, "MergePartsToNew: Invalid index (internal_pid1)\n");
    return -1;
  }
  if (internal_pid2 < 0)
  {
    fprintf(stderr, "MergePartsToNew: Invalid index (internal_pid2)\n");
    return -1;
  }
  part_src1 = mesh->parts[mesh->hdr.Parts[internal_pid1]];
  part_src2 = mesh->parts[mesh->hdr.Parts[internal_pid2]];

  for (;;)
  {
    if (mesh->hdr.Parts[mesh->parts_len - 1] >= 0)
    {
      if (!FCELIB_TYPES_AddParts(mesh, 1))
      {
        internal_pid_new = -1;
        break;
      }
    }

    /* Get first unused index */
    internal_pid_new = FCELIB_TYPES_GetFirstUnusedGlobalPartIdx(mesh);
    vidx_1st = FCELIB_TYPES_GetFirstUnusedGlobalVertexIdx(mesh);
    tidx_1st = FCELIB_TYPES_GetFirstUnusedGlobalTriangleIdx(mesh);


    /* Add part */
    mesh->hdr.Parts[internal_pid_new] = FCELIB_UTIL_ArrMax(mesh->hdr.Parts, mesh->parts_len) + 1;
    part_new = (FcelibPart *)malloc(sizeof(*part_new));
    if (!part_new)
    {
      fprintf(stderr, "MergePartsToNew: Cannot allocate memory (part)\n");
      internal_pid_new = -1;
      break;
    }
    memset(part_new, 0, sizeof(*part_new));
    part_new->pvertices_len = 0;
    part_new->ptriangles_len = 0;

    mesh->parts[ mesh->hdr.Parts[internal_pid_new] ] = part_new;

    sprintf(part_new->PartName, "%d_%d", internal_pid1, internal_pid2);
    part_new->PartPos.x = 0.0f;
    part_new->PartPos.y = 0.0f;
    part_new->PartPos.z = 0.0f;

    part_new->PNumTriangles = part_src1->PNumTriangles + part_src2->PNumTriangles;
    part_new->PNumVertices = part_src1->PNumVertices + part_src2->PNumVertices;

    ++mesh->hdr.NumParts;

#if SCL_DEBUG >= 1
    printf("triangles %d + %d = %d\n", part_src1->PNumTriangles, part_src2->PNumTriangles, part_new->PNumTriangles);
    printf("vertices %d + %d = %d\n", part_src1->PNumVertices, part_src2->PNumVertices, part_new->PNumVertices);
#endif

    /* Copy vertices */
    if (!FCELIB_TYPES_AddVerticesToPart(part_new, part_new->PNumVertices))
    {
      internal_pid_new = -1;
      break;
    }

    if (mesh->vertices_len < vidx_1st + part_new->PNumVertices)
    {
      if (!FCELIB_TYPES_AddVerticesToMesh(mesh, vidx_1st + part_new->PNumVertices - mesh->vertices_len))
      {
        internal_pid_new = -1;
        break;
      }
    }
    mesh->hdr.NumVertices += part_new->PNumVertices;

    old_global_to_new_global_idxs = (int *)malloc(mesh->vertices_len * sizeof(*old_global_to_new_global_idxs));
    if (!old_global_to_new_global_idxs)
    {
      fprintf(stderr, "MergePartsToNew: Cannot allocate memory (map)\n");
      internal_pid_new = -1;
      break;
    }
    memset(old_global_to_new_global_idxs, 0xFF, mesh->vertices_len * sizeof(*old_global_to_new_global_idxs));

    /* i - vert index in source, j - vert index in receiver */
    for (i = 0, j = 0; i < part_src1->pvertices_len && j < part_src1->PNumVertices; ++i)
    {
      if (part_src1->PVertices[i] < 0)
        continue;

      mesh->vertices[vidx_1st + j] = (FcelibVertex *)malloc(sizeof(**mesh->vertices));
      if (!mesh->vertices[vidx_1st + j])
      {
        fprintf(stderr, "MergePartsToNew: Cannot allocate memory (vert1)\n");
        internal_pid_new = -1;
        break;
      }
      part_new->PVertices[j] = vidx_1st + j;
      FCELIB_TYPES_CpyVert(mesh->vertices[vidx_1st + j], mesh->vertices[ part_src1->PVertices[i] ]);
      FCELIB_TYPES_VertAddPosition(mesh->vertices[vidx_1st + j], &part_src1->PartPos);
      old_global_to_new_global_idxs[part_src1->PVertices[i]] = vidx_1st + j;

      ++j;
    }
    if (internal_pid_new < 0)
      break;

    for (i = 0; i < part_src2->pvertices_len && j < part_src1->PNumVertices + part_src2->PNumVertices; ++i)
    {
      if (part_src2->PVertices[i] < 0)
        continue;

      mesh->vertices[vidx_1st + j] = (FcelibVertex *)malloc(sizeof(**mesh->vertices));
      if (!mesh->vertices[vidx_1st + j])
      {
        fprintf(stderr, "MergePartsToNew: Cannot allocate memory (vert2)\n");
        internal_pid_new = -1;
        break;
      }
      part_new->PVertices[j] = vidx_1st + j;
      FCELIB_TYPES_CpyVert(mesh->vertices[vidx_1st + j], mesh->vertices[ part_src2->PVertices[i] ]);
      FCELIB_TYPES_VertAddPosition(mesh->vertices[vidx_1st + j], &part_src2->PartPos);
      old_global_to_new_global_idxs[ part_src2->PVertices[i] ] = vidx_1st + j;

      ++j;
    }
    if (internal_pid_new < 0)
      break;

    /* Copy triangles */
    if (!FCELIB_TYPES_AddTrianglesToPart(part_new, part_new->PNumTriangles))
    {
      internal_pid_new = -1;
      break;
    }

    if (mesh->triangles_len < tidx_1st + part_new->PNumTriangles)
    {
      if (!FCELIB_TYPES_AddTrianglesToMesh(mesh, tidx_1st + part_new->PNumTriangles - mesh->triangles_len))
      {
        internal_pid_new = -1;
        break;
      }
    }
    mesh->hdr.NumTriangles += part_new->PNumTriangles;

    for (i = 0, j = 0; i < part_src1->ptriangles_len && j < part_src1->PNumTriangles; ++i)
    {
      if (part_src1->PTriangles[i] < 0)
        continue;
      mesh->triangles[tidx_1st + j] = (FcelibTriangle *)malloc(sizeof(**mesh->triangles));
      if (!mesh->triangles[tidx_1st + j])
      {
        /* fatal error? */
        fprintf(stderr, "MergePartsToNew: Cannot allocate memory (triag1)\n");
        internal_pid_new = -1;
        break;
      }
      part_new->PTriangles[j] = tidx_1st + j;
      FCELIB_TYPES_CpyTriag(mesh->triangles[tidx_1st + j], mesh->triangles[ part_src1->PTriangles[i] ]);

      for (n = 0; n < 3; ++n)
        mesh->triangles[tidx_1st + j]->vidx[n] = old_global_to_new_global_idxs[mesh->triangles[tidx_1st + j]->vidx[n]];

      ++j;
    }
    if (internal_pid_new < 0)
      break;
    for (i = 0; i < part_src2->ptriangles_len && j < part_src1->PNumTriangles + part_src2->PNumTriangles; ++i)
    {
      if (part_src2->PTriangles[i] < 0)
        continue;

      mesh->triangles[tidx_1st + j] = (FcelibTriangle *)malloc(sizeof(**mesh->triangles));
      if (!mesh->triangles[tidx_1st + j])
      {
        /* fatal error */
        fprintf(stderr, "MergePartsToNew: Cannot allocate memory (triag2)\n");
        internal_pid_new = -1;
        break;
      }
      part_new->PTriangles[j] = tidx_1st + j;
      FCELIB_TYPES_CpyTriag(mesh->triangles[tidx_1st + j], mesh->triangles[ part_src2->PTriangles[i] ]);
      for (n = 0; n < 3; ++n)
        mesh->triangles[tidx_1st + j]->vidx[n] = old_global_to_new_global_idxs[mesh->triangles[tidx_1st + j]->vidx[n]];

      ++j;
    }

    pid_new = FCELIB_TYPES_GetOrderByInternalPartIdx(mesh, internal_pid_new);
    if (pid_new < 0)
    {
      fprintf(stderr, "MergePartsToNew: Cannot get new part idx\n");
      break;
    }

    break;
  }  /* for (;;) */

  free(old_global_to_new_global_idxs);

  return pid_new;
}

/*
  Move up part in order ('up' means towards idx=0)
    If it's the first part, do nothing.
    If not, swap with previous part.
  Returns new index on success, -1 on failure.
*/
int FCELIB_OP_MoveUpPart(FcelibMesh *mesh, const int pid)
{
  int internal_pid;
  int internal_pid_previous;

  internal_pid = FCELIB_TYPES_GetInternalPartIdxByOrder(mesh, pid);
  if (internal_pid < 0)
  {
    fprintf(stderr, "MoveUpPart: part %d does not exist\n", pid);
    return -1;
  }

  internal_pid_previous = FCELIB_TYPES_GetInternalPartIdxByOrder(mesh, pid - 1);
  if (internal_pid_previous < 0)
    return pid;

  {
    const int tmp = mesh->hdr.Parts[internal_pid];
    mesh->hdr.Parts[internal_pid] = mesh->hdr.Parts[internal_pid_previous];
    mesh->hdr.Parts[internal_pid_previous] = tmp;
  }

  return pid - 1;
}

#endif  /* FCELIB_OP_H_ */
