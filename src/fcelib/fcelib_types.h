/*
  fcelib_types.h
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
  implements library types, init & free, validation, some basic functionality.

  parts, triags, verts are written into arrays that may contain NULL elements.
  These elements can be accessed via ordered index arrays. New elements are
  added at the end.
 **/


#ifndef FCELIB_TYPES_H
#define FCELIB_TYPES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fcelib_fcetypes.h"

#ifdef __cplusplus
namespace fcelib {
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  tVector VertPos;
  tVector NormPos;
  tVector DamgdVertPos;
  tVector DamgdNormPos;
  int     Animation;  /* 0x4 = immovable, 0x0 othw */
} FcelibVertex;

typedef struct {
  int   tex_page;
  int   vidx[3];  /* global vertex indexes */
  int   flag;
  float U[3];
  float V[3];
} FcelibTriangle;

typedef struct {
  char    PartName[64];
  tVector PartPos;

  int     PNumVertices;    /* true count for this part */
  int     pvertices_len;   /* array length */
  int    *PVertices;       /* ordered list of global vert idxs, -1 for unused */

  int     PNumTriangles;   /* true count for this part */
  int     ptriangles_len;  /* array length */
  int    *PTriangles;      /* ordered list of global triag idxs, -1 for unused */
} FcelibPart;

typedef struct FcelibHeader {
  int      NumTriangles;
  int      NumVertices;
  int      NumArts = 1;

  int      NumParts;       /* true count */
  int      *Parts;         /* ordered list of part indexes, -1 for unused */

  int      NumDummies;     /* <= 16 */
  tVector  Dummies[16];
  char     DummyNames[16 * 64];

  int      NumColors;      /* <= 16 */
  int      NumSecColors;   /* <= 16, FCE3 only */
  tColor4  PriColors[16];
  tColor4  IntColors[16];  /* FCE4 only */
  tColor4  SecColors[16];
  tColor4  DriColors[16];  /* FCE4 only */
} FcelibHeader;

/* Wnon-c-typedef-for-linkage, http://wg21.link/p1766r1 */
/* potential issue with C++03 https://github.com/tinyobjloader/tinyobjloader/issues/259#issuecomment-590675708 */
typedef struct FcelibMesh {
  int              freed = 1;      /* has instance been destroyed before? */

  FcelibHeader     hdr;

  int              parts_len;      /* array length */
  FcelibPart     **parts;          /* may contain NULL elements */

  /* Each vert and triag belongs to exactly one part, respectively. */
  int              triangles_len;  /* array length */
  FcelibTriangle **triangles;      /* may contain NULL elements */
  int              vertices_len;   /* array length */
  FcelibVertex   **vertices;       /* may contain NULL elements */
} FcelibMesh;


/* init, destroy, validate -------------------------------------------------- */

void FCELIB_TYPES_InitMesh(FcelibMesh *mesh)
{
  int i;

  if (mesh->freed != 1)
    fprintf(stderr, "Warning: InitMesh: mesh is not free'd (requires FCELIB_FreeMesh)\n");
  mesh->freed = 1;

  mesh->hdr.NumTriangles = 0;
  mesh->hdr.NumVertices = 0;

  mesh->hdr.NumParts = 0;
  mesh->hdr.Parts = NULL;

  mesh->hdr.NumDummies = 0;
  for (i = 0; i < 16; ++i)
  {
    mesh->hdr.Dummies[i].x = 0.0f;
    mesh->hdr.Dummies[i].y = 0.0f;
    mesh->hdr.Dummies[i].z = 0.0f;
  }
  memset(mesh->hdr.DummyNames, '\0', sizeof(mesh->hdr.DummyNames));

  mesh->hdr.NumColors = 0;
  mesh->hdr.NumSecColors = 0;
  for (i = 0; i < 16; ++i)
  {
    mesh->hdr.PriColors[i].hue = '\0';
    mesh->hdr.PriColors[i].saturation = '\0';
    mesh->hdr.PriColors[i].brightness = '\0';
    mesh->hdr.PriColors[i].transparency = '\0';

    mesh->hdr.IntColors[i].hue = '\0';
    mesh->hdr.IntColors[i].saturation = '\0';
    mesh->hdr.IntColors[i].brightness = '\0';
    mesh->hdr.IntColors[i].transparency = '\0';

    mesh->hdr.SecColors[i].hue = '\0';
    mesh->hdr.SecColors[i].saturation = '\0';
    mesh->hdr.SecColors[i].brightness = '\0';
    mesh->hdr.SecColors[i].transparency = '\0';

    mesh->hdr.DriColors[i].hue = '\0';
    mesh->hdr.DriColors[i].saturation = '\0';
    mesh->hdr.DriColors[i].brightness = '\0';
    mesh->hdr.DriColors[i].transparency = '\0';
  }

  mesh->parts_len = 0;
  mesh->parts = NULL;
  mesh->triangles_len = 0;
  mesh->triangles = NULL;
  mesh->vertices_len = 0;
  mesh->vertices = NULL;
}

void FCELIB_TYPES_FreeMesh(FcelibMesh *mesh)
{
  int i;
  int k;
  int n;
  FcelibPart *part;

  if (mesh->freed == 1)
    return;

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
    part = mesh->parts[ mesh->hdr.Parts[i] ];
    free(part);
  }  /* for i */

  if (mesh->triangles)
    free(mesh->triangles);
  mesh->triangles = NULL;
  if (mesh->vertices)
    free(mesh->vertices);
  mesh->vertices = NULL;
  if (mesh->parts)
    free(mesh->parts);
  mesh->parts = NULL;
  if (mesh->hdr.Parts)
    free(mesh->hdr.Parts);
  mesh->hdr.Parts = NULL;

  mesh->parts_len = 0;
  mesh->triangles_len = 0;
  mesh->vertices_len = 0;

  mesh->hdr.NumParts = 0;
  mesh->hdr.NumTriangles = 0;
  mesh->hdr.NumVertices = 0;

  mesh->hdr.NumDummies = 0;
  for (i = 0; i < 16; ++i)
  {
    mesh->hdr.Dummies[i].x = 0.0;
    mesh->hdr.Dummies[i].y = 0.0;
    mesh->hdr.Dummies[i].z = 0.0;
  }
  memset(mesh->hdr.DummyNames, '\0', sizeof(mesh->hdr.DummyNames));

  mesh->hdr.NumColors = 0;
  mesh->hdr.NumSecColors = 0;
  for (i = 0; i < 16; ++i)
  {
    mesh->hdr.PriColors[i].hue = '\0';
    mesh->hdr.PriColors[i].saturation = '\0';
    mesh->hdr.PriColors[i].brightness = '\0';
    mesh->hdr.PriColors[i].transparency = '\0';

    mesh->hdr.IntColors[i].hue = '\0';
    mesh->hdr.IntColors[i].saturation = '\0';
    mesh->hdr.IntColors[i].brightness = '\0';
    mesh->hdr.IntColors[i].transparency = '\0';

    mesh->hdr.SecColors[i].hue = '\0';
    mesh->hdr.SecColors[i].saturation = '\0';
    mesh->hdr.SecColors[i].brightness = '\0';
    mesh->hdr.SecColors[i].transparency = '\0';

    mesh->hdr.DriColors[i].hue = '\0';
    mesh->hdr.DriColors[i].saturation = '\0';
    mesh->hdr.DriColors[i].brightness = '\0';
    mesh->hdr.DriColors[i].transparency = '\0';
  }

  mesh->freed = 1;
}

/* Returns: 1 = valid mesh, -1 = empty valid mesh, 0 = invalid mesh */
int FCELIB_TYPES_ValidateMesh(const FcelibMesh mesh)
{
  int i;
  int j;
  int count_parts;
  int sum_triags = 0;
  int sum_verts = 0;
  FcelibPart *part = NULL;

  if (mesh.parts_len == 0     && !mesh.parts     && !mesh.hdr.Parts &&
      mesh.triangles_len == 0 && !mesh.triangles &&
      mesh.vertices_len == 0  && !mesh.vertices)
    return -1;

  if (mesh.parts_len > 0 && !mesh.parts)
  {
    fprintf(stderr, "ValidateMesh: unexpected NULL pointer (mesh.parts)\n");
    return 0;
  }
  if (mesh.parts_len > 0 && !mesh.hdr.Parts)
  {
    fprintf(stderr, "ValidateMesh: unexpected NULL pointer (mesh.hdr.Parts)\n");
    return 0;
  }
  if (mesh.triangles_len > 0 && !mesh.triangles)
  {
    fprintf(stderr, "ValidateMesh: unexpected NULL pointer (mesh.triangles)\n");
    return 0;
  }
  if (mesh.vertices_len > 0 && !mesh.vertices)
  {
    fprintf(stderr, "ValidateMesh: unexpected NULL pointer (mesh.vertices)\n");
    return 0;
  }

  for (i = 0, count_parts = 0; i < mesh.parts_len; ++i)
  {
    if (mesh.hdr.Parts[i] >= mesh.parts_len)
    {
      fprintf(stderr, "ValidateMesh: inconsistent list (mesh.hdr.Parts[i])\n");
      return 0;
    }

    if (mesh.hdr.Parts[i] < 0)
      continue;
    ++count_parts;

    part = mesh.parts[mesh.hdr.Parts[i]];
    if (!part)
    {
      fprintf(stderr, "ValidateMesh: unexpected NULL pointer (mesh.parts[mesh.hdr.Parts[i]]) %d %d\n", i, mesh.hdr.Parts[i]);
      return 0;
    }

    sum_triags += part->PNumTriangles;
    sum_verts += part->PNumVertices;
  }  /* for i */

  if (count_parts != mesh.hdr.NumParts)
  {
    fprintf(stderr, "ValidateMesh: inconsistent list (mesh.hdr.NumParts)\n");
    return 0;
  }
  if (sum_triags != mesh.hdr.NumTriangles)
  {
    fprintf(stderr, "ValidateMesh: inconsistent list (%d != mesh.hdr.NumTriangles = %d)\n", sum_triags, mesh.hdr.NumTriangles);
    return 0;
  }
  if (sum_verts != mesh.hdr.NumVertices)
  {
    fprintf(stderr, "ValidateMesh: inconsistent list (%d != mesh.hdr.NumVertices = %d)\n", sum_verts, mesh.hdr.NumVertices);
    return 0;
  }

  for (i = 0; i < mesh.parts_len; ++i)
  {
    if (mesh.hdr.Parts[i] < 0)
      continue;
    part = mesh.parts[mesh.hdr.Parts[i]];
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
      if (part->PTriangles[j] >= mesh.triangles_len)
      {
        fprintf(stderr, "ValidateMesh: inconsistent list (part->PTriangles[j] = %d>=%d) %d %d\n", part->PTriangles[j], mesh.triangles_len, i, j);
        return 0;
      }
      if (part->PTriangles[j] < 0)
        continue;

      if (!mesh.triangles[part->PTriangles[j]])
      {
        fprintf(stderr, "ValidateMesh: unexpected NULL pointer (mesh.triangles[part->PTriangles[j]]) %d %d\n", i, j);
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
      if (part->PVertices[j] >= mesh.vertices_len)
      {
        fprintf(stderr, "ValidateMesh: inconsistent list (part->PVertices[j]) %d %d\n", i, j);
        return 0;
      }

      if (part->PVertices[j] < 0)
        continue;

      if (!mesh.vertices[part->PVertices[j]])
      {
        fprintf(stderr, "ValidateMesh: unexpected NULL pointer (mesh.vertices[part->PVertices[j]]) %d %d\n", i, j);
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

/* Assumes mesh->hdr.NumParts > 0 */
int FCELIB_TYPES_GetFirstUnusedGlobalTriangleIdx(const FcelibMesh *mesh)
{
  int tidx = -1;
  int i;
  FcelibPart *part;

#if FCECVERBOSE == 1
    fprintf(stdout, "GetFirstUnusedGlobalTriangleIdx: ");
#endif

  /* Get internally last part (has internally last verts) */
  i = FCELIB_MISC_ArrMax(mesh->hdr.Parts, mesh->parts_len);
#if FCECVERBOSE == 1
    fprintf(stdout, "first unused triag index in part... %d", i);
#endif

  /* Get internally last triag */
  if (i >= 0)
  {
    part = mesh->parts[i];
#if FCECVERBOSE == 1
    fprintf(stdout, " %s (%d)", part->PartName, part->ptriangles_len);
#endif

    if (part->ptriangles_len > 0)
      tidx = FCELIB_MISC_ArrMax(part->PTriangles, part->ptriangles_len);
  }
#if FCECVERBOSE == 1
    fprintf(stdout, "\n");
#endif

  return tidx + 1;
}

/* Assumes mesh->hdr.NumParts > 0 */
int FCELIB_TYPES_GetFirstUnusedGlobalVertexIdx(const FcelibMesh *mesh)
{
  int vidx = -1;
  int i;
  FcelibPart *part;

#if FCECVERBOSE == 1
    fprintf(stdout, "GetFirstUnusedGlobalVertexIdx: ");
#endif

  /* Get internally last part (has internally last verts) */
  i = FCELIB_MISC_ArrMax(mesh->hdr.Parts, mesh->parts_len);
#if FCECVERBOSE == 1
    fprintf(stdout, "first unused vert index in part... %d", i);
#endif

  /* Get internally last vert */
  if (i >= 0)
  {
    part = mesh->parts[i];
#if FCECVERBOSE == 1
    fprintf(stdout, " %s (%d)", part->PartName, part->pvertices_len);
#endif

    if (part->pvertices_len > 0)
      vidx = FCELIB_MISC_ArrMax(part->PVertices, part->pvertices_len);
  }
#if FCECVERBOSE == 1
    fprintf(stdout, "\n");
#endif

  return vidx + 1;
}

/* Returns -1 on failure. */
int FCELIB_TYPES_GetInternalPartIdxByOrder(const FcelibMesh *mesh, const int order)
{
  int pid = -1;
  int count;

  for (;;)
  {
    if ((order < 0) || (order >= mesh->parts_len))
    {
      fprintf(stderr, "GetInternalPartIdxByOrder: part %d not found (len=%d)\n", order, mesh->parts_len);
      break;
    }

    for (pid = 0, count = -1; pid < mesh->parts_len; ++pid)
    {
      if (mesh->hdr.Parts[pid] > -1)
        ++count;
      if (count == order)
        break;
    }

    if (pid == mesh->parts_len)
    {
      fprintf(stderr, "GetInternalPartIdxByOrder: part %d not found\n", order);
      pid = -1;
      break;
    }

    break;
  }

  return pid;
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

  ptr = realloc(mesh->hdr.Parts, (size_t)new_len * sizeof(*mesh->hdr.Parts));
  if (!ptr)
  {
    fprintf(stderr, "FCELIB_TYPES_AddParts: Cannot reallocate memory (hdr.Parts)\n");
    return 0;
  }
  mesh->hdr.Parts = (int *)ptr;
  ptr = NULL;
  memset(mesh->hdr.Parts + mesh->parts_len, -1, (size_t)(new_len - mesh->parts_len) * sizeof(*mesh->hdr.Parts));

  ptr = realloc(mesh->parts, (size_t)new_len * sizeof(*mesh->parts));
  if (!ptr)
  {
    fprintf(stderr, "FCELIB_TYPES_AddParts: Cannot reallocate memory (parts)\n");
    return 0;
  }
  mesh->parts = (FcelibPart **)ptr;
  ptr = NULL;
  memset(mesh->parts + mesh->parts_len, 0, (size_t)(new_len - mesh->parts_len) * sizeof(*mesh->parts));

  mesh->parts_len = new_len;
  return 1;
}

/* mesh->hdr.NumTriangles is not changed */
int FCELIB_TYPES_AddTrianglesToMesh(FcelibMesh *mesh, const int num_required)
{
  void *ptr = NULL;

  ptr = realloc(mesh->triangles, (size_t)(mesh->triangles_len + num_required) * sizeof(*mesh->triangles));
  if (!ptr)
  {
    fprintf(stderr, "FCELIB_TYPES_AddTriangles: Cannot reallocate memory\n");
    return 0;
  }
  mesh->triangles = (FcelibTriangle **)ptr;
  ptr = NULL;
  memset(mesh->triangles + mesh->triangles_len, 0, (size_t)num_required * sizeof(*mesh->triangles));

  mesh->triangles_len += num_required;
  return 1;
}

/* mesh->hdr.NumVertices is not changed */
int FCELIB_TYPES_AddVerticesToMesh(FcelibMesh *mesh, const int num_required)
{
  void *ptr = NULL;

  ptr = realloc(mesh->vertices, (size_t)(mesh->vertices_len + num_required) * sizeof(*mesh->vertices));
  if (!ptr)
  {
    fprintf(stderr, "FCELIB_TYPES_AddVertices: Cannot reallocate memory\n");
    return 0;
  }
  mesh->vertices = (FcelibVertex **)ptr;
  ptr = NULL;
  memset(mesh->vertices + mesh->vertices_len, 0, (size_t)num_required * sizeof(*mesh->vertices));

  mesh->vertices_len += num_required;
  return 1;
}

int FCELIB_TYPES_AddTrianglesToPart(FcelibPart *part, const int num_required)
{
  void *ptr = NULL;

  part->ptriangles_len += num_required;
  ptr = realloc(part->PTriangles, (size_t)part->ptriangles_len * sizeof(*part->PTriangles));
  if (!ptr)
  {
    fprintf(stderr, "AddTriangles2: Cannot reallocate memory (part->PTriangles)\n");
    return 0;
  }
  part->PTriangles = (int *)ptr;
  ptr = NULL;
  memset(part->PTriangles, -1, (size_t)part->ptriangles_len * sizeof(*part->PTriangles));

  return 1;
}

int FCELIB_TYPES_AddVerticesToPart(FcelibPart *part, const int num_required)
{
  void *ptr = NULL;

  part->pvertices_len += num_required;
  ptr = realloc(part->PVertices, (size_t)part->pvertices_len * sizeof(*part->PVertices));
  if (!ptr)
  {
    fprintf(stderr, "AddVertices2: Cannot reallocate memory (part->PVertices)\n");
    return 0;
  }
  part->PVertices = (int *)ptr;
  ptr = NULL;
  memset(part->PVertices, -1, (size_t)part->pvertices_len * sizeof(*part->PVertices));

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

void FCELIB_TYPES_VertAddPosition(FcelibVertex *vert, const tVector *pos)
{
  vert->VertPos.x += pos->x;
  vert->VertPos.y += pos->y;
  vert->VertPos.z += pos->z;
  vert->NormPos.x += pos->x;
  vert->NormPos.y += pos->y;
  vert->NormPos.z += pos->z;
  vert->DamgdVertPos.x += pos->x;
  vert->DamgdVertPos.y += pos->y;
  vert->DamgdVertPos.z += pos->z;
  vert->DamgdNormPos.x += pos->x;
  vert->DamgdNormPos.y += pos->y;
  vert->DamgdNormPos.z += pos->z;
}

int FCELIB_TYPES_GetPartLocalCentroid(FcelibMesh *mesh, FcelibPart *part, tVector *centroid)
{
  int retv = 0;
  int i;
  float* x_arr;
  float* y_arr;
  float* z_arr;
  float* xyz_arr = NULL;
  const int PNumVertices = part->PNumVertices;
  FcelibVertex *vert;
  int count_verts = 0;

  for (;;)
  {
    if (PNumVertices == 0)
    {
      centroid->x = 0.0f;
      centroid->y = 0.0f;
      centroid->z = 0.0f;
      retv = 1;
      break;
    }

    xyz_arr = (float *)malloc((size_t)(3 * (PNumVertices + 1)) * sizeof(*xyz_arr));
    if (!xyz_arr)
    {
      fprintf(stderr, "GetPartLocalCentroid: Cannot allocate memory\n");
      break;
    }
    memset(xyz_arr, 0, (size_t)(3 * (PNumVertices + 1)) * sizeof(*xyz_arr));

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

      ++count_verts;
    }

    qsort(x_arr, (size_t)count_verts, sizeof(*x_arr), FCELIB_MISC_CompareFloats);
    qsort(y_arr, (size_t)count_verts, sizeof(*y_arr), FCELIB_MISC_CompareFloats);
    qsort(z_arr, (size_t)count_verts, sizeof(*z_arr), FCELIB_MISC_CompareFloats);

    centroid->x = 0.5f * FCELIB_MISC_Abs(x_arr[count_verts - 1] - x_arr[0]) + x_arr[0];
    centroid->y = 0.5f * FCELIB_MISC_Abs(y_arr[count_verts - 1] - y_arr[0]) + y_arr[0];
    centroid->z = 0.5f * FCELIB_MISC_Abs(z_arr[count_verts - 1] - z_arr[0]) + z_arr[0];

    retv = 1;
    break;
  }

  if (xyz_arr)
  {
    free(xyz_arr);
    x_arr = NULL;
    y_arr = NULL;
    z_arr = NULL;
  }

  return retv;
}

/* Does not move part w.r.t. to global coordinates */
void FCELIB_TYPES_ResetPartCenter(FcelibMesh *mesh, FcelibPart *part, const tVector new_PartPos)
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
    vert->NormPos.x += part->PartPos.x - new_PartPos.x;
    vert->NormPos.y += part->PartPos.y - new_PartPos.y;
    vert->NormPos.z += part->PartPos.z - new_PartPos.z;
    vert->DamgdVertPos.x += part->PartPos.x - new_PartPos.x;
    vert->DamgdVertPos.y += part->PartPos.y - new_PartPos.y;
    vert->DamgdVertPos.z += part->PartPos.z - new_PartPos.z;
    vert->DamgdNormPos.x += part->PartPos.x - new_PartPos.x;
    vert->DamgdNormPos.y += part->PartPos.y - new_PartPos.y;
    vert->DamgdNormPos.z += part->PartPos.z - new_PartPos.z;
    ++count_verts;
  }
  memcpy(&part->PartPos.x, &new_PartPos.x, sizeof(part->PartPos.x));
  memcpy(&part->PartPos.y, &new_PartPos.y, sizeof(part->PartPos.y));
  memcpy(&part->PartPos.z, &new_PartPos.z, sizeof(part->PartPos.z));
}


/* stats -------------------------------------------------------------------- */

void FCELIB_TYPES_PrintMeshInfo(const FcelibMesh mesh)
{
  int i;
  int j;
  int verts = 0;
  int triags = 0;

  if (!FCELIB_TYPES_ValidateMesh(mesh))
  {
    fprintf(stderr, "PrintMeshInfo: invalid mesh\n");
    return;
  }

  printf("NumTriangles (true) = %d\n", mesh.hdr.NumTriangles);
  printf("triangles_len (alloc'd) = %d\n", mesh.triangles_len);

  printf("NumVertices (true) = %d\n", mesh.hdr.NumVertices);
  printf("vertices_len (alloc'd) = %d\n", mesh.vertices_len);

  printf("NumParts (true) = %d\n", mesh.hdr.NumParts);
  printf("parts_len (alloc'd) = %d\n", mesh.parts_len);

  printf("NumArts = %d\n", mesh.hdr.NumArts);
  printf("NumDummies = %d\n", mesh.hdr.NumDummies);
  printf("NumColors = %d\n", mesh.hdr.NumColors);
  printf("NumSecColors = %d\n", mesh.hdr.NumSecColors);

  printf("Parts:\n"
         "Ord Idx   Verts  Triangles  (PartPos)                          FCE3 role            Name\n");
  for (i = 0, j = 0; i < mesh.parts_len; ++i)
  {
    if (mesh.hdr.Parts[i] < 0)
      continue;

    printf(" %2d  %2d   %5d      %5d  (%9f, %9f, %9f)  %20s %s\n",
           j,
           mesh.hdr.Parts[i],
           mesh.parts[mesh.hdr.Parts[i]]->PNumVertices,
           mesh.parts[mesh.hdr.Parts[i]]->PNumTriangles,
           mesh.parts[mesh.hdr.Parts[i]]->PartPos.x, mesh.parts[mesh.hdr.Parts[i]]->PartPos.y, mesh.parts[mesh.hdr.Parts[i]]->PartPos.z,
           j < kFceLibImplementedFce3Parts ? kFce3PartsNames[j] : "",
           mesh.parts[mesh.hdr.Parts[i]]->PartName);

    verts  += mesh.parts[mesh.hdr.Parts[i]]->PNumVertices;
    triags += mesh.parts[mesh.hdr.Parts[i]]->PNumTriangles;

    ++j;
  }
  printf("    = %5d    = %5d\n",
         verts, triags);

  printf("DummyNames (Position):\n");
  for (i = 0; i < mesh.hdr.NumDummies; ++i)
  {
    printf(" %2d  (%9f, %9f, %9f) %.64s\n", i,
           mesh.hdr.Dummies[i].x, mesh.hdr.Dummies[i].y, mesh.hdr.Dummies[i].z,
           mesh.hdr.DummyNames + (i * 64));
  }

  printf("Car colors (hue, saturation, brightness, transparency):\n");
  for (i = 0; i < mesh.hdr.NumColors; ++i)
  {
    printf(" %2d  Primary     %3d, %3d, %3d, %3d\n", i,
          mesh.hdr.PriColors[i].hue, mesh.hdr.PriColors[i].saturation,
          mesh.hdr.PriColors[i].brightness, mesh.hdr.PriColors[i].transparency);
    printf(" %2d  Interior    %3d, %3d, %3d, %3d\n", i,
          mesh.hdr.IntColors[i].hue, mesh.hdr.IntColors[i].saturation,
          mesh.hdr.IntColors[i].brightness, mesh.hdr.IntColors[i].transparency);
    printf(" %2d  Secondary   %3d, %3d, %3d, %3d\n", i,
          mesh.hdr.SecColors[i].hue, mesh.hdr.SecColors[i].saturation,
          mesh.hdr.SecColors[i].brightness, mesh.hdr.SecColors[i].transparency);
    printf(" %2d  Driver hair %3d, %3d, %3d, %3d\n", i,
          mesh.hdr.DriColors[i].hue, mesh.hdr.DriColors[i].saturation,
          mesh.hdr.DriColors[i].brightness, mesh.hdr.DriColors[i].transparency);
  }
}

/* Prints ref'ed global part indexes. */
void FCELIB_TYPES_PrintMeshParts(const FcelibMesh mesh)
{
  int j;

  if (!FCELIB_TYPES_ValidateMesh(mesh))
  {
    fprintf(stderr, "PrintMeshParts: Cannot print\n");
    return;
  }

  printf("NumParts = %d, parts_len = %d, [\n",
         mesh.hdr.NumParts, mesh.parts_len);

  for (j = 0; j < mesh.parts_len; ++j)
    printf("%d, ", mesh.hdr.Parts[j]);

  printf("\n]\n");
}

/* Prints ref'ed global triag indexes for each part. */
void FCELIB_TYPES_PrintMeshTriangles(const FcelibMesh mesh)
{
  int i;
  int j;

  if (!FCELIB_TYPES_ValidateMesh(mesh))
  {
    fprintf(stderr, "PrintMeshTriangles: Cannot print\n");
    return;
  }

  for (i = 0; i < mesh.parts_len; ++i)
  {
    if (mesh.hdr.Parts[i] < 0)
      continue;

    printf("Part %d '%s', PNumTriangles = %d, ptriangles_len = %d, [\n",
           i, mesh.parts[mesh.hdr.Parts[i]]->PartName,
           mesh.parts[mesh.hdr.Parts[i]]->PNumTriangles, mesh.parts[mesh.hdr.Parts[i]]->ptriangles_len);

    for (j = 0; j < mesh.parts[mesh.hdr.Parts[i]]->ptriangles_len; ++j)
      printf("%d, ", mesh.parts[mesh.hdr.Parts[i]]->PTriangles[j]);

    printf("\n]\n");
  }
}

/* Prints ref'ed global vert indexes for each part. */
void FCELIB_TYPES_PrintMeshVertices(const FcelibMesh mesh)
{
  int i;
  int j;

  if (!FCELIB_TYPES_ValidateMesh(mesh))
  {
    fprintf(stderr, "PrintMeshVertices: Cannot print\n");
    return;
  }

  for (i = 0; i < mesh.parts_len; ++i)
  {
    if (mesh.hdr.Parts[i] < 0)
      continue;

    printf("Part %d '%s', PNumVertices = %d, pvertices_len = %d, [\n",
           i, mesh.parts[mesh.hdr.Parts[i]]->PartName,
           mesh.parts[mesh.hdr.Parts[i]]->PNumVertices, mesh.parts[mesh.hdr.Parts[i]]->pvertices_len);

    for (j = 0; j < mesh.parts[mesh.hdr.Parts[i]]->pvertices_len; ++j)
      printf("%d, ", mesh.parts[mesh.hdr.Parts[i]]->PVertices[j]);

    printf("\n]\n");
  }
}

#ifdef __cplusplus
}  /* extern "C" */
#endif

#ifdef __cplusplus
}  /* namespace fcelib */
#endif

#endif  /* FCELIB_TYPES_H */