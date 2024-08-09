/*
  fcelib_types.h
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
  implements library types, init & release, validation, some basic functionality

  parts, triags, verts are written into arrays that may contain NULL elements.
  These elements can be accessed via ordered index arrays. New elements are
  added at the end.

  Many operations are carried out on index arrays.
  Array elements (parts, triags, verts) are accessed in constant time at first.
  Once an index array has been flagged as dirty, access is of linear complexity.
**/

#ifndef FCELIB_TYPES_H_
#define FCELIB_TYPES_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./fcelib_fcetypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __cplusplus
typedef struct FcelibVertex FcelibVertex;
typedef struct FcelibTriangle FcelibTriangle;
typedef struct FcelibPart FcelibPart;
typedef struct FcelibHeader FcelibHeader;
typedef struct FcelibMesh FcelibMesh;
#endif

struct FcelibVertex {
  tVector VertPos;
  tVector NormPos;
  tVector DamgdVertPos;
  tVector DamgdNormPos;
  int     Animation;  /* 0x4 = immovable, 0x0 othw */
};

struct FcelibTriangle {
  int   tex_page;
  int   vidx[3];  /* global vertex indexes */
  int   flag;
  float U[3];
  float V[3];
};

struct FcelibPart {
  int     PNumVertices;    /* number of elements: true count for this part */
  int     pvertices_len;   /* capacity: array length */
  int     PNumTriangles;   /* number of elements: true count for this part */
  int     ptriangles_len;  /* capacity: array length */
  char    PartName[64];

  tVector PartPos;
  int    *PVertices;       /* ordered list of global vert idxs, -1 for unused */
  int    *PTriangles;      /* ordered list of global triag idxs, -1 for unused */
};

struct FcelibHeader {
  int      Unknown3;       /* FCE4M experimental */
  int      NumTriangles;
  int      NumVertices;
  int      NumArts;
  int      NumParts;       /* number of elements: true count (i.e., count of entries in *Parts that are > -1) */
  int      NumDummies;     /* <= 16 */
  int      NumColors;      /* <= 16 */
  int      NumSecColors;   /* <= 16, FCE3 only */

  tColor4  PriColors[16];
  tColor4  IntColors[16];  /* FCE4 only */
  tColor4  SecColors[16];
  tColor4  DriColors[16];  /* FCE4 only */
  tVector  Dummies[16];
  char     DummyNames[16 * 64];
  int      *Parts;         /* ordered list of part indexes, -1 for unused; length given by mesh.parts_len */
};

struct FcelibMesh {
  int              parts_len;          /* capacity: array length */
  int              triangles_len;      /* capacity: array length */
  int              vertices_len;       /* capacity: array length */

  FcelibHeader     hdr;

  /*
    Access via appropriate ordered index array only.
    for **parts, use *hdr.Parts
    for **triangles, use *FcelibPart.PTriangles
    for **vertices, use *FcelibPart.PVertices
    Also see FCELIB_TYPES_* service functions.

    Each vert and triag belongs to exactly one part, respectively.
  */
  FcelibPart     **parts;          /* may contain NULL elements */
  FcelibTriangle **triangles;      /* may contain NULL elements */
  FcelibVertex   **vertices;       /* may contain NULL elements */

  void           (*release)(struct FcelibMesh*);
};

#ifdef __cplusplus
}  /* extern "C" */
#endif

/* release, init, validate -------------------------------------------------- */

/*
  Call via mesh->release(), never directly.

  Afterwards (!mesh->release).
*/
void FCELIB_TYPES_FreeMesh(FcelibMesh *mesh)
{
  int i;
  int k;
  int n;
  FcelibPart *part;

  /* If index arrays are dirty, safe access for free'ing. */
  for (i = mesh->parts_len - 1; i >= 0 ; --i)
  {
    if (mesh->hdr.Parts[i] < 0)
      continue;
    part = mesh->parts[ mesh->hdr.Parts[i] ];

    for (n = part->pvertices_len - 1, k = part->PNumVertices - 1; n >= 0 && k >= 0; --n)
    {
      if (part->PVertices[n] < 0)
        continue;
      free(mesh->vertices[ part->PVertices[n] ]);
      --k;
    }  /* for n, k */
    free(part->PVertices);

    for (n = part->ptriangles_len - 1, k = part->PNumTriangles - 1; n >= 0 && k >= 0; --n)
    {
      if (part->PTriangles[n] < 0)
        continue;
      free(mesh->triangles[ part->PTriangles[n] ]);
      --k;
    }  /* for n, k */
    free(part->PTriangles);
  }  /* for i */

  for (i = mesh->parts_len - 1; i >= 0 ; --i)
  {
    if (mesh->hdr.Parts[i] < 0)
      continue;
    free(mesh->parts[ mesh->hdr.Parts[i] ]);
  }  /* for i */

  if (mesh->hdr.Parts)  free(mesh->hdr.Parts);
  if (mesh->parts)  free(mesh->parts);
  if (mesh->triangles)  free(mesh->triangles);
  if (mesh->vertices)  free(mesh->vertices);

  mesh->release = NULL;
}

/*
  Assumes (mesh). memset's mesh to 0. Silently re-initializes.
*/
FcelibMesh *FCELIB_TYPES_InitMesh(FcelibMesh *mesh)
{
#ifndef FCELIB_PYTHON_BINDINGS
#ifdef __cplusplus
  if (mesh->release == &FCELIB_TYPES_FreeMesh)
    mesh->release(mesh);
#endif
#endif

  memset(mesh, 0, sizeof(*mesh));
  mesh->hdr.NumArts = 1;
  mesh->release = &FCELIB_TYPES_FreeMesh;
  return mesh;
}

/* Returns: 1 = valid mesh, -1 = empty valid mesh, 0 = invalid mesh */
int FCELIB_TYPES_ValidateMesh(const FcelibMesh *mesh)
{
  int i;
  int j;
  int count_parts;
  int sum_triags = 0;
  int sum_verts = 0;
  FcelibPart *part = NULL;

  if (!mesh->release || mesh->release != &FCELIB_TYPES_FreeMesh)  return 0;

  if (mesh->parts_len == 0     && !mesh->parts     && !mesh->hdr.Parts &&
      mesh->triangles_len == 0 && !mesh->triangles &&
      mesh->vertices_len == 0  && !mesh->vertices)
    return -1;

  if (mesh->parts_len > 0 && !mesh->parts)
  {
    fprintf(stderr, "ValidateMesh: unexpected NULL pointer (mesh->parts)\n");
    return 0;
  }
  if (mesh->parts_len > 0 && !mesh->hdr.Parts)
  {
    fprintf(stderr, "ValidateMesh: unexpected NULL pointer (mesh->hdr.Parts)\n");
    return 0;
  }
  if (mesh->triangles_len > 0 && !mesh->triangles)
  {
    fprintf(stderr, "ValidateMesh: unexpected NULL pointer (mesh->triangles)\n");
    return 0;
  }
  if (mesh->vertices_len > 0 && !mesh->vertices)
  {
    fprintf(stderr, "ValidateMesh: unexpected NULL pointer (mesh->vertices)\n");
    return 0;
  }

  for (i = 0, count_parts = 0; i < mesh->parts_len; ++i)
  {
    if (mesh->hdr.Parts[i] >= mesh->parts_len)
    {
      fprintf(stderr, "ValidateMesh: inconsistent list (mesh->hdr.Parts[i])\n");
      return 0;
    }

    if (mesh->hdr.Parts[i] < 0)
      continue;
    ++count_parts;

    part = mesh->parts[mesh->hdr.Parts[i]];
    if (!part)
    {
      fprintf(stderr, "ValidateMesh: unexpected NULL pointer (mesh->parts[mesh->hdr.Parts[i]]) %d %d\n", i, mesh->hdr.Parts[i]);
      return 0;
    }

    sum_triags += part->PNumTriangles;
    sum_verts += part->PNumVertices;
  }  /* for i */

  if (count_parts != mesh->hdr.NumParts)
  {
    fprintf(stderr, "ValidateMesh: inconsistent list (%d != mesh->hdr.NumParts = %d)\n", count_parts, mesh->hdr.NumParts);
    return 0;
  }
  if (sum_triags != mesh->hdr.NumTriangles)
  {
    fprintf(stderr, "ValidateMesh: inconsistent list (%d != mesh->hdr.NumTriangles = %d)\n", sum_triags, mesh->hdr.NumTriangles);
    return 0;
  }
  if (sum_verts != mesh->hdr.NumVertices)
  {
    fprintf(stderr, "ValidateMesh: inconsistent list (%d != mesh->hdr.NumVertices = %d)\n", sum_verts, mesh->hdr.NumVertices);
    return 0;
  }

  for (i = 0; i < mesh->parts_len; ++i)
  {
    if (mesh->hdr.Parts[i] < 0)
      continue;
    part = mesh->parts[mesh->hdr.Parts[i]];
    /* if (!part) - see above */

    if (!part->PTriangles)
    {
      fprintf(stderr, "ValidateMesh: unexpected NULL pointer (part->PTriangles) %d\n", i);
      return 0;
    }
    for (j = 0, sum_triags = 0; j < part->ptriangles_len; ++j)
    {
      if (sum_triags > part->PNumTriangles)
      {
        fprintf(stderr, "ValidateMesh: invalid count (part->PNumTriangles) i%d j%d %d>%d\n", i, j, sum_triags, part->PNumTriangles);
        return 0;
      }
      if (part->PTriangles[j] >= mesh->triangles_len)
      {
        fprintf(stderr, "ValidateMesh: inconsistent list (part->PTriangles[j] = %d>=%d) %d %d\n", part->PTriangles[j], mesh->triangles_len, i, j);
        return 0;
      }
      if (part->PTriangles[j] < 0)
        continue;

      if (!mesh->triangles[part->PTriangles[j]])
      {
        fprintf(stderr, "ValidateMesh: unexpected NULL pointer (mesh->triangles[part->PTriangles[j]]) %d %d\n", i, j);
        return 0;
      }

      ++sum_triags;
    }  /* for j */
    if (sum_triags != part->PNumTriangles)
    {
      fprintf(stderr, "ValidateMesh: invalid count (part->PNumTriangles) i%d %d!=%d\n", i, sum_triags, part->PNumTriangles);
      return 0;
    }

    if (!part->PVertices)
    {
      fprintf(stderr, "ValidateMesh: unexpected NULL pointer (part->PVertices) %d\n", i);
      return 0;
    }
    for (j = 0, sum_verts = 0; j < part->pvertices_len; ++j)
    {
      if (sum_verts > part->PNumVertices)
      {
        fprintf(stderr, "ValidateMesh: invalid count (part->PNumVertices) i%d j%d %d>%d\n", i, j, sum_verts, part->PNumVertices);
        return 0;
      }
      if (part->PVertices[j] >= mesh->vertices_len)
      {
        fprintf(stderr, "ValidateMesh: inconsistent list (part->PVertices[j]) %d %d\n", i, j);
        return 0;
      }

      if (part->PVertices[j] < 0)
        continue;

      if (!mesh->vertices[part->PVertices[j]])
      {
        fprintf(stderr, "ValidateMesh: unexpected NULL pointer (mesh->vertices[part->PVertices[j]]) %d %d\n", i, j);
        return 0;
      }

      ++sum_verts;
    }  /* for j */
    if (sum_verts != part->PNumVertices)
    {
      fprintf(stderr, "ValidateMesh: invalid count (part->PNumVertices) i%d %d!=%d\n", i, sum_verts, part->PNumVertices);
      return 0;
    }
  }  /* for i */

  return 1;
}

/* service ------------------------------------------------------------------ */

int FCELIB_TYPES_GetFirstUnusedGlobalPartIdx(const FcelibMesh *mesh)
{
  int pidx = mesh->parts_len - 1;

  while (pidx >= 0 && mesh->hdr.Parts[pidx] < 0)
    --pidx;
  ++pidx;

  return pidx;
}

/* Assumes mesh->hdr.NumParts > 0 */
int FCELIB_TYPES_GetFirstUnusedGlobalTriangleIdx(const FcelibMesh *mesh)
{
  int tidx = -1;
  int i;
  int pidx;
  FcelibPart *part;

  for (i = 0; i < mesh->parts_len; ++i)
  {
    pidx = mesh->hdr.Parts[i];
    if (pidx < 0)
      continue;
    part = mesh->parts[pidx];
    if (part->ptriangles_len > 0)
      tidx = -FCELIB_UTIL_Min(-tidx, -FCELIB_UTIL_ArrMax(part->PTriangles, part->ptriangles_len));
  }

  return tidx + 1;
}

/* Assumes mesh->hdr.NumParts > 0 */
int FCELIB_TYPES_GetFirstUnusedGlobalVertexIdx(const FcelibMesh *mesh)
{
  int vidx = -1;
  int i;
  int pidx;
  FcelibPart *part;

  for (i = 0; i < mesh->parts_len; ++i)
  {
    pidx = mesh->hdr.Parts[i];
    if (pidx < 0)
      continue;
    part = mesh->parts[pidx];
    if (part->pvertices_len > 0)
      vidx = -FCELIB_UTIL_Min(-vidx, -FCELIB_UTIL_ArrMax(part->PVertices, part->pvertices_len));
  }

  return vidx + 1;
}

#if 0
/* experimental */

/* Not usually called directly. */
int __FCELIB_TYPES_GetInternalIndex(int *indexes, const int indexes_len, const int idx)
{
  int internal_idx;
  for (internal_idx = 0; internal_idx < indexes_len; ++internal_idx)
  {
    if (internal_idx == idx)
    {
      return internal_idx;
    }
  }
  return -1;
}

int FCELIB_TYPES_GetInternalTriangleIndex(FcelibMesh *mesh, const int internal_pidx, const int tidx)
{
  if ((mesh->state & kFceLibFlagTrianglesDirty) == 0)
  {
    return tidx;
  }
  else
  {
    int internal_idx = -1;
    FcelibPart *part = mesh->parts[internal_pidx];
    return __FCELIB_TYPES_GetInternalIndex(part->PTriangles, part->ptriangles_len, tidx);
  }
}
#endif

/* Returns -1 on failure. */
int FCELIB_TYPES_GetInternalPartIdxByOrder(const FcelibMesh *mesh, const int order)
{
  int pidx = -1;
  int count;

  for (;;)
  {
    if ((order < 0) || (order >= mesh->parts_len))
    {
      fprintf(stderr, "GetInternalPartIdxByOrder: part %d not found (len=%d)\n", order, mesh->parts_len);
      break;
    }

    for (pidx = 0, count = -1; pidx < mesh->parts_len; ++pidx)
    {
      if (mesh->hdr.Parts[pidx] > -1)
        ++count;
      if (count == order)
        break;
    }

    if (pidx == mesh->parts_len)
    {
      fprintf(stderr, "GetInternalPartIdxByOrder: part %d not found\n", order);
      pidx = -1;
      break;
    }

    break;
  }

  return pidx;
}

/* Returns -1 on failure. */
int FCELIB_TYPES_GetOrderByInternalPartIdx(const FcelibMesh *mesh, const int idx)
{
  int order = -1;
  int i;

  for (;;)
  {
    if ((idx < 0) || (idx >= mesh->parts_len))
    {
      fprintf(stderr, "GetOrderByInternalPartIdx: internal part %d not found (len=%d)\n", idx, mesh->parts_len);
      break;
    }

    for (i = 0, order = -1; i < mesh->parts_len; ++i)
    {
      if (mesh->hdr.Parts[i] > -1)
        ++order;
      if (mesh->hdr.Parts[i] == idx)
        break;
    }

    if (i == mesh->parts_len)
    {
      fprintf(stderr, "GetOrderByInternalPartIdx: internal part %d not found\n", idx);
      order = -1;
      break;
    }

    break;
  }

  return order;
}

int FCELIB_TYPES_AddParts(FcelibMesh *mesh, const int num_required)
{
  void *ptr = NULL;
  int new_len = mesh->parts_len + num_required;

  ptr = realloc(mesh->hdr.Parts, new_len * sizeof(*mesh->hdr.Parts));
  if (!ptr)
  {
    fprintf(stderr, "FCELIB_TYPES_AddParts: Cannot reallocate memory (hdr.Parts)\n");
    return 0;
  }
  mesh->hdr.Parts = (int *)ptr;
  ptr = NULL;
  /* for signed int, -1 is represented as 0xFFFFFFFF */
  memset(mesh->hdr.Parts + mesh->parts_len, 0xFF, (new_len - mesh->parts_len) * sizeof(*mesh->hdr.Parts));

  ptr = realloc(mesh->parts, new_len * sizeof(*mesh->parts));
  if (!ptr)
  {
    fprintf(stderr, "FCELIB_TYPES_AddParts: Cannot reallocate memory (parts)\n");
    return 0;
  }
  mesh->parts = (FcelibPart **)ptr;
  ptr = NULL;
  memset(mesh->parts + mesh->parts_len, 0, (new_len - mesh->parts_len) * sizeof(*mesh->parts));

  mesh->parts_len = new_len;
  return 1;
}

/* mesh->hdr.NumTriangles is not changed */
int FCELIB_TYPES_AddTrianglesToMesh(FcelibMesh *mesh, const int num_required)
{
  void *ptr = NULL;

  ptr = realloc(mesh->triangles, (mesh->triangles_len + num_required) * sizeof(*mesh->triangles));
  if (!ptr)
  {
    fprintf(stderr, "FCELIB_TYPES_AddTriangles: Cannot reallocate memory\n");
    return 0;
  }
  mesh->triangles = (FcelibTriangle **)ptr;
  ptr = NULL;
  memset(mesh->triangles + mesh->triangles_len, 0, num_required * sizeof(*mesh->triangles));

  mesh->triangles_len += num_required;
  return 1;
}

/* mesh->hdr.NumVertices is not changed */
int FCELIB_TYPES_AddVerticesToMesh(FcelibMesh *mesh, const int num_required)
{
  void *ptr = NULL;

  ptr = realloc(mesh->vertices, (mesh->vertices_len + num_required) * sizeof(*mesh->vertices));
  if (!ptr)
  {
    fprintf(stderr, "FCELIB_TYPES_AddVertices: Cannot reallocate memory\n");
    return 0;
  }
  mesh->vertices = (FcelibVertex **)ptr;
  ptr = NULL;
  memset(mesh->vertices + mesh->vertices_len, 0, num_required * sizeof(*mesh->vertices));

  mesh->vertices_len += num_required;
  return 1;
}

int FCELIB_TYPES_AddTrianglesToPart(FcelibPart *part, const int num_required)
{
  void *ptr = NULL;

  part->ptriangles_len += num_required;
  ptr = realloc(part->PTriangles, part->ptriangles_len * sizeof(*part->PTriangles));
  if (!ptr)
  {
    fprintf(stderr, "AddTriangles2: Cannot reallocate memory (part->PTriangles)\n");
    return 0;
  }
  part->PTriangles = (int *)ptr;
  ptr = NULL;
  /* for signed int, -1 is represented as 0xFFFFFFFF */
  memset(part->PTriangles, 0xFF, part->ptriangles_len * sizeof(*part->PTriangles));

  return 1;
}

int FCELIB_TYPES_AddVerticesToPart(FcelibPart *part, const int num_required)
{
  void *ptr = NULL;

  part->pvertices_len += num_required;
  ptr = realloc(part->PVertices, part->pvertices_len * sizeof(*part->PVertices));
  if (!ptr)
  {
    fprintf(stderr, "AddVertices2: Cannot reallocate memory (part->PVertices)\n");
    return 0;
  }
  part->PVertices = (int *)ptr;
  ptr = NULL;
  /* for signed int, -1 is represented as 0xFFFFFFFF */
  memset(part->PVertices, 0xFF, part->pvertices_len * sizeof(*part->PVertices));

  return 1;
}

void FCELIB_TYPES_CpyTriag(FcelibTriangle *dest, const FcelibTriangle *src)
{
  dest->tex_page = src->tex_page;
  memcpy(dest->vidx, src->vidx, sizeof(src->vidx));
  dest->flag = src->flag;
  memcpy(dest->U, src->U, sizeof(src->U));
  memcpy(dest->V, src->V, sizeof(src->V));
}

void FCELIB_TYPES_CpyVert(FcelibVertex *dest, const FcelibVertex *src)
{
  memcpy(&dest->VertPos.x, &src->VertPos.x, sizeof(float));
  memcpy(&dest->VertPos.y, &src->VertPos.y, sizeof(float));
  memcpy(&dest->VertPos.z, &src->VertPos.z, sizeof(float));
  memcpy(&dest->NormPos.x, &src->NormPos.x, sizeof(float));
  memcpy(&dest->NormPos.y, &src->NormPos.y, sizeof(float));
  memcpy(&dest->NormPos.z, &src->NormPos.z, sizeof(float));
  memcpy(&dest->DamgdVertPos.x, &src->DamgdVertPos.x, sizeof(float));
  memcpy(&dest->DamgdVertPos.y, &src->DamgdVertPos.y, sizeof(float));
  memcpy(&dest->DamgdVertPos.z, &src->DamgdVertPos.z, sizeof(float));
  memcpy(&dest->DamgdNormPos.x, &src->DamgdNormPos.x, sizeof(float));
  memcpy(&dest->DamgdNormPos.y, &src->DamgdNormPos.y, sizeof(float));
  memcpy(&dest->DamgdNormPos.z, &src->DamgdNormPos.z, sizeof(float));
  dest->Animation = src->Animation;
}

/* Does not change vertex normal. */
void FCELIB_TYPES_VertAddPosition(FcelibVertex *vert, const tVector *pos)
{
  vert->VertPos.x += pos->x;
  vert->VertPos.y += pos->y;
  vert->VertPos.z += pos->z;
  vert->DamgdVertPos.x += pos->x;
  vert->DamgdVertPos.y += pos->y;
  vert->DamgdVertPos.z += pos->z;
}

/* Assumes part belongs to mesh. Assumes centroid is not NULL. Result in centroid. */
int FCELIB_TYPES_GetPartCentroid(const FcelibMesh *mesh, const FcelibPart *part, tVector *centroid)
{
  int retv = 0;
  int i;
  float* x_arr;
  float* y_arr;
  float* z_arr;
  float* xyz_arr;
  const int PNumVertices = part->PNumVertices;
  FcelibVertex *vert;
  int count_verts = 0;

  for (;;)
  {
    if (PNumVertices == 0)
    {
      memset(centroid, 0, sizeof(*centroid));
      retv = 1;
      break;
    }

    xyz_arr = (float *)malloc(3 * (PNumVertices + 1) * sizeof(*xyz_arr));
    if (!xyz_arr)
    {
      fprintf(stderr, "GetPartLocalCentroid: Cannot allocate memory\n");
      break;
    }
    memset(xyz_arr, 0, 3 * (PNumVertices + 1) * sizeof(*xyz_arr));

    x_arr = xyz_arr;
    y_arr = xyz_arr + PNumVertices;
    z_arr = xyz_arr + PNumVertices * 2;

    /* i - internal vert index, count_verts - vert order */
    for (i = 0; i < part->pvertices_len && count_verts < PNumVertices; ++i)
    {
      if (part->PVertices[i] < 0)
        continue;

      vert = mesh->vertices[ part->PVertices[i] ];
      x_arr[count_verts] = vert->VertPos.x + part->PartPos.x;
      y_arr[count_verts] = vert->VertPos.y + part->PartPos.y;
      z_arr[count_verts] = vert->VertPos.z + part->PartPos.z;
#if FCECVERBOSE >= 2
      printf("%d: %f = %f + %f\n", count_verts, x_arr[count_verts], vert->VertPos.x, part->PartPos.x);
      printf("%d: %f = %f + %f\n", count_verts, y_arr[count_verts], vert->VertPos.y, part->PartPos.y);
      printf("%d: %f = %f + %f\n", count_verts, z_arr[count_verts], vert->VertPos.z, part->PartPos.z);
      printf("\n");
#endif

      ++count_verts;
    }

    qsort(x_arr, count_verts, sizeof(*x_arr), FCELIB_UTIL_CompareFloats);
    qsort(y_arr, count_verts, sizeof(*y_arr), FCELIB_UTIL_CompareFloats);
    qsort(z_arr, count_verts, sizeof(*z_arr), FCELIB_UTIL_CompareFloats);
#if FCECVERBOSE >= 2
    printf("<%s> min: (%f, %f, %f) max: (%f, %f, %f)\n", part->PartName, x_arr[0], y_arr[0], z_arr[0], x_arr[count_verts - 1], y_arr[count_verts - 1], z_arr[count_verts - 1]);
#endif
    centroid->x = 0.5f * FCELIB_UTIL_Abs(x_arr[count_verts - 1] - x_arr[0]) + x_arr[0];
    centroid->y = 0.5f * FCELIB_UTIL_Abs(y_arr[count_verts - 1] - y_arr[0]) + y_arr[0];
    centroid->z = 0.5f * FCELIB_UTIL_Abs(z_arr[count_verts - 1] - z_arr[0]) + z_arr[0];
#if FCECVERBOSE >= 2
    printf("centroid->x: %f (%f, %f)\n", centroid->x, x_arr[count_verts - 1], x_arr[0]);
    printf("centroid->y: %f (%f, %f)\n", centroid->y, y_arr[count_verts - 1], y_arr[0]);
    printf("centroid->z: %f (%f, %f)\n", centroid->z, z_arr[count_verts - 1], z_arr[0]);
#endif

    free(xyz_arr);

    retv = 1;
    break;
  }

  return retv;
}

/*
  Does not move part w.r.t. to global coordinates
  Assumes part belongs to mesh.
*/
void FCELIB_TYPES_ResetPartCenter(const FcelibMesh *mesh, FcelibPart *part, const tVector new_PartPos)
{
  FcelibVertex *vert;
  int count_verts = 0;
  int i;
  for (i = 0; i < part->pvertices_len && count_verts < part->PNumVertices; ++i)
  {
    if (part->PVertices[i] < 0)
      continue;
    vert = mesh->vertices[ part->PVertices[i] ];
    vert->VertPos.x += part->PartPos.x - new_PartPos.x;
    vert->VertPos.y += part->PartPos.y - new_PartPos.y;
    vert->VertPos.z += part->PartPos.z - new_PartPos.z;
    vert->DamgdVertPos.x += part->PartPos.x - new_PartPos.x;
    vert->DamgdVertPos.y += part->PartPos.y - new_PartPos.y;
    vert->DamgdVertPos.z += part->PartPos.z - new_PartPos.z;
    ++count_verts;
  }
  memcpy(&part->PartPos.x, &new_PartPos.x, sizeof(float));
  memcpy(&part->PartPos.y, &new_PartPos.y, sizeof(float));
  memcpy(&part->PartPos.z, &new_PartPos.z, sizeof(float));
}

/* stats -------------------------------------------------------------------- */

void FCELIB_TYPES_PrintMeshInfo(const FcelibMesh *mesh)
{
  int i;
  int j;
  int verts = 0;
  int triags = 0;

  printf("NumTriangles (true) = %d\n", mesh->hdr.NumTriangles);
  printf("triangles_len (alloc'd) = %d\n", mesh->triangles_len);

  printf("NumVertices (true) = %d\n", mesh->hdr.NumVertices);
  printf("vertices_len (alloc'd) = %d\n", mesh->vertices_len);

  printf("NumParts (true) = %d\n", mesh->hdr.NumParts);
  printf("parts_len (alloc'd) = %d\n", mesh->parts_len);

  printf("NumArts = %d\n", mesh->hdr.NumArts);
  printf("NumDummies = %d\n", mesh->hdr.NumDummies);
  printf("NumColors = %d\n", mesh->hdr.NumColors);
  printf("NumSecColors = %d\n", mesh->hdr.NumSecColors);

  printf("Unknown3 (0x0924) = %d (0x%04x)\n", mesh->hdr.Unknown3, mesh->hdr.Unknown3);

  printf("Parts:\n"
         "Ord Idx   Verts  Triangles  (PartPos)                          FCE3 role            Name\n");
  for (i = 0, j = 0; i < mesh->parts_len; ++i)
  {
    if (mesh->hdr.Parts[i] < 0)
      continue;

    printf(" %2d  %2d   %5d      %5d  (%9f, %9f, %9f)  %20s %s\n",
           j,
           mesh->hdr.Parts[i],
           mesh->parts[mesh->hdr.Parts[i]]->PNumVertices,
           mesh->parts[mesh->hdr.Parts[i]]->PNumTriangles,
           mesh->parts[mesh->hdr.Parts[i]]->PartPos.x, mesh->parts[mesh->hdr.Parts[i]]->PartPos.y, mesh->parts[mesh->hdr.Parts[i]]->PartPos.z,
           j < kFceLibImplementedFce3Parts ? kFce3PartsNames[j] : "",
           mesh->parts[mesh->hdr.Parts[i]]->PartName);

    verts  += mesh->parts[mesh->hdr.Parts[i]]->PNumVertices;
    triags += mesh->parts[mesh->hdr.Parts[i]]->PNumTriangles;

    ++j;
  }
  printf("    = %5d    = %5d\n",
         verts, triags);

  printf("DummyNames (Position):\n");
  for (i = 0; i < mesh->hdr.NumDummies; ++i)
  {
    printf(" %2d  (%9f, %9f, %9f) %.64s\n", i,
           mesh->hdr.Dummies[i].x, mesh->hdr.Dummies[i].y, mesh->hdr.Dummies[i].z,
           mesh->hdr.DummyNames + (i * 64));
  }

  printf("Car colors (hue, saturation, brightness, transparency):\n");
  for (i = 0; i < mesh->hdr.NumColors; ++i)
  {
    printf(" %2d  Primary     %3d, %3d, %3d, %3d\n", i,
          mesh->hdr.PriColors[i].hue, mesh->hdr.PriColors[i].saturation,
          mesh->hdr.PriColors[i].brightness, mesh->hdr.PriColors[i].transparency);
    printf(" %2d  Interior    %3d, %3d, %3d, %3d\n", i,
          mesh->hdr.IntColors[i].hue, mesh->hdr.IntColors[i].saturation,
          mesh->hdr.IntColors[i].brightness, mesh->hdr.IntColors[i].transparency);
    printf(" %2d  Secondary   %3d, %3d, %3d, %3d\n", i,
          mesh->hdr.SecColors[i].hue, mesh->hdr.SecColors[i].saturation,
          mesh->hdr.SecColors[i].brightness, mesh->hdr.SecColors[i].transparency);
    printf(" %2d  Driver hair %3d, %3d, %3d, %3d\n", i,
          mesh->hdr.DriColors[i].hue, mesh->hdr.DriColors[i].saturation,
          mesh->hdr.DriColors[i].brightness, mesh->hdr.DriColors[i].transparency);
  }
#ifndef FCELIB_PYTHON_BINDINGS
  fflush(stdout);
#endif
}

/* Debug: Prints ref'ed global part indexes. */
void FCELIB_TYPES_PrintMeshParts(const FcelibMesh *mesh)
{
  int j;

  printf("NumParts = %d, parts_len = %d, [\n",
         mesh->hdr.NumParts, mesh->parts_len);

  for (j = 0; j < mesh->parts_len; ++j)
    printf("%d, ", mesh->hdr.Parts[j]);

  printf("\n]\n");
#ifndef FCELIB_PYTHON_BINDINGS
  fflush(stdout);
#endif
}

/* Debug: Prints ref'ed global triag indexes for each part. */
void FCELIB_TYPES_PrintMeshTriangles(const FcelibMesh *mesh)
{
  int i;
  int j;

  for (i = 0; i < mesh->parts_len; ++i)
  {
    if (mesh->hdr.Parts[i] < 0)
      continue;

    printf("Part %d '%s', PNumTriangles = %d, ptriangles_len = %d, [\n",
           i, mesh->parts[mesh->hdr.Parts[i]]->PartName,
           mesh->parts[mesh->hdr.Parts[i]]->PNumTriangles, mesh->parts[mesh->hdr.Parts[i]]->ptriangles_len);

    for (j = 0; j < mesh->parts[mesh->hdr.Parts[i]]->ptriangles_len; ++j)
      printf("%d, ", mesh->parts[mesh->hdr.Parts[i]]->PTriangles[j]);

    printf("\n]\n");
  }
#ifndef FCELIB_PYTHON_BINDINGS
  fflush(stdout);
#endif
}

/* Debug: Prints ref'ed global vert indexes for each part. */
void FCELIB_TYPES_PrintMeshVertices(const FcelibMesh *mesh)
{
  int i;
  int j;

  for (i = 0; i < mesh->parts_len; ++i)
  {
    if (mesh->hdr.Parts[i] < 0)
      continue;

    printf("Part %d '%s', PNumVertices = %d, pvertices_len = %d, [\n",
           i, mesh->parts[mesh->hdr.Parts[i]]->PartName,
           mesh->parts[mesh->hdr.Parts[i]]->PNumVertices, mesh->parts[mesh->hdr.Parts[i]]->pvertices_len);

    for (j = 0; j < mesh->parts[mesh->hdr.Parts[i]]->pvertices_len; ++j)
      printf("%d, ", mesh->parts[mesh->hdr.Parts[i]]->PVertices[j]);

    printf("\n]\n");
  }
#ifndef FCELIB_PYTHON_BINDINGS
  fflush(stdout);
#endif
}

#endif  /* FCELIB_TYPES_H_ */
