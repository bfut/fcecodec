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

typedef struct {
  int      NumTriangles;
  int      NumVertices;

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

typedef struct {
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

/* init, destroy, validate */

void FCELIB_TYPES_InitMesh(FcelibMesh *mesh)
{
  int i;
  
  if (mesh->freed != 1)
    fprintf(stderr, "Warning: InitMesh: mesh is not free'd (requires FCELIB_FreeMesh)\n");

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
  memset(mesh->hdr.DummyNames, '\0', (size_t)sizeof(mesh->hdr.DummyNames));

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
  
  if (mesh->freed == 1)
    return;
  
  if (mesh->vertices_len > 0)
  {
    for (i = mesh->vertices_len - 1; i >= 0; --i)
    {
      if (mesh->vertices[i])
        free(mesh->vertices[i]);
    }
    if (mesh->vertices)
    {
      free(mesh->vertices);
      mesh->vertices = NULL;
    }
  }
  if (mesh->triangles_len > 0)
  {
    for (i = mesh->triangles_len - 1; i >= 0; --i)
    {
      if (mesh->triangles[i])
        free(mesh->triangles[i]);
    }
    if (mesh->triangles)
    {
      free(mesh->triangles);
      mesh->triangles = NULL;
    }
  }
  if (mesh->parts_len > 0)
  {
    for (i = mesh->parts_len - 1; i >= 0; --i)
    {
      if (mesh->parts[i])
      {
        if (mesh->parts[i]->PTriangles)
          free(mesh->parts[i]->PTriangles);
        if (mesh->parts[i]->PVertices)
          free(mesh->parts[i]->PVertices);
        free(mesh->parts[i]);
      }
    }
    if (mesh->parts)
    {
      free(mesh->parts);
      mesh->parts = NULL;
    }
  }

  mesh->hdr.NumTriangles = 0;
  mesh->hdr.NumVertices = 0;

  mesh->hdr.NumParts = 0;
  if (mesh->hdr.Parts)
    free(mesh->hdr.Parts);

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

  mesh->parts_len = 0;
  mesh->triangles_len = 0;
  mesh->vertices_len = 0;
  
  mesh->freed = 1;
}


/* Returns: 1 = valid mesh, -1 = empty valid mesh, 0 = invalid mesh */
int FCELIB_TYPES_ValidateMesh(FcelibMesh mesh)
{
  int i;
  int j;
  int count_parts;

  if ((!mesh.parts) || (!mesh.triangles) || (!mesh.vertices))
    return -1;

  for (i = 0, count_parts = 0; i < mesh.parts_len; ++i)
  {
    if (mesh.hdr.Parts[i] >= mesh.parts_len)
    {
      fprintf(stderr, "inconsistent list (mesh.hdr.Parts[i])\n");
      return 0;
    }

    if (mesh.hdr.Parts[i] < 0)
      continue;

    ++count_parts;

    if (!mesh.parts[mesh.hdr.Parts[i]])
    {
      fprintf(stderr, "unexpected NULL pointer (mesh.parts[i]) %d %d\n", i, mesh.hdr.Parts[i]);
      return 0;
    }

    if (!mesh.parts[mesh.hdr.Parts[i]]->PTriangles)
    {
      fprintf(stderr, "unexpected NULL pointer (mesh.parts[i]->PTriangles)\n");
      return 0;
    }
    for (j = 0; j < mesh.parts[mesh.hdr.Parts[i]]->ptriangles_len; ++j)
    {
      if ((mesh.parts[mesh.hdr.Parts[i]]->PTriangles[j] != -1) &&
          (!mesh.triangles[mesh.parts[mesh.hdr.Parts[i]]->PTriangles[j]]))
      {
        fprintf(stderr, "unexpected NULL pointer (mesh.triangles[mesh.parts[mesh.hdr.Parts[i]]->PTriangles[j]])\n");
        return 0;
      }
    }

    if (!mesh.parts[mesh.hdr.Parts[i]]->PNumVertices)
    {
      fprintf(stderr, "unexpected NULL pointer (mesh.parts[i]->PNumVertices)\n");
      return 0;
    }
    for (j = 0; j < mesh.parts[mesh.hdr.Parts[i]]->pvertices_len; ++j)
    {
      if ((mesh.parts[mesh.hdr.Parts[i]]->PVertices[j] == -1) &&
          (!mesh.vertices[mesh.parts[mesh.hdr.Parts[i]]->PVertices[j]]))
      {
        fprintf(stderr, "unexpected NULL pointer (mesh.vertices[mesh.parts[mesh.hdr.Parts[i]]->PVertices[j]])\n");
        return 0;
      }
    }
  }  /* for i */

  if (count_parts != mesh.hdr.NumParts)
  {
    fprintf(stderr, "inconsistent list (mesh.hdr.NumParts)\n");
    return 0;
  }

  return 1;
}


/* service ------------------------------------------------------------------ */

/* Returns -1 on failure. */
int FCELIB_TYPES_GetInternalPartIdxByOrder(FcelibMesh *mesh, const int idx)
{
  int i = 0;

  for (;;)
  {
    if ((idx < 0) || (idx >= mesh->parts_len))
    {
      fprintf(stderr, "GetInternalPartIdxByOrder: part %d not found (len=%d)\n", idx, mesh->parts_len);
      i = -1;
      break;
    }

    for (int count = -1; i < mesh->parts_len; ++i)
    {
      if (mesh->hdr.Parts[i] > -1)
        ++count;
      if (count == idx)
        break;
    }

    if (i == mesh->parts_len)
    {
      fprintf(stderr, "GetInternalPartIdxByOrder: part %d not found\n", idx);
      i = -1;
      break;
    }

    break;
  }

  return i;
}

/* Returns -1 on failure. */
int FCELIB_TYPES_GetOrderByInternalPartIdx(FcelibMesh *mesh, const int idx)
{
  int order = -1;
  int i = 0;

  for (;;)
  {
    if ((idx < 0) || (idx >= mesh->parts_len))
    {
      fprintf(stderr, "GetOrderByInternalPartIdx: part %d not found (len=%d)\n", idx, mesh->parts_len);
      break;
    }

    for (order = -1; i < mesh->parts_len; ++i)
    {
      if (mesh->hdr.Parts[i] > -1)
        ++order;
      if (mesh->hdr.Parts[i] == idx)
        break;
    }

    if (i == mesh->parts_len)
    {
      fprintf(stderr, "GetOrderByInternalPartIdx: part %d not found\n", idx);
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
  int new_len = 2 * mesh->parts_len;

  if (new_len < mesh->parts_len + num_required)
    new_len = 2 * (mesh->parts_len + num_required);

  ptr = realloc(mesh->hdr.Parts, (size_t)new_len * sizeof(int));
  if (!ptr)
  {
    fprintf(stderr, "FCELIB_TYPES_AddParts: Cannot reallocate memory (hdr.Parts)\n");
    return 0;
  }
  mesh->hdr.Parts = (int *)ptr;
  ptr = NULL;
  memset(mesh->hdr.Parts + mesh->parts_len, -1, (size_t)(new_len - mesh->parts_len) * sizeof(int));

  ptr = realloc(mesh->parts, (size_t)new_len * sizeof(FcelibPart *));
  if (!ptr)
  {
    fprintf(stderr, "FCELIB_TYPES_AddParts: Cannot reallocate memory (parts)\n");
    return 0;
  }
  mesh->parts = (FcelibPart **)ptr;
  memset(mesh->parts + mesh->parts_len, 0, (size_t)(new_len - mesh->parts_len) * sizeof(FcelibPart *));

  mesh->parts_len = new_len;
  return 1;
}


int FCELIB_TYPES_AddTriangles(FcelibMesh *mesh, const int num_required)
{
  void *ptr = NULL;
  int new_len = 2 * mesh->triangles_len;

  if (new_len < mesh->triangles_len + num_required)
    new_len = 2 * (mesh->triangles_len + num_required);

  ptr = realloc(mesh->triangles, (size_t)new_len * sizeof(FcelibTriangle *));
  if (!ptr)
  {
    fprintf(stderr, "Cannot reallocate memory\n");
    return 0;
  }
  mesh->triangles = (FcelibTriangle **)ptr;
  memset(mesh->triangles + mesh->triangles_len, 0, (size_t)(new_len - mesh->triangles_len) * sizeof(FcelibTriangle *));

  mesh->triangles_len = new_len;
  return 1;
}


int FCELIB_TYPES_AddVertices(FcelibMesh *mesh, const int num_required)
{
  void *ptr = NULL;
  int new_len = 2 * mesh->vertices_len;

  if (new_len < mesh->vertices_len + num_required)
    new_len = 2 * (mesh->vertices_len + num_required);

  ptr = realloc(mesh->vertices, (size_t)new_len * sizeof(FcelibVertex *));
  if (!ptr)
  {
    fprintf(stderr, "Cannot reallocate memory\n");
    return 0;
  }
  mesh->vertices = (FcelibVertex **)ptr;
  memset(mesh->vertices + mesh->vertices_len, 0, (size_t)(new_len - mesh->vertices_len) * sizeof(FcelibVertex *));

  mesh->vertices_len = new_len;
  return 1;
}


int FCELIB_TYPES_AddTriangles2(FcelibMesh *mesh, FcelibPart *part, const int num_required)
{
  void *ptr = NULL;
  const int new_len_p = part->ptriangles_len + num_required;
  const int new_len = mesh->triangles_len + num_required;

  ptr = realloc(part->PTriangles, (size_t)new_len_p * sizeof(int));
  if (!ptr)
  {
    fprintf(stderr, "AddTriangles2: Cannot reallocate memory (part->PTriangles)\n");
    return 0;
  }
  part->PTriangles = (int *)ptr;
  ptr = NULL;
  memset(part->PTriangles, -1, (size_t)new_len_p * sizeof(int));
  
  ptr = realloc(mesh->triangles, (size_t)new_len * sizeof(FcelibTriangle *));
  if (!ptr)
  {
    fprintf(stderr, "AddTriangles2: Cannot reallocate memory (triangles)\n");
    return 0;
  }
  mesh->triangles = (FcelibTriangle **)ptr;
  memset(mesh->triangles + mesh->triangles_len, 0, (size_t)(new_len - mesh->triangles_len) * sizeof(FcelibTriangle *));

  part->ptriangles_len = new_len_p;
  mesh->triangles_len = new_len;
  return 1;
}


int FCELIB_TYPES_AddVertices2(FcelibMesh *mesh, FcelibPart *part, const int num_required)
{
  void *ptr = NULL;
  const int new_len_p = part->pvertices_len + num_required;
  const int new_len = mesh->vertices_len + num_required;
    
  ptr = realloc(part->PVertices, (size_t)new_len_p * sizeof(int));
  if (!ptr)
  {
    fprintf(stderr, "AddVertices2: Cannot reallocate memory (part->PVertices)\n");
    return 0;
  }
  part->PVertices = (int *)ptr;
  ptr = NULL;
  memset(part->PVertices, -1, (size_t)new_len_p * sizeof(int));

  ptr = realloc(mesh->vertices, (size_t)new_len * sizeof(FcelibVertex *));
  if (!ptr)
  {
    fprintf(stderr, "AddVertices2: Cannot reallocate memory (vertices)\n");
    return 0;
  }
  mesh->vertices = (FcelibVertex **)ptr;
  memset(mesh->vertices + mesh->vertices_len, 0, (size_t)(new_len - mesh->vertices_len) * sizeof(FcelibVertex *));

  part->pvertices_len = new_len_p;
  mesh->vertices_len = new_len;
  return 1;
}


void FCELIB_TYPES_CpyTriag(FcelibTriangle *dest, FcelibTriangle *src)
{
  dest->tex_page = src->tex_page;
  memcpy(dest->vidx, src->vidx, (size_t)3 * (size_t)sizeof(int));
  dest->flag = src->flag;
  memcpy(dest->U, src->U, (size_t)3 * (size_t)sizeof(float));
  memcpy(dest->V, src->V, (size_t)3 * (size_t)sizeof(float));
}

void FCELIB_TYPES_CpyVert(FcelibVertex *dest, FcelibVertex *src)
{
  memcpy(&dest->VertPos.x, &src->VertPos.x, (size_t)sizeof(float));
  memcpy(&dest->VertPos.y, &src->VertPos.y, (size_t)sizeof(float));
  memcpy(&dest->VertPos.z, &src->VertPos.z, (size_t)sizeof(float));
  memcpy(&dest->NormPos.x, &src->NormPos.x, (size_t)sizeof(float));
  memcpy(&dest->NormPos.y, &src->NormPos.y, (size_t)sizeof(float));
  memcpy(&dest->NormPos.z, &src->NormPos.z, (size_t)sizeof(float));
  memcpy(&dest->DamgdVertPos.x, &src->DamgdVertPos.x, (size_t)sizeof(float));
  memcpy(&dest->DamgdVertPos.y, &src->DamgdVertPos.y, (size_t)sizeof(float));
  memcpy(&dest->DamgdVertPos.z, &src->DamgdVertPos.z, (size_t)sizeof(float));
  memcpy(&dest->DamgdNormPos.x, &src->DamgdNormPos.x, (size_t)sizeof(float));
  memcpy(&dest->DamgdNormPos.y, &src->DamgdNormPos.y, (size_t)sizeof(float));
  memcpy(&dest->DamgdNormPos.z, &src->DamgdNormPos.z, (size_t)sizeof(float));
  dest->Animation = src->Animation;
}

int FCELIB_TYPES_GetPartLocalCentroid(FcelibMesh *mesh, FcelibPart *part, tVector *centroid)
{
  int retv = 0;
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
      fprintf(stderr, "GetPartCentroid: Cannot allocate memory\n");
      break;
    }
    memset(xyz_arr, 0, (size_t)(3 * (PNumVertices + 1)) * sizeof(*xyz_arr));

    x_arr = xyz_arr;
    y_arr = xyz_arr + PNumVertices;
    z_arr = xyz_arr + PNumVertices * 2;

    // i - internal vert index, count_verts - vert order
    for (int i = 0; i < part->pvertices_len && count_verts < PNumVertices; ++i)
    {
      if (part->PVertices[i] < 0)
        continue;

      vert = mesh->vertices[ part->PVertices[i] ];
      x_arr[i] = vert->VertPos.x + part->PartPos.x;
      y_arr[i] = vert->VertPos.y + part->PartPos.y;
      z_arr[i] = vert->VertPos.z + part->PartPos.z;

      ++count_verts;
    }

    qsort(x_arr, (size_t)count_verts, sizeof(*x_arr), FCELIB_MISC_CompareFloats);
    qsort(y_arr, (size_t)count_verts, sizeof(*y_arr), FCELIB_MISC_CompareFloats);
    qsort(z_arr, (size_t)count_verts, sizeof(*z_arr), FCELIB_MISC_CompareFloats);

    centroid->x = 0.5f * abs(x_arr[count_verts - 1] - x_arr[0]) + x_arr[0];
    centroid->y = 0.5f * abs(y_arr[count_verts - 1] - y_arr[0]) + y_arr[0];
    centroid->z = 0.5f * abs(z_arr[count_verts - 1] - z_arr[0]) + z_arr[0];

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
void FCELIB_TYPES_ResetPartPos(FcelibMesh *mesh, FcelibPart *part, tVector new_PartPos)
{
  FcelibVertex *vert;
  int count_verts = 0;
  for (int i = 0; i < part->pvertices_len && count_verts < part->PNumVertices; ++i)
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
  memcpy(&part->PartPos.x, &new_PartPos.x, (size_t)sizeof(float));
  memcpy(&part->PartPos.y, &new_PartPos.y, (size_t)sizeof(float));
  memcpy(&part->PartPos.z, &new_PartPos.z, (size_t)sizeof(float));
}

#if 0
void FCELIB_MISC_LocalizeVerts(tVector *vert, int NumVertices, tVector PartPos)
{
  int i;

  for (i = 0; i < NumVertices; ++i)
  {
    vert[i].x -= PartPos.x;
    vert[i].y -= PartPos.y;
    vert[i].z -= PartPos.z;
  }
}
#endif


void FCELIB_TYPES_PrintMeshInfo(FcelibMesh mesh)
{
  int i;
  int j;
  int verts = 0;
  int triags = 0;

  if (!FCELIB_TYPES_ValidateMesh(mesh))
    return;

  printf("NumTriangles (true) = %d\n", mesh.hdr.NumTriangles);
  printf("triangles_len (alloc'd) = %d\n", mesh.triangles_len);

  printf("NumVertices (true) = %d\n", mesh.hdr.NumVertices);
  printf("vertices_len (alloc'd) = %d\n", mesh.vertices_len);

  printf("NumParts (true) = %d\n", mesh.hdr.NumParts);
  printf("parts_len (alloc'd) = %d\n", mesh.parts_len);

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
void FCELIB_TYPES_PrintMeshParts(FcelibMesh mesh)
{
  int j;

  if (!FCELIB_TYPES_ValidateMesh(mesh))
    return;

  printf("NumParts = %d, parts_len = %d, [\n",
         mesh.hdr.NumParts, mesh.parts_len);

  for (j = 0; j < mesh.parts_len; ++j)
    printf("%d, ", mesh.hdr.Parts[j]);

  printf("\n]\n");
}

/* Prints ref'ed global triag indexes for each part. */
void FCELIB_TYPES_PrintMeshTriangles(FcelibMesh mesh)
{
  int i;
  int j;

  if (!FCELIB_TYPES_ValidateMesh(mesh))
    return;

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
void FCELIB_TYPES_PrintMeshVertices(FcelibMesh mesh)
{
  int i;
  int j;

  if (!FCELIB_TYPES_ValidateMesh(mesh))
    return;

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
