/*
  fcelib_io.h
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
  import/export FCE3, FCE4, FCE4M
 **/

#ifndef FCELIB_IO_H
#define FCELIB_IO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
//#include "LFB/LFB_io.h"
#endif

#include "fcelib_fcetypes.h"
#include "fcelib_misc.h"
#include "fcelib_types.h"

#ifdef __cplusplus
namespace fcelib {
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* decode formats ----------------------------------------------------------- */

/* Params: FCE buffer, uninitialized FcelibMesh. Returns bool.
   Assumes valid FCE data. */
int FCELIB_IO_DecodeFce(const void *inbuf, int buf_size, FcelibMesh *mesh)
{
  int retv = 0;
  int i;
  int j;
  size_t n;
  const unsigned char *buf = (const unsigned char *)inbuf;
  int fce_version;

  if (!mesh)
  {
    fprintf(stderr, "DecodeFce: mesh is NULL\n");
    return 0;
  }
  
  if (mesh->freed != 1)
  {
    fprintf(stderr, "DecodeFce: mesh is not free'd or initialized)\n");
    return 0;
  }
  mesh->freed = 0;

  for (;;)
  {
    if (buf_size < 0x1F04)
    {
      fprintf(stderr, "DecodeFce: Format error: header too small\n");
      break;
    }

    memcpy(&fce_version, buf, (size_t)4);

    switch (fce_version)
    {
      case 0x00101014: case 0x00101015:
      {
        FceHeader4 header;
        const int kHdrSize = 0x2038;

        if (buf_size < 0x2038)
        {
          fprintf(stderr, "DecodeFce: Format error: header too small\n");
          break;
        }

        if (FCELIB_FCETYPES_Fce4ValidateHeader(buf, buf_size) != 1)
          return 0;

        header = FCELIB_FCETYPES_GetFceHeader4(buf);


        /* Header ----------------------------------------------------------- */
          /* NumTriangles - counted below */
          /* NumVertices - counted below */
        mesh->hdr.NumParts = header.NumParts;
        mesh->parts_len = 2 * mesh->hdr.NumParts;

        mesh->hdr.Parts = (int *)malloc((size_t)mesh->parts_len * sizeof(int));
        memset(mesh->hdr.Parts, -1, (size_t)mesh->parts_len * sizeof(int));
        for (i = 0; i < header.NumParts; ++i)
          mesh->hdr.Parts[i] = i;

        mesh->hdr.NumDummies = header.NumDummies;
        for (i = 0; i < mesh->hdr.NumDummies; ++i)
        {
          memcpy(&mesh->hdr.Dummies[i].x, buf + 0x005c + i * 12 + 0x0, (size_t)4);
          memcpy(&mesh->hdr.Dummies[i].y, buf + 0x005c + i * 12 + 0x4, (size_t)4);
          memcpy(&mesh->hdr.Dummies[i].z, buf + 0x005c + i * 12 + 0x8, (size_t)4);
          mesh->hdr.DummyNames[(i + 1) * 64 - 1] = '\0';  /* ensure string */
        }
        memcpy(&mesh->hdr.DummyNames, buf + 0x0a28, (size_t)(16 * 64));         /** TODO: ensure string, cleaning up! */

        mesh->hdr.NumColors = header.NumColors;
        mesh->hdr.NumSecColors = mesh->hdr.NumColors;
        for (i = 0; i < mesh->hdr.NumColors; ++i)
        {
          memcpy(&mesh->hdr.PriColors[i].hue,          buf + 0x0824 + i * 4 + 0x0, (size_t)1);
          memcpy(&mesh->hdr.PriColors[i].saturation,   buf + 0x0824 + i * 4 + 0x1, (size_t)1);
          memcpy(&mesh->hdr.PriColors[i].brightness,   buf + 0x0824 + i * 4 + 0x2, (size_t)1);
          memcpy(&mesh->hdr.PriColors[i].transparency, buf + 0x0824 + i * 4 + 0x3, (size_t)1);

          memcpy(&mesh->hdr.IntColors[i].hue,          buf + 0x0864 + i * 4 + 0x0, (size_t)1);
          memcpy(&mesh->hdr.IntColors[i].saturation,   buf + 0x0864 + i * 4 + 0x1, (size_t)1);
          memcpy(&mesh->hdr.IntColors[i].brightness,   buf + 0x0864 + i * 4 + 0x2, (size_t)1);
          memcpy(&mesh->hdr.IntColors[i].transparency, buf + 0x0864 + i * 4 + 0x3, (size_t)1);

          memcpy(&mesh->hdr.SecColors[i].hue,          buf + 0x08a4 + i * 4 + 0x0, (size_t)1);
          memcpy(&mesh->hdr.SecColors[i].saturation,   buf + 0x08a4 + i * 4 + 0x1, (size_t)1);
          memcpy(&mesh->hdr.SecColors[i].brightness,   buf + 0x08a4 + i * 4 + 0x2, (size_t)1);
          memcpy(&mesh->hdr.SecColors[i].transparency, buf + 0x08a4 + i * 4 + 0x3, (size_t)1);

          memcpy(&mesh->hdr.DriColors[i].hue,          buf + 0x08e4 + i * 4 + 0x0, (size_t)1);
          memcpy(&mesh->hdr.DriColors[i].saturation,   buf + 0x08e4 + i * 4 + 0x1, (size_t)1);
          memcpy(&mesh->hdr.DriColors[i].brightness,   buf + 0x08e4 + i * 4 + 0x2, (size_t)1);
          memcpy(&mesh->hdr.DriColors[i].transparency, buf + 0x08e4 + i * 4 + 0x3, (size_t)1);
        }


        /* Parts ------------------------------------------------------------ */
        if (mesh->hdr.NumParts == 0)  /** TODO: test */
        {
          retv = 1;
          break;
        }

        mesh->parts = (FcelibPart **)malloc((size_t)mesh->parts_len * sizeof(FcelibPart *));
        if (!mesh->parts)
        {
          fprintf(stderr, "DecodeFce: Cannot allocate memory\n");
          break;
        }
        memset(mesh->parts, 0, (size_t)mesh->parts_len * sizeof(FcelibPart *));

        for (i = 0; i < mesh->hdr.NumParts; ++i)
        {
          mesh->parts[i] = (FcelibPart *)malloc(sizeof(FcelibPart));
          if (!mesh->parts[i])
          {
            fprintf(stderr, "DecodeFce: Cannot allocate memory\n");
            break;
          }

          memcpy(mesh->parts[i]->PartName, header.PartNames + i * 64, (size_t)64);
          mesh->parts[i]->PartName[63] = '\0';  /* ensure string */

          mesh->parts[i]->PartPos.x = header.PartPos[i].x;
          mesh->parts[i]->PartPos.y = header.PartPos[i].y;
          mesh->parts[i]->PartPos.z = header.PartPos[i].z;

          mesh->parts[i]->PNumVertices = header.PNumVertices[i];
          mesh->parts[i]->pvertices_len = 2 * mesh->parts[i]->PNumVertices;
          mesh->parts[i]->PVertices = NULL;

          mesh->parts[i]->PNumTriangles = header.PNumTriangles[i];
          mesh->parts[i]->ptriangles_len = 2 * mesh->parts[i]->PNumTriangles;
          mesh->parts[i]->PTriangles = NULL;

          /* update global counts */
          mesh->vertices_len += mesh->parts[i]->pvertices_len;
          mesh->triangles_len += mesh->parts[i]->ptriangles_len;

          mesh->parts[i]->PVertices = (int *)malloc((size_t)(mesh->parts[i]->pvertices_len * sizeof(int)));
          if (!mesh->parts[i]->PVertices)
          {
            fprintf(stderr, "DecodeFce: Cannot allocate memory\n");
            break;
          }
          memset(mesh->parts[i]->PVertices, -1, (size_t)(mesh->parts[i]->pvertices_len * sizeof(int)));

          mesh->parts[i]->PTriangles = (int *)malloc((size_t)(mesh->parts[i]->ptriangles_len * sizeof(int)));
          if (!mesh->parts[i]->PTriangles)
          {
            fprintf(stderr, "DecodeFce: Cannot allocate memory\n");
            break;
          }
          memset(mesh->parts[i]->PTriangles, -1, (size_t)(mesh->parts[i]->ptriangles_len) * sizeof(int));

        }  /* for i */
        for (i = mesh->hdr.NumParts; i < mesh->parts_len; ++i)
          mesh->parts[i] = NULL;


        /* Triangles -------------------------------------------------------- */
        if (mesh->triangles_len == 0)  /** TODO: test */
        {
          retv = 1;
          break;
        }

        mesh->triangles = (FcelibTriangle **)malloc((size_t)mesh->triangles_len * sizeof(FcelibTriangle *));
        if (!mesh->triangles)
        {
          fprintf(stderr, "DecodeFce: Cannot allocate memory\n");
          break;
        }
        memset(mesh->triangles, '\0', (size_t)mesh->triangles_len * sizeof(FcelibTriangle *));

        mesh->hdr.NumTriangles = 0;
        mesh->hdr.NumVertices = 0;

        for (i = 0; i < mesh->hdr.NumParts; ++i)
        {
          for (j = 0; j < mesh->parts[i]->PNumTriangles; ++j)
          {
            mesh->parts[i]->PTriangles[j] = mesh->hdr.NumTriangles;

            mesh->triangles[mesh->hdr.NumTriangles] = (FcelibTriangle *)malloc(sizeof(FcelibTriangle));
            if (!mesh->triangles[mesh->hdr.NumTriangles])
            {
              fprintf(stderr, "DecodeFce: Cannot allocate memory\n");
              break;
            }

            memcpy(&mesh->triangles[mesh->hdr.NumTriangles]->tex_page, buf + kHdrSize + header.TriaTblOffset + (j + header.P1stTriangles[i]) * 56 + 0x00, (size_t)4);
            memcpy(&mesh->triangles[mesh->hdr.NumTriangles]->vidx,     buf + kHdrSize + header.TriaTblOffset + (j + header.P1stTriangles[i]) * 56 + 0x04, (size_t)12);

            /* Globalize vert index references */
            for (n = 0; n < 3; ++n)
              mesh->triangles[mesh->hdr.NumTriangles]->vidx[n] += mesh->hdr.NumVertices;

            memcpy(&mesh->triangles[mesh->hdr.NumTriangles]->flag, buf + kHdrSize + header.TriaTblOffset + (j + header.P1stTriangles[i]) * 56 + 0x1C, (size_t)4);
            memcpy(&mesh->triangles[mesh->hdr.NumTriangles]->U,    buf + kHdrSize + header.TriaTblOffset + (j + header.P1stTriangles[i]) * 56 + 0x20, (size_t)12);
            memcpy(&mesh->triangles[mesh->hdr.NumTriangles]->V,    buf + kHdrSize + header.TriaTblOffset + (j + header.P1stTriangles[i]) * 56 + 0x2C, (size_t)12);

            if (fce_version == 0x00101014)
            {
              for (n = 0; n < 3; ++n)
                mesh->triangles[mesh->hdr.NumTriangles]->V[n] = 1 - mesh->triangles[mesh->hdr.NumTriangles]->V[n];
            }

            ++mesh->hdr.NumTriangles;
          }

          mesh->hdr.NumVertices += mesh->parts[i]->PNumVertices;
        }
        for (i = mesh->hdr.NumTriangles; i < mesh->triangles_len; ++i)
          mesh->triangles[i] = NULL;


        /* Vertices --------------------------------------------------------- */
        if (mesh->vertices_len == 0)  /** TODO: test */
        {
          retv = 1;
          break;
        }

        mesh->vertices = (FcelibVertex **)malloc((size_t)(mesh->vertices_len * sizeof(FcelibVertex *)));
        if (!mesh->vertices)
        {
          fprintf(stderr, "DecodeFce: Cannot allocate memory\n");
          break;
        }
        memset(mesh->vertices, '\0', (size_t)mesh->vertices_len * sizeof(FcelibVertex *));

        mesh->hdr.NumVertices = 0;

        for (i = 0; i < mesh->hdr.NumParts; ++i)
        {
          /* Get vertices by global fce idx */
          for (j = 0; j < mesh->parts[i]->PNumVertices; ++j)
          {
            mesh->parts[i]->PVertices[j] = mesh->hdr.NumVertices;

            mesh->vertices[mesh->hdr.NumVertices] = (FcelibVertex *)malloc(sizeof(FcelibVertex));
            if (!mesh->vertices[mesh->hdr.NumVertices])
            {
              fprintf(stderr, "DecodeFce: Cannot allocate memory\n");
              break;
            }

            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->VertPos.x, buf + kHdrSize + header.VertTblOffset + (j + header.P1stVertices[i]) * 12 + 0x0, (size_t)4);
            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->VertPos.y, buf + kHdrSize + header.VertTblOffset + (j + header.P1stVertices[i]) * 12 + 0x4, (size_t)4);
            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->VertPos.z, buf + kHdrSize + header.VertTblOffset + (j + header.P1stVertices[i]) * 12 + 0x8, (size_t)4);

            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->NormPos.x, buf + kHdrSize + header.NormTblOffset + (j + header.P1stVertices[i]) * 12 + 0x0, (size_t)4);
            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->NormPos.y, buf + kHdrSize + header.NormTblOffset + (j + header.P1stVertices[i]) * 12 + 0x4, (size_t)4);
            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->NormPos.z, buf + kHdrSize + header.NormTblOffset + (j + header.P1stVertices[i]) * 12 + 0x8, (size_t)4);

            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->DamgdVertPos.x, buf + kHdrSize + header.DamgdVertTblOffset + (j + header.P1stVertices[i]) * 12 + 0x0, (size_t)4);
            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->DamgdVertPos.y, buf + kHdrSize + header.DamgdVertTblOffset + (j + header.P1stVertices[i]) * 12 + 0x4, (size_t)4);
            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->DamgdVertPos.z, buf + kHdrSize + header.DamgdVertTblOffset + (j + header.P1stVertices[i]) * 12 + 0x8, (size_t)4);

            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->DamgdNormPos.x, buf + kHdrSize + header.DamgdNormTblOffset + (j + header.P1stVertices[i]) * 12 + 0x0, (size_t)4);
            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->DamgdNormPos.y, buf + kHdrSize + header.DamgdNormTblOffset + (j + header.P1stVertices[i]) * 12 + 0x4, (size_t)4);
            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->DamgdNormPos.z, buf + kHdrSize + header.DamgdNormTblOffset + (j + header.P1stVertices[i]) * 12 + 0x8, (size_t)4);

            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->Animation, buf + kHdrSize + header.AnimationTblOffset + (j + header.P1stVertices[i]) * 4, (size_t)4);

            ++mesh->hdr.NumVertices;
          }
        }
        for (i = mesh->hdr.NumVertices; i < mesh->vertices_len; ++i)
          mesh->vertices[i] = NULL;

      }  /* case FCE4, FCEM */

      retv = 1;
      break;

      default:  /* FCE3 */
      {
        FceHeader3 header;
        const int kHdrSize = 0x1F04;

        if (FCELIB_FCETYPES_Fce3ValidateHeader(buf, buf_size) != 1)
          return 0;

        header = FCELIB_FCETYPES_GetFceHeader3(buf);

        /* Header ----------------------------------------------------------- */
          /* NumTriangles - counted below */
          /* NumVertices - counted below */
        mesh->hdr.NumParts = header.NumParts;
        mesh->parts_len = 2 * mesh->hdr.NumParts;

        mesh->hdr.Parts = (int *)malloc((size_t)mesh->parts_len * sizeof(int));
        memset(mesh->hdr.Parts, -1, (size_t)mesh->parts_len * sizeof(int));
        for (i = 0; i < header.NumParts; ++i)
          mesh->hdr.Parts[i] = i;

        mesh->hdr.NumDummies = header.NumDummies;
        for (i = 0; i < mesh->hdr.NumDummies; ++i)
        {
          memcpy(&mesh->hdr.Dummies[i].x, buf + 0x0038 + i * 12 + 0x0, (size_t)4);
          memcpy(&mesh->hdr.Dummies[i].y, buf + 0x0038 + i * 12 + 0x4, (size_t)4);
          memcpy(&mesh->hdr.Dummies[i].z, buf + 0x0038 + i * 12 + 0x8, (size_t)4);
          mesh->hdr.DummyNames[(i + 1) * 64 - 1] = '\0';  /* ensure string */
        }
        memcpy(&mesh->hdr.DummyNames, buf + 0x0A04, (size_t)(16 * 64));         /** TODO: ensure string, cleaning up! */

        mesh->hdr.NumColors = header.NumPriColors;
        for (i = 0; i < mesh->hdr.NumColors; ++i)
        {
          /* unsigned char from little-endian int */
          memcpy(&mesh->hdr.PriColors[i].hue,          buf + 0x0800 + i * 16 + 0x0, (size_t)1);
          memcpy(&mesh->hdr.PriColors[i].saturation,   buf + 0x0800 + i * 16 + 0x4, (size_t)1);
          memcpy(&mesh->hdr.PriColors[i].brightness,   buf + 0x0800 + i * 16 + 0x8, (size_t)1);
          memcpy(&mesh->hdr.PriColors[i].transparency, buf + 0x0800 + i * 16 + 0xC, (size_t)1);

          memcpy(&mesh->hdr.DriColors[i].hue,          buf + 0x0800 + i * 16 + 0x0, (size_t)1);
          memcpy(&mesh->hdr.DriColors[i].saturation,   buf + 0x0800 + i * 16 + 0x4, (size_t)1);
          memcpy(&mesh->hdr.DriColors[i].brightness,   buf + 0x0800 + i * 16 + 0x8, (size_t)1);
          memcpy(&mesh->hdr.DriColors[i].transparency, buf + 0x0800 + i * 16 + 0xC, (size_t)1);
        }

        mesh->hdr.NumSecColors = header.NumSecColors;
        for (i = 0; i < mesh->hdr.NumSecColors; ++i)
        {
          /* unsigned char from little-endian int */
          memcpy(&mesh->hdr.SecColors[i].hue,          buf + 0x0904 + i * 16 + 0x0, (size_t)1);
          memcpy(&mesh->hdr.SecColors[i].saturation,   buf + 0x0904 + i * 16 + 0x4, (size_t)1);
          memcpy(&mesh->hdr.SecColors[i].brightness,   buf + 0x0904 + i * 16 + 0x8, (size_t)1);
          memcpy(&mesh->hdr.SecColors[i].transparency, buf + 0x0904 + i * 16 + 0xC, (size_t)1);

          memcpy(&mesh->hdr.IntColors[i].hue,          buf + 0x0904 + i * 16 + 0x0, (size_t)1);
          memcpy(&mesh->hdr.IntColors[i].saturation,   buf + 0x0904 + i * 16 + 0x4, (size_t)1);
          memcpy(&mesh->hdr.IntColors[i].brightness,   buf + 0x0904 + i * 16 + 0x8, (size_t)1);
          memcpy(&mesh->hdr.IntColors[i].transparency, buf + 0x0904 + i * 16 + 0xC, (size_t)1);
        }


        /* Parts ------------------------------------------------------------ */
        if (mesh->hdr.NumParts == 0)  /** TODO: test */
        {
          retv = 1;
          break;
        }

        mesh->parts = (FcelibPart **)malloc((size_t)mesh->parts_len * sizeof(FcelibPart *));
        if (!mesh->parts)
        {
          fprintf(stderr, "Cannot allocate memory\n");
          break;
        }
        memset(mesh->parts, '\0', (size_t)mesh->parts_len * sizeof(FcelibPart *));

        for (i = 0; i < mesh->hdr.NumParts; ++i)
        {
          mesh->parts[i] = (FcelibPart *)malloc(sizeof(FcelibPart));
          if (!mesh->parts[i])
          {
            fprintf(stderr, "Cannot allocate memory\n");
            break;
          }
          memcpy(mesh->parts[i]->PartName, header.PartNames + i * 64, (size_t)64);
          mesh->parts[i]->PartName[63] = '\0';  /* ensure string */

          mesh->parts[i]->PartPos.x = header.PartPos[i].x;
          mesh->parts[i]->PartPos.y = header.PartPos[i].y;
          mesh->parts[i]->PartPos.z = header.PartPos[i].z;

          mesh->parts[i]->PNumVertices = header.PNumVertices[i];
          mesh->parts[i]->pvertices_len = 2 * mesh->parts[i]->PNumVertices;
          mesh->parts[i]->PVertices = NULL;

          mesh->parts[i]->PNumTriangles = header.PNumTriangles[i];
          mesh->parts[i]->ptriangles_len = 2 * mesh->parts[i]->PNumTriangles;
          mesh->parts[i]->PTriangles = NULL;

          /* update global counts */
          mesh->vertices_len += mesh->parts[i]->pvertices_len;
          mesh->triangles_len += mesh->parts[i]->ptriangles_len;

          mesh->parts[i]->PVertices = (int *)malloc((size_t)(mesh->parts[i]->pvertices_len * sizeof(int)));
          if (!mesh->parts[i]->PVertices)
          {
            fprintf(stderr, "Cannot allocate memory\n");
            break;
          }
          memset(mesh->parts[i]->PVertices, -1, (size_t)(mesh->parts[i]->pvertices_len * sizeof(int)));

          mesh->parts[i]->PTriangles = (int *)malloc((size_t)(mesh->parts[i]->ptriangles_len * sizeof(int)));
          if (!mesh->parts[i]->PTriangles)
          {
            fprintf(stderr, "Cannot allocate memory\n");
            break;
          }
          memset(mesh->parts[i]->PTriangles, -1, (size_t)(mesh->parts[i]->ptriangles_len * sizeof(int)));

        }  /* for i */
        for (i = mesh->hdr.NumParts; i < mesh->parts_len; ++i)
          mesh->parts[i] = NULL;


        /* Triangles -------------------------------------------------------- */
        if (mesh->triangles_len == 0)  /** TODO: test */
        {
          retv = 1;
          break;
        }

        mesh->triangles = (FcelibTriangle **)malloc((size_t)mesh->triangles_len * sizeof(FcelibTriangle *));
        if (!mesh->triangles)
        {
          fprintf(stderr, "Cannot allocate memory\n");
          break;
        }
        memset(mesh->triangles, '\0', (size_t)mesh->triangles_len * sizeof(FcelibTriangle *));

        mesh->hdr.NumTriangles = 0;
        mesh->hdr.NumVertices = 0;

        for (i = 0; i < mesh->hdr.NumParts; ++i)
        {
          for (j = 0; j < mesh->parts[i]->PNumTriangles; ++j)
          {
            mesh->parts[i]->PTriangles[j] = mesh->hdr.NumTriangles;

            mesh->triangles[mesh->hdr.NumTriangles] = (FcelibTriangle *)malloc(sizeof(FcelibTriangle));
            if (!mesh->triangles[mesh->hdr.NumTriangles])
            {
              fprintf(stderr, "Cannot allocate memory\n");
              break;
            }

            memcpy(&mesh->triangles[mesh->hdr.NumTriangles]->tex_page, buf + kHdrSize + header.TriaTblOffset + (j + header.P1stTriangles[i]) * 56 + 0x00, (size_t)4);
            memcpy(&mesh->triangles[mesh->hdr.NumTriangles]->vidx,     buf + kHdrSize + header.TriaTblOffset + (j + header.P1stTriangles[i]) * 56 + 0x04, (size_t)12);

            /* Globalize vert index references */
            for (n = 0; n < 3; ++n)
              mesh->triangles[mesh->hdr.NumTriangles]->vidx[n] += mesh->hdr.NumVertices;

            memcpy(&mesh->triangles[mesh->hdr.NumTriangles]->flag, buf + kHdrSize + header.TriaTblOffset + (j + header.P1stTriangles[i]) * 56 + 0x1C, (size_t)4);
            memcpy(&mesh->triangles[mesh->hdr.NumTriangles]->U,    buf + kHdrSize + header.TriaTblOffset + (j + header.P1stTriangles[i]) * 56 + 0x20, (size_t)12);
            memcpy(&mesh->triangles[mesh->hdr.NumTriangles]->V,    buf + kHdrSize + header.TriaTblOffset + (j + header.P1stTriangles[i]) * 56 + 0x2C, (size_t)12);

            ++mesh->hdr.NumTriangles;
          }

          mesh->hdr.NumVertices += mesh->parts[i]->PNumVertices;
        }
        for (i = mesh->hdr.NumTriangles; i < mesh->triangles_len; ++i)
          mesh->triangles[i] = NULL;


        /* Vertices --------------------------------------------------------- */
        if (mesh->vertices_len == 0)  /** TODO: test - this requires (mesh->triangles_len == 0) */
        {
          retv = 1;
          break;
        }

        mesh->vertices = (FcelibVertex **)malloc((size_t)(mesh->vertices_len * sizeof(FcelibVertex *)));
        if (!mesh->vertices)
        {
          fprintf(stderr, "Cannot allocate memory\n");
          break;
        }
        memset(mesh->vertices, '\0', (size_t)mesh->vertices_len * sizeof(FcelibVertex *));

        mesh->hdr.NumVertices = 0;

        for (i = 0; i < mesh->hdr.NumParts; ++i)
        {
          /* Get vertices by global fce idx */
          for (j = 0; j < mesh->parts[i]->PNumVertices; ++j)
          {
            mesh->parts[i]->PVertices[j] = mesh->hdr.NumVertices;

            mesh->vertices[mesh->hdr.NumVertices] = (FcelibVertex *)malloc(sizeof(FcelibVertex));
            if (!mesh->vertices[mesh->hdr.NumVertices])
            {
              fprintf(stderr, "Cannot allocate memory\n");
              break;
            }

            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->VertPos.x, buf + kHdrSize + header.VertTblOffset + (j + header.P1stVertices[i]) * 12 + 0x0, (size_t)4);
            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->VertPos.y, buf + kHdrSize + header.VertTblOffset + (j + header.P1stVertices[i]) * 12 + 0x4, (size_t)4);
            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->VertPos.z, buf + kHdrSize + header.VertTblOffset + (j + header.P1stVertices[i]) * 12 + 0x8, (size_t)4);

            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->NormPos.x, buf + kHdrSize + header.NormTblOffset + (j + header.P1stVertices[i]) * 12 + 0x0, (size_t)4);
            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->NormPos.y, buf + kHdrSize + header.NormTblOffset + (j + header.P1stVertices[i]) * 12 + 0x4, (size_t)4);
            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->NormPos.z, buf + kHdrSize + header.NormTblOffset + (j + header.P1stVertices[i]) * 12 + 0x8, (size_t)4);

            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->DamgdVertPos.x, buf + kHdrSize + header.VertTblOffset + (j + header.P1stVertices[i]) * 12 + 0x0, (size_t)4);
            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->DamgdVertPos.y, buf + kHdrSize + header.VertTblOffset + (j + header.P1stVertices[i]) * 12 + 0x4, (size_t)4);
            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->DamgdVertPos.z, buf + kHdrSize + header.VertTblOffset + (j + header.P1stVertices[i]) * 12 + 0x8, (size_t)4);

            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->DamgdNormPos.x, buf + kHdrSize + header.NormTblOffset + (j + header.P1stVertices[i]) * 12 + 0x0, (size_t)4);
            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->DamgdNormPos.y, buf + kHdrSize + header.NormTblOffset + (j + header.P1stVertices[i]) * 12 + 0x4, (size_t)4);
            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->DamgdNormPos.z, buf + kHdrSize + header.NormTblOffset + (j + header.P1stVertices[i]) * 12 + 0x8, (size_t)4);

            mesh->vertices[mesh->hdr.NumVertices]->Animation = 0x0;

            ++mesh->hdr.NumVertices;
          }
        }
        for (i = mesh->hdr.NumVertices; i < mesh->vertices_len; ++i)
          mesh->vertices[i] = NULL;

      }  /* case default */

      /* Tidy up names */
      for (i = 0; i < mesh->hdr.NumDummies; i++)
      {
        n = strlen(mesh->hdr.DummyNames + i * 64);
        memset(mesh->hdr.DummyNames + i * 64 + n, '\0', (size_t)(64 - n));
      }
      memset(mesh->hdr.DummyNames + mesh->hdr.NumDummies * 64, '\0', (size_t)((16 - mesh->hdr.NumDummies) * 64));
      for (i = 0; i < mesh->hdr.NumParts; ++i)
      {
        n = strlen(mesh->parts[i]->PartName);
        memset(mesh->parts[i]->PartName + n, '\0', (size_t)(64 - n));
      }

      retv = 1;
      break;
    }

    break;
  }  /* for (;;) */

  if (retv != 1)
    FCELIB_TYPES_FreeMesh(mesh);

  return retv;
}


/* encode ------------------------------------------------------------------- */

/* The FCE triangle flag is output as material name. Returns boolean. */
int FCELIB_IO_ExportObj(FcelibMesh *mesh,
                        const void *objpath, const void *mtlpath,
                        const char *texture_name,
                        const int print_damage, const int print_dummies)
{
  int retv = 1;
  int i;
  int j;
  int n;
  FILE *outf = NULL;
  int sum_verts = 0;
  int sum_triags = 0;
  int *global_mesh_to_global_obj_idxs;
  FcelibPart *part;

  if (!FCELIB_TYPES_ValidateMesh(*mesh))
    return 0;

  global_mesh_to_global_obj_idxs = (int *)malloc(mesh->vertices_len * sizeof(int));
  if (!global_mesh_to_global_obj_idxs)
  {
    fprintf(stderr, "ExportObj: Cannot allocate memory\n");
    return 0;
  }

  for (;;)
  {
    /* Print mtl (used triangle 12-bit flags as materials) ------------------ */
    {
      char mtls[4096];
      memset(mtls, '0', sizeof(mtls));
      int count_mtls = 0;

      for (i = 0; i < mesh->triangles_len; ++i)
      {
        if (mesh->triangles[i])
        {
          if (mtls[mesh->triangles[i]->flag & 0xfff] != '1')
          {
            mtls[mesh->triangles[i]->flag & 0xfff] = '1';
            ++count_mtls;
          }
        }
      }

      outf = fopen((char *)mtlpath, "wb");
      if (!outf)
      {
        fprintf(stderr, "ExportObj: cannot create file '%s'\n", (char *)mtlpath);
        retv = 0;
        break;
      }

      fprintf(outf, "# fcecodec MTL File: '%s'\n"
                    "# Material Count: %d\n",
                    (char *)objpath, count_mtls);

      for (i = 0; i < 4096; ++i)
      {
        if (mtls[i] == '1')
        {
          fprintf(outf, "\n"
                        "newmtl 0x%03x\n"
                        "Ka 1.000 1.000 1.000\n"
                        "Kd 1.000 1.000 1.000\n"
                        "Ks 0.000 0.000 0.000\n"
                        "d 0.7\n"
                        "Tr 0.3\n"
                        "illum 2\n"
                        "map_Kd %s\n",
                        i, texture_name);
        }
      }

      if (fclose(outf) != 0)
      {
        fprintf(stderr, "ExportObj: cannot close file '%s'\n", (char *)mtlpath);
        retv = 0;
        break;
      }
      outf = NULL;
    }

    /* Print obj ------------------------------------------------------------ */
    outf = fopen((char *)objpath, "wb");
    if (!outf)
    {
      fprintf(stderr, "ExportObj: cannot create file '%s'\n", (char *)objpath);
      retv = 0;
      break;
    }

    fprintf(outf, "# fcecodec OBJ File: '%s'\n"
                  "# github.com/bfut/fcecodec\n"
                  "mtllib %s\n", (char *)objpath, (char *)mtlpath);

    for (i = 0; i < mesh->parts_len; ++i)
    {
      if (mesh->hdr.Parts[i] < 0)
        continue;

      part = mesh->parts[ mesh->hdr.Parts[i] ];
      if (!part)
      {
        fprintf(stderr, "ExportObj: unexpected NULL pointer (mesh->parts[mesh->hdr.Parts[i]]\n");
        retv = 0;
        break;
      }

      /* BEGIN printing undamaged part */

      /* Create map: global vert index to local part idx (of used-in-this-part verts) */
      memset(global_mesh_to_global_obj_idxs, -1, (size_t)mesh->vertices_len * sizeof(int));
      for (j = 0; j < mesh->parts[ mesh->hdr.Parts[i] ]->pvertices_len; ++j)
      {
        if (mesh->parts[ mesh->hdr.Parts[i] ]->PVertices[j] < 0)
          continue;

        global_mesh_to_global_obj_idxs[
          mesh->parts[ mesh->hdr.Parts[i] ]->PVertices[j]
        ] = j + 1 + sum_verts;
      }

      fprintf(outf, "\no %s\n", mesh->parts[ mesh->hdr.Parts[i] ]->PartName);

      fprintf(outf, "#part position %f %f %f\n",
                    mesh->parts[ mesh->hdr.Parts[i] ]->PartPos.x,
                    mesh->parts[ mesh->hdr.Parts[i] ]->PartPos.y,
                    mesh->parts[ mesh->hdr.Parts[i] ]->PartPos.z);
      fprintf(outf, "\n");

      /* Verts */
      fprintf(outf, "#%d verts\n", mesh->parts[ mesh->hdr.Parts[i] ]->PNumVertices);
      for (j = 0; j < mesh->parts[ mesh->hdr.Parts[i] ]->pvertices_len; ++j)
      {
        if (mesh->parts[ mesh->hdr.Parts[i] ]->PVertices[j] < 0)
          continue;

        fprintf(outf, "v %f %f %f\n",
                      mesh->vertices[
                        mesh->parts[ mesh->hdr.Parts[i] ]->PVertices[j]
                      ]->VertPos.x + mesh->parts[ mesh->hdr.Parts[i] ]->PartPos.x,
                      mesh->vertices[
                        mesh->parts[ mesh->hdr.Parts[i] ]->PVertices[j]
                      ]->VertPos.y + mesh->parts[ mesh->hdr.Parts[i] ]->PartPos.y,
                      - (
                        mesh->vertices[
                          mesh->parts[ mesh->hdr.Parts[i] ]->PVertices[j]
                        ]->VertPos.z + mesh->parts[ mesh->hdr.Parts[i] ]->PartPos.z
                      )
               );
      }
      fprintf(outf, "\n");

      /* Texture coordinates */
      fprintf(outf, "#%d vt\n", 3 * mesh->parts[ mesh->hdr.Parts[i] ]->PNumTriangles);
      for (j = 0; j < mesh->parts[ mesh->hdr.Parts[i] ]->ptriangles_len; ++j)
      {
        if (mesh->parts[ mesh->hdr.Parts[i] ]->PTriangles[j] < 0)
          continue;

        for (n = 0; n < 3; ++n)
        {
          fprintf(outf, "vt %f %f\n",
                      mesh->triangles[
                        mesh->parts[ mesh->hdr.Parts[i] ]->PTriangles[j]
                      ]->U[n],
                      mesh->triangles[
                        mesh->parts[ mesh->hdr.Parts[i] ]->PTriangles[j]
                      ]->V[n]
               );
        }  /* for n */
      }
      fprintf(outf, "\n");

      /** Normals (TODO: really add part pos ???) */
      fprintf(outf, "#%d normals\n", mesh->parts[ mesh->hdr.Parts[i] ]->PNumVertices);
      for (j = 0; j < mesh->parts[ mesh->hdr.Parts[i] ]->pvertices_len; ++j)
      {
        if (mesh->parts[ mesh->hdr.Parts[i] ]->PVertices[j] < 0)
          continue;

        fprintf(outf, "vn %f %f %f\n",
                      mesh->vertices[
                        mesh->parts[ mesh->hdr.Parts[i] ]->PVertices[j]
                      ]->NormPos.x + mesh->parts[ mesh->hdr.Parts[i] ]->PartPos.x,
                      mesh->vertices[
                        mesh->parts[ mesh->hdr.Parts[i] ]->PVertices[j]
                      ]->NormPos.y + mesh->parts[ mesh->hdr.Parts[i] ]->PartPos.y,
                      - (
                        mesh->vertices[
                          mesh->parts[ mesh->hdr.Parts[i] ]->PVertices[j]
                        ]->NormPos.z + mesh->parts[ mesh->hdr.Parts[i] ]->PartPos.z
                      )
               );
      }
      fprintf(outf, "\n");

      /* Triangles */
      fprintf(outf, "#%d faces (verts: %d..%d)\n", mesh->parts[ mesh->hdr.Parts[i] ]->PNumTriangles, sum_verts + 1, sum_verts + mesh->parts[ mesh->hdr.Parts[i] ]->PNumVertices);
      for (j = 0; j < mesh->parts[ mesh->hdr.Parts[i] ]->ptriangles_len; ++j)
      {

        if (mesh->parts[ mesh->hdr.Parts[i] ]->PTriangles[j] < 0)
          continue;

        fprintf(outf, "usemtl 0x%03x\n"
                      "s 1\n",
                      mesh->triangles[
                        mesh->parts[ mesh->hdr.Parts[i] ]->PTriangles[j]
                      ]->flag & 0xfff);

        fprintf(outf, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
                      global_mesh_to_global_obj_idxs[
                        mesh->triangles[
                          mesh->parts[ mesh->hdr.Parts[i] ]->PTriangles[j]
                        ]->vidx[0]
                      ],
                      3 * (j + sum_triags) + 1 + 0,
                      global_mesh_to_global_obj_idxs[
                        mesh->triangles[
                          mesh->parts[ mesh->hdr.Parts[i] ]->PTriangles[j]
                        ]->vidx[0]
                      ],

                      global_mesh_to_global_obj_idxs[
                        mesh->triangles[
                          mesh->parts[ mesh->hdr.Parts[i] ]->PTriangles[j]
                        ]->vidx[1]
                      ],
                      3 * (j + sum_triags) + 1 + 1,
                      global_mesh_to_global_obj_idxs[
                        mesh->triangles[
                          mesh->parts[ mesh->hdr.Parts[i] ]->PTriangles[j]
                        ]->vidx[1]
                      ],

                      global_mesh_to_global_obj_idxs[
                        mesh->triangles[
                          mesh->parts[ mesh->hdr.Parts[i] ]->PTriangles[j]
                        ]->vidx[2]
                      ],
                      3 * (j + sum_triags) + 1 + 2,
                      global_mesh_to_global_obj_idxs[
                        mesh->triangles[
                          mesh->parts[ mesh->hdr.Parts[i] ]->PTriangles[j]
                        ]->vidx[2]
                      ]
                );
      }  /* for j triangles */
      fprintf(outf, "\n");

      sum_verts  += mesh->parts[ mesh->hdr.Parts[i] ]->PNumVertices;
      sum_triags += mesh->parts[ mesh->hdr.Parts[i] ]->PNumTriangles;

      /* END printing undamaged part */


      /* BEGIN printing damaged part */
      if (print_damage)
      {
        /* Create map: global vert index to local part idx (of used-in-this-part verts) */
        memset(global_mesh_to_global_obj_idxs, -1, (size_t)mesh->vertices_len * sizeof(int));
        for (j = 0; j < mesh->parts[ mesh->hdr.Parts[i] ]->pvertices_len; ++j)
        {
          if (mesh->parts[ mesh->hdr.Parts[i] ]->PVertices[j] < 0)
            continue;

          global_mesh_to_global_obj_idxs[
            mesh->parts[ mesh->hdr.Parts[i] ]->PVertices[j]
          ] = j + 1 + sum_verts;
        }

        fprintf(outf, "\no DAMAGE_%s\n", mesh->parts[ mesh->hdr.Parts[i] ]->PartName);

        fprintf(outf, "#part position %f %f %f\n",
                      mesh->parts[ mesh->hdr.Parts[i] ]->PartPos.x,
                      mesh->parts[ mesh->hdr.Parts[i] ]->PartPos.y,
                      mesh->parts[ mesh->hdr.Parts[i] ]->PartPos.z);
        fprintf(outf, "\n");

        /* Verts */
        fprintf(outf, "#%d verts\n", mesh->parts[ mesh->hdr.Parts[i] ]->PNumVertices);
        for (j = 0; j < mesh->parts[ mesh->hdr.Parts[i] ]->pvertices_len; ++j)
        {
          if (mesh->parts[ mesh->hdr.Parts[i] ]->PVertices[j] < 0)
            continue;

          fprintf(outf, "v %f %f %f\n",
                        mesh->vertices[
                          mesh->parts[ mesh->hdr.Parts[i] ]->PVertices[j]
                        ]->DamgdVertPos.x + mesh->parts[ mesh->hdr.Parts[i] ]->PartPos.x,
                        mesh->vertices[
                          mesh->parts[ mesh->hdr.Parts[i] ]->PVertices[j]
                        ]->DamgdVertPos.y + mesh->parts[ mesh->hdr.Parts[i] ]->PartPos.y,
                        - (
                          mesh->vertices[
                            mesh->parts[ mesh->hdr.Parts[i] ]->PVertices[j]
                          ]->DamgdVertPos.z + mesh->parts[ mesh->hdr.Parts[i] ]->PartPos.z
                        )
                );
        }
        fprintf(outf, "\n");

        /* Texture coordinates */
        fprintf(outf, "#%d vt\n", 3 * mesh->parts[ mesh->hdr.Parts[i] ]->PNumTriangles);
        for (j = 0; j < mesh->parts[ mesh->hdr.Parts[i] ]->ptriangles_len; ++j)
        {
          if (mesh->parts[ mesh->hdr.Parts[i] ]->PTriangles[j] < 0)
            continue;

          for (n = 0; n < 3; ++n)
          {
            fprintf(outf, "vt %f %f\n",
                        mesh->triangles[
                          mesh->parts[ mesh->hdr.Parts[i] ]->PTriangles[j]
                        ]->U[n],
                        mesh->triangles[
                          mesh->parts[ mesh->hdr.Parts[i] ]->PTriangles[j]
                        ]->V[n]
                );
          }  /* for n */
        }
        fprintf(outf, "\n");

        /** Normals (TODO: really add part pos ???) */
        fprintf(outf, "#%d normals\n", mesh->parts[ mesh->hdr.Parts[i] ]->PNumVertices);
        for (j = 0; j < mesh->parts[ mesh->hdr.Parts[i] ]->pvertices_len; ++j)
        {
          if (mesh->parts[ mesh->hdr.Parts[i] ]->PVertices[j] < 0)
            continue;

          fprintf(outf, "vn %f %f %f\n",
                        mesh->vertices[
                          mesh->parts[ mesh->hdr.Parts[i] ]->PVertices[j]
                        ]->DamgdNormPos.x + mesh->parts[ mesh->hdr.Parts[i] ]->PartPos.x,
                        mesh->vertices[
                          mesh->parts[ mesh->hdr.Parts[i] ]->PVertices[j]
                        ]->DamgdNormPos.y + mesh->parts[ mesh->hdr.Parts[i] ]->PartPos.y,
                        - (
                          mesh->vertices[
                            mesh->parts[ mesh->hdr.Parts[i] ]->PVertices[j]
                          ]->DamgdNormPos.z + mesh->parts[ mesh->hdr.Parts[i] ]->PartPos.z
                        )
                );
        }
        fprintf(outf, "\n");

        /* Triangles */
        fprintf(outf, "#%d faces (verts: %d..%d)\n", mesh->parts[ mesh->hdr.Parts[i] ]->PNumTriangles, sum_verts + 1, sum_verts + mesh->parts[ mesh->hdr.Parts[i] ]->PNumVertices);
        for (j = 0; j < mesh->parts[ mesh->hdr.Parts[i] ]->ptriangles_len; ++j)
        {

          if (mesh->parts[ mesh->hdr.Parts[i] ]->PTriangles[j] < 0)
            continue;

          fprintf(outf, "usemtl 0x%03x\n"
                        "s 1\n",
                        mesh->triangles[
                          mesh->parts[ mesh->hdr.Parts[i] ]->PTriangles[j]
                        ]->flag & 0xfff);

          fprintf(outf, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
                        global_mesh_to_global_obj_idxs[
                          mesh->triangles[
                            mesh->parts[ mesh->hdr.Parts[i] ]->PTriangles[j]
                          ]->vidx[0]
                        ],
                        3 * (j + sum_triags) + 1 + 0,
                        global_mesh_to_global_obj_idxs[
                          mesh->triangles[
                            mesh->parts[ mesh->hdr.Parts[i] ]->PTriangles[j]
                          ]->vidx[0]
                        ],

                        global_mesh_to_global_obj_idxs[
                          mesh->triangles[
                            mesh->parts[ mesh->hdr.Parts[i] ]->PTriangles[j]
                          ]->vidx[1]
                        ],
                        3 * (j + sum_triags) + 1 + 1,
                        global_mesh_to_global_obj_idxs[
                          mesh->triangles[
                            mesh->parts[ mesh->hdr.Parts[i] ]->PTriangles[j]
                          ]->vidx[1]
                        ],

                        global_mesh_to_global_obj_idxs[
                          mesh->triangles[
                            mesh->parts[ mesh->hdr.Parts[i] ]->PTriangles[j]
                          ]->vidx[2]
                        ],
                        3 * (j + sum_triags) + 1 + 2,
                        global_mesh_to_global_obj_idxs[
                          mesh->triangles[
                            mesh->parts[ mesh->hdr.Parts[i] ]->PTriangles[j]
                          ]->vidx[2]
                        ]
                  );
        }  /* for j triangles */
        fprintf(outf, "\n");

        sum_verts  += mesh->parts[ mesh->hdr.Parts[i] ]->PNumVertices;
        sum_triags += mesh->parts[ mesh->hdr.Parts[i] ]->PNumTriangles;

      }  /* if (print_damage) */
      /* END printing undamaged part */


    }  /* for i parts */

    if (print_dummies)
    {
      for (i = 0; i < mesh->hdr.NumDummies; ++i)
      {
        fprintf(outf, "\no DUMMY_%s\n", mesh->hdr.DummyNames + (i * 64));

        /* Vertices */
        fprintf(outf, "#position %f %f %f\n",
                      mesh->hdr.Dummies[i].x,
                      mesh->hdr.Dummies[i].y,
                      mesh->hdr.Dummies[i].z);

        for (j = 0; j < 6; ++j)
        {
          fprintf(outf, "v %f %f %f\n",
                        0.1f * kVertDiamond[3 * j + 0] + mesh->hdr.Dummies[i].x,
                        0.1f * kVertDiamond[3 * j + 1] + mesh->hdr.Dummies[i].y,
                        0.1f * kVertDiamond[3 * j + 2] + mesh->hdr.Dummies[i].z * (-1)
                 );
        }

        /* Triangles */
        fprintf(outf, "\n#f %d..%d (%d)\n", sum_triags + 1, sum_triags + 8, 8);

        for (j = 0; j < 8; ++j)
        {
          fprintf(outf, "f %d %d %d\n",
                        kTrianglesDiamond[3 * j + 0] + sum_verts,
                        kTrianglesDiamond[3 * j + 1] + sum_verts,
                        kTrianglesDiamond[3 * j + 2] + sum_verts
                 );
        }

        sum_verts  += 6;
        sum_triags += 8;
      }
    }  /* if (print_dummies) */

    if (fclose(outf) != 0)
    {
      fprintf(stderr, "ExportObj: cannot close file '%s'\n", (char *)objpath);
      retv = 0;
      break;
    }

    break;
  }/* for (;;) */

  free(global_mesh_to_global_obj_idxs);
  return retv;
}


/* Returns boolean. Limited to 64 parts. center_parts == 1 centers all parts. */
int FCELIB_IO_EncodeFce3_Fopen(FcelibMesh *mesh, const void *fcepath, const int center_parts)
{
  int retv = 1;
  int i;
  int j;
  int n;
  int k;
  int m;
  FILE *outf = NULL;
  int sum_verts = 0;
  int sum_triags = 0;
  int *global_mesh_to_local_fce_idxs;
  FcelibPart *part;
  int buf;
  unsigned char buffer[64];
  
  fprintf(stderr, "Deprecated Warning: use FCELIB_IO_EncodeFce3 instead");

  if (!FCELIB_TYPES_ValidateMesh(*mesh))
    return 0;

  global_mesh_to_local_fce_idxs = (int *)malloc(mesh->vertices_len * sizeof(int));
  if (!global_mesh_to_local_fce_idxs)
  {
    fprintf(stderr, "EncodeFce3: Cannot allocate memory\n");
    return 0;
  }

  for (;;)
  {
    outf = fopen((char *)fcepath, "wb");
    if (!outf)
    {
      fprintf(stderr, "EncodeFce3: cannot create file '%s'\n", (char *)fcepath);
      retv = 0;
      break;
    }

    /* Print header --------------------------------------------------------- */
    memset(buffer, '\0', (size_t)64);

    buf = 0;
    fwrite(&buf, sizeof(int), (size_t)1, outf);
    fwrite(&mesh->hdr.NumTriangles, sizeof(int), (size_t)1, outf);
    fwrite(&mesh->hdr.NumVertices, sizeof(int), (size_t)1, outf);
    buf = 1;
    fwrite(&buf, sizeof(int), (size_t)1, outf);

    buf = 0;
    fwrite(&buf, sizeof(int), (size_t)1, outf);
    buf += 12 * mesh->hdr.NumVertices;
    fwrite(&buf, sizeof(int), (size_t)1, outf);
    buf += 12 * mesh->hdr.NumVertices;
    fwrite(&buf, sizeof(int), (size_t)1, outf);

    buf += 56 * mesh->hdr.NumTriangles;
    fwrite(&buf, sizeof(int), (size_t)1, outf);
    buf += 32 * mesh->hdr.NumVertices;
    fwrite(&buf, sizeof(int), (size_t)1, outf);
    buf += 12 * mesh->hdr.NumVertices;
    fwrite(&buf, sizeof(int), (size_t)1, outf);

    /* Center high body parts around local centroid (order idxs: 0-4, 12) */
    if (center_parts == 1)
    {
      tVector centroid;
      // i - internal part index, j - part order
      for (i = 0, j = 0; i < mesh->parts_len && j < FCELIB_MISC_Min(12, mesh->hdr.NumParts); ++i)
      {
        if (mesh->hdr.Parts[i] < 0 || (j > 4 && j != 12))
          continue;

        part = mesh->parts[ mesh->hdr.Parts[i] ];
        FCELIB_TYPES_GetPartLocalCentroid(mesh, part, &centroid);
        FCELIB_TYPES_ResetPartPos(mesh, part, centroid);

        ++j;
      }
    }

    /* Compute HalfSize from high body parts (order idxs: 0-4, 12) */
    {
      float *x_array = NULL;
      float *y_array;
      float *z_array;
      FcelibVertex *vert;
      int count_verts = 0;

      x_array = (float *)malloc((size_t)(3 * (mesh->vertices_len + 1)) * sizeof(*x_array));
      if (!x_array)
      {
        fprintf(stderr, "EncodeFce3: Cannot allocate memory\n");
        retv = 0;
        break;
      }
      memset(x_array, '\0', (size_t)(3 * (mesh->vertices_len + 1)) * sizeof(*x_array));
      y_array = x_array + mesh->vertices_len;
      z_array = y_array + mesh->vertices_len;

      // i - internal part index, j - part order
      for (i = 0, j = 0; i < mesh->parts_len && j < FCELIB_MISC_Min(12, mesh->hdr.NumParts); ++i)
      {
        if (mesh->hdr.Parts[i] < 0 || (j > 4 && j != 12))
          continue;

        part = mesh->parts[ mesh->hdr.Parts[i] ];

        // n - internal vert index, k - vert order
        for (n = 0, k = 0; n < part->pvertices_len && k < part->PNumVertices; ++n)
        {
          if (part->PVertices[n] < 0)
            continue;

          vert = mesh->vertices[ part->PVertices[n] ];
          x_array[count_verts + k] = vert->VertPos.x + part->PartPos.x;
          y_array[count_verts + k] = vert->VertPos.y + part->PartPos.y;
          z_array[count_verts + k] = vert->VertPos.z + part->PartPos.z;

          ++k;
        }
        count_verts += k - 1;

        ++j;
      }

      qsort(x_array, (size_t)count_verts, (size_t)4, FCELIB_MISC_CompareFloats);
      qsort(y_array, (size_t)count_verts, (size_t)4, FCELIB_MISC_CompareFloats);
      qsort(z_array, (size_t)count_verts, (size_t)4, FCELIB_MISC_CompareFloats);

      x_array[0] = 0.5f * abs(x_array[count_verts - 1] - x_array[0]);
      y_array[0] = abs(y_array[0]) - 0.02f;
      z_array[0] = 0.5f * abs(z_array[count_verts - 1] - z_array[0]);

      fwrite(x_array, sizeof(float), (size_t)1, outf);
      fwrite(y_array, sizeof(float), (size_t)1, outf);
      fwrite(z_array, sizeof(float), (size_t)1, outf);

      free(x_array);
    }  /* Set HalfSizes */

    fwrite(&mesh->hdr.NumDummies, sizeof(int), (size_t)1, outf);
    for (i = 0; i < mesh->hdr.NumDummies; ++i)
    {
      fwrite(&mesh->hdr.Dummies[i].x, sizeof(float), (size_t)1, outf);
      fwrite(&mesh->hdr.Dummies[i].y, sizeof(float), (size_t)1, outf);
      fwrite(&mesh->hdr.Dummies[i].z, sizeof(float), (size_t)1, outf);
    }
    buf = 0;
    for (i = mesh->hdr.NumDummies; i < 16; ++i)
    {
      fwrite(&buf, sizeof(float), (size_t)1, outf);
      fwrite(&buf, sizeof(float), (size_t)1, outf);
      fwrite(&buf, sizeof(float), (size_t)1, outf);
    }

    /* PartPos */
    fwrite(&mesh->hdr.NumParts, sizeof(int), (size_t)1, outf);
    for (i = 0, j = 0; (j < FCELIB_MISC_Min(64, mesh->hdr.NumParts)) && (i < mesh->parts_len); ++i)
    {
      if (mesh->hdr.Parts[i] < 0)
        continue;

      part = mesh->parts[ mesh->hdr.Parts[i] ];
      fwrite(&part->PartPos.x, sizeof(float), (size_t)1, outf);
      fwrite(&part->PartPos.y, sizeof(float), (size_t)1, outf);
      fwrite(&part->PartPos.z, sizeof(float), (size_t)1, outf);
      ++j;
    }
    buf = 0;
    for (i = mesh->hdr.NumParts; i < 64; ++i)
    {
      fwrite(&buf, sizeof(float), (size_t)1, outf);
      fwrite(&buf, sizeof(float), (size_t)1, outf);
      fwrite(&buf, sizeof(float), (size_t)1, outf);
    }

    /* P1stVertices */
    for (i = 0, j = 0; (j < FCELIB_MISC_Min(64, mesh->hdr.NumParts)) && (i < mesh->parts_len); ++i)
    {
      if (mesh->hdr.Parts[i] < 0)
        continue;

      part = mesh->parts[ mesh->hdr.Parts[i] ];
      fwrite(&sum_verts, sizeof(int), (size_t)1, outf);

      sum_verts += part->PNumVertices;
      ++j;
    }
    buf = 0;
    for (i = mesh->hdr.NumParts; i < 64; ++i)
      fwrite(&buf, sizeof(int), (size_t)1, outf);

    /* PNumVertices */
    for (i = 0, j = 0; (j < FCELIB_MISC_Min(64, mesh->hdr.NumParts)) && (i < mesh->parts_len); ++i)
    {
      if (mesh->hdr.Parts[i] < 0)
        continue;

      part = mesh->parts[ mesh->hdr.Parts[i] ];
      fwrite(&part->PNumVertices, sizeof(int), (size_t)1, outf);

      ++j;
    }
    buf = 0;
    for (i = mesh->hdr.NumParts; i < 64; ++i)
      fwrite(&buf, sizeof(int), (size_t)1, outf);

    /* P1stTriangles */
    for (i = 0, j = 0; (j < FCELIB_MISC_Min(64, mesh->hdr.NumParts)) && (i < mesh->parts_len); ++i)
    {
      if (mesh->hdr.Parts[i] < 0)
        continue;

      part = mesh->parts[ mesh->hdr.Parts[i] ];
      fwrite(&sum_triags, sizeof(int), (size_t)1, outf);

      sum_triags += part->PNumTriangles;
      ++j;
    }
    buf = 0;
    for (i = mesh->hdr.NumParts; i < 64; ++i)
      fwrite(&buf, sizeof(int), (size_t)1, outf);

    /* PNumTriangles */
    for (i = 0, j = 0; (j < FCELIB_MISC_Min(64, mesh->hdr.NumParts)) && (i < mesh->parts_len); ++i)
    {
      if (mesh->hdr.Parts[i] < 0)
        continue;

      part = mesh->parts[ mesh->hdr.Parts[i] ];
      fwrite(&part->PNumTriangles, sizeof(int), (size_t)1, outf);

      ++j;
    }
    buf = 0;
    for (i = mesh->hdr.NumParts; i < 64; ++i)
      fwrite(&buf, sizeof(int), (size_t)1, outf);

    /* PriColors */
    fwrite(&mesh->hdr.NumColors, sizeof(int), (size_t)1, outf);
    buf = 0;
    for (i = 0; i < mesh->hdr.NumColors; ++i)
    {
      memcpy(&buf, &mesh->hdr.PriColors[i].hue,          (size_t)1);
      fwrite(&buf, sizeof(int), (size_t)1, outf);

      memcpy(&buf, &mesh->hdr.PriColors[i].saturation,   (size_t)1);
      fwrite(&buf, sizeof(int), (size_t)1, outf);

      memcpy(&buf, &mesh->hdr.PriColors[i].brightness,   (size_t)1);
      fwrite(&buf, sizeof(int), (size_t)1, outf);

      memcpy(&buf, &mesh->hdr.PriColors[i].transparency, (size_t)1);
      fwrite(&buf, sizeof(int), (size_t)1, outf);
    }
    for (i = mesh->hdr.NumColors; i < 16; ++i)
      fwrite(buffer, (size_t)1, (size_t)(4 * sizeof(int)), outf);

    /* SecColors */
    fwrite(&mesh->hdr.NumSecColors, sizeof(int), (size_t)1, outf);
    buf = 0;
    for (i = 0; i < mesh->hdr.NumSecColors; ++i)
    {
      memcpy(&buf, &mesh->hdr.SecColors[i].hue,          (size_t)1);
      fwrite(&buf, sizeof(int), (size_t)1, outf);

      memcpy(&buf, &mesh->hdr.SecColors[i].saturation,   (size_t)1);
      fwrite(&buf, sizeof(int), (size_t)1, outf);

      memcpy(&buf, &mesh->hdr.SecColors[i].brightness,   (size_t)1);
      fwrite(&buf, sizeof(int), (size_t)1, outf);

      memcpy(&buf, &mesh->hdr.SecColors[i].transparency, (size_t)1);
      fwrite(&buf, sizeof(int), (size_t)1, outf);
    }
    for (i = mesh->hdr.NumSecColors; i < 16; ++i)
      fwrite(buffer, (size_t)1, (size_t)(4 * sizeof(int)), outf);

    /* DummyNames */
    fwrite(&mesh->hdr.DummyNames, (size_t)(16 * 64), (size_t)1, outf);

    /* PartNames */
    for (i = 0, j = 0; (j < FCELIB_MISC_Min(64, mesh->hdr.NumParts)) && (i < mesh->parts_len); ++i)
    {
      if (mesh->hdr.Parts[i] < 0)
        continue;

      part = mesh->parts[ mesh->hdr.Parts[i] ];
      fwrite(&part->PartName, (size_t)64, (size_t)1, outf);

      ++j;
    }
    for (i = mesh->hdr.NumParts; i < 64; ++i)
      fwrite(buffer, (size_t)1, (size_t)64, outf);

    /* Unknown2 */
    for (i = 0; i < 4; ++i)
      fwrite(buffer, (size_t)1, (size_t)64, outf);


    /* Print vertices ------------------------------------------------------- */
    for (i = 0, j = 0; (j < FCELIB_MISC_Min(64, mesh->hdr.NumParts)) && (i < mesh->parts_len); ++i)
    {
      if (mesh->hdr.Parts[i] < 0)
        continue;

      part = mesh->parts[ mesh->hdr.Parts[i] ];
      for (n = 0, k = 0; (k < part->PNumVertices) && (n < part->pvertices_len); ++n)
      {
        if (part->PVertices[n] < 0)
          continue;

        memcpy(buffer + 0x00, &mesh->vertices[ part->PVertices[n] ]->VertPos.x, (size_t)4);
        memcpy(buffer + 0x04, &mesh->vertices[ part->PVertices[n] ]->VertPos.y, (size_t)4);
        memcpy(buffer + 0x08, &mesh->vertices[ part->PVertices[n] ]->VertPos.z, (size_t)4);
        fwrite(buffer, (size_t)12, (size_t)1, outf);

        ++k;
      }

      ++j;
    }


    /* Print normals -------------------------------------------------------- */
    for (i = 0, j = 0; (j < FCELIB_MISC_Min(64, mesh->hdr.NumParts)) && (i < mesh->parts_len); ++i)
    {
      if (mesh->hdr.Parts[i] < 0)
        continue;

      part = mesh->parts[ mesh->hdr.Parts[i] ];
      for (n = 0, k = 0; (k < part->PNumVertices) && (n < part->pvertices_len); ++n)
      {
        if (part->PVertices[n] < 0)
          continue;

        memcpy(buffer + 0x00, &mesh->vertices[ part->PVertices[n] ]->NormPos.x, (size_t)4);
        memcpy(buffer + 0x04, &mesh->vertices[ part->PVertices[n] ]->NormPos.y, (size_t)4);
        memcpy(buffer + 0x08, &mesh->vertices[ part->PVertices[n] ]->NormPos.z, (size_t)4);
        fwrite(buffer, (size_t)12, (size_t)1, outf);

        ++k;
      }

      ++j;
    }


    /* Print triangles ------------------------------------------------------ */
    for (i = 0, j = 0; (j < FCELIB_MISC_Min(64, mesh->hdr.NumParts)) && (i < mesh->parts_len); ++i)
    {
      if (mesh->hdr.Parts[i] < 0)
        continue;

      part = mesh->parts[ mesh->hdr.Parts[i] ];

      /* Create map: global vert index to local part idx (of used-in-this-part verts) */
      memset(global_mesh_to_local_fce_idxs, -1, (size_t)mesh->vertices_len * sizeof(int));
      for (n = 0; n < part->pvertices_len; ++n)
      {
        if (part->PVertices[n] < 0)
          continue;

        global_mesh_to_local_fce_idxs[
          part->PVertices[n]
        ] = n;
      }

      for (n = 0, k = 0; (k < part->PNumTriangles) && (n < part->ptriangles_len); ++n)
      {
        if (part->PTriangles[n] < 0)
          continue;

        memcpy(buffer + 0x00, &mesh->triangles[ part->PTriangles[n] ]->tex_page, (size_t)4);
        memcpy(buffer + 0x04, &global_mesh_to_local_fce_idxs[ mesh->triangles[ part->PTriangles[n] ]->vidx[0] ], (size_t)4);
        memcpy(buffer + 0x08, &global_mesh_to_local_fce_idxs[ mesh->triangles[ part->PTriangles[n] ]->vidx[1] ], (size_t)4);
        memcpy(buffer + 0x0C, &global_mesh_to_local_fce_idxs[ mesh->triangles[ part->PTriangles[n] ]->vidx[2] ], (size_t)4);
        for (m = 0; m < 6; ++m)
        {
          buf = 0xff00;
          memcpy(buffer + 0x10 + m * 2 + 0x0, &buf, (size_t)2);
        }
        memcpy(buffer + 0x1C, &mesh->triangles[ part->PTriangles[n] ]->flag, (size_t)4);
        memcpy(buffer + 0x20, &mesh->triangles[ part->PTriangles[n] ]->U, (size_t)12);
        memcpy(buffer + 0x2C, &mesh->triangles[ part->PTriangles[n] ]->V, (size_t)12);

        fwrite(buffer, (size_t)56, (size_t)1, outf);

        ++k;
      }

      ++j;
    }


    /* Print Reserve1 ------------------------------------------------------- */
    memset(buffer, '\0', (size_t)32);
    for (i = 0, j = 0; (j < FCELIB_MISC_Min(64, mesh->hdr.NumParts)) && (i < mesh->parts_len); ++i)
    {
      if (mesh->hdr.Parts[i] < 0)
        continue;

      part = mesh->parts[ mesh->hdr.Parts[i] ];
      for (n = 0; n < part->PNumVertices; ++n)
        fwrite(buffer, (size_t)32, (size_t)1, outf);

      ++j;
    }


    /* Print Reserve2 ------------------------------------------------------- */
    for (i = 0, j = 0; (j < FCELIB_MISC_Min(64, mesh->hdr.NumParts)) && (i < mesh->parts_len); ++i)
    {
      if (mesh->hdr.Parts[i] < 0)
        continue;

      part = mesh->parts[ mesh->hdr.Parts[i] ];
      for (n = 0; n < part->PNumVertices; ++n)
        fwrite(buffer, (size_t)12, (size_t)1, outf);

      ++j;
    }


    /* Print Reserve3 ------------------------------------------------------- */
    for (i = 0, j = 0; (j < FCELIB_MISC_Min(64, mesh->hdr.NumParts)) && (i < mesh->parts_len); ++i)
    {
      if (mesh->hdr.Parts[i] < 0)
        continue;

      part = mesh->parts[ mesh->hdr.Parts[i] ];
      for (n = 0; n < part->PNumVertices; ++n)
        fwrite(buffer, (size_t)12, (size_t)1, outf);

      ++j;
    }


    if (fclose(outf) != 0)
    {
      fprintf(stderr, "EncodeFce3: cannot close file '%s'\n", (char *)fcepath);
      retv = 0;
      break;
    }

    break;
  }/* for (;;) */

  free(global_mesh_to_local_fce_idxs);

  return retv;
}



/* Limited to 64 parts. Returns boolean. */
int FCELIB_IO_EncodeFce3(unsigned char **outbuf, const int outbuf_size, FcelibMesh *mesh, const int center_parts)
{
  int retv = 0;
  int i;
  int j;
  int n;
  int k;
  int sum_verts = 0;
  int sum_triags = 0;
  int *global_mesh_to_local_fce_idxs = NULL;
  FcelibPart *part;
  FcelibTriangle *triag;
  int buf;
  int tmp;
  
  for (;;)
  {
    if (!FCELIB_TYPES_ValidateMesh(*mesh))
      break;

    global_mesh_to_local_fce_idxs = (int *)malloc((size_t)mesh->vertices_len * sizeof(*global_mesh_to_local_fce_idxs));
    if (!global_mesh_to_local_fce_idxs)
    {
      fprintf(stderr, "EncodeFce3: Cannot allocate memory\n");
      break;
    }
  
    memset(*outbuf, 0, (size_t)outbuf_size * sizeof(**outbuf));

    /* Header --------------------------------------------------------------- */
    /* tmp = 0;
    memcpy(*buf + 0x0000, &tmp, (size_t)4); */
    memcpy(*outbuf + 0x0004, &mesh->hdr.NumTriangles, (size_t)4);
    memcpy(*outbuf + 0x0008, &mesh->hdr.NumVertices, (size_t)4);
    buf = 1;
    memcpy(*outbuf + 0x000C, &buf, (size_t)4);
    
//    buf = 0;
//    memcpy(*outbuf + 0x0010, &buf, (size_t)4);
    buf  = 12 * mesh->hdr.NumVertices;
    memcpy(*outbuf + 0x0014, &buf, (size_t)4);
    buf += 12 * mesh->hdr.NumVertices;
    memcpy(*outbuf + 0x0018, &buf, (size_t)4);

    buf += 56 * mesh->hdr.NumTriangles;
    memcpy(*outbuf + 0x001C, &buf, (size_t)4);
    buf += 32 * mesh->hdr.NumVertices;
    memcpy(*outbuf + 0x0020, &buf, (size_t)4);
    buf += 12 * mesh->hdr.NumVertices;
    memcpy(*outbuf + 0x0024, &buf, (size_t)4);
    
#if 0
    /* Center high body parts around local centroid (order idxs: 0-4, 12) */
    if (center_parts == 1)
    {
      tVector centroid;
      // i - internal part index, j - part order
      for (i = 0, j = 0; i < mesh->parts_len && j < FCELIB_MISC_Min(12, mesh->hdr.NumParts); ++i)
      {
        if (mesh->hdr.Parts[i] < 0 || (j > 4 && j != 12))
          continue;

        part = mesh->parts[ mesh->hdr.Parts[i] ];
        FCELIB_TYPES_GetPartLocalCentroid(mesh, part, &centroid);
        FCELIB_TYPES_ResetPartPos(mesh, part, centroid);

        ++j;
      }
    }
#endif
    /* Center parts around local centroid */
    if (center_parts == 1)
    {
      tVector centroid;
      // i - internal part index, j - part order
      for (i = 0, j = 0; i < mesh->parts_len && j < FCELIB_MISC_Min(12, mesh->hdr.NumParts); ++i)
      {
        if (mesh->hdr.Parts[i] < 0)
          continue;
        part = mesh->parts[ mesh->hdr.Parts[i] ];
        FCELIB_TYPES_GetPartLocalCentroid(mesh, part, &centroid);
        FCELIB_TYPES_ResetPartPos(mesh, part, centroid);
        ++j;
      }
    }
    
    /* Compute HalfSize from high body parts (order idxs: 0-4, 12) */
    {
      float *x_array = NULL;
      float *y_array;
      float *z_array;
      FcelibVertex *vert;
      int count_verts = 0;

      x_array = (float *)malloc((size_t)(3 * (mesh->vertices_len + 1)) * sizeof(*x_array));
      if (!x_array)
      {
        fprintf(stderr, "EncodeFce3: Cannot allocate memory\n");
        retv = 0;
        break;
      }
      memset(x_array, '\0', (size_t)(3 * (mesh->vertices_len + 1)) * sizeof(*x_array));
      y_array = x_array + mesh->vertices_len;
      z_array = y_array + mesh->vertices_len;

      // i - internal part index, j - part order
      for (i = 0, j = 0; i < mesh->parts_len && j < FCELIB_MISC_Min(12, mesh->hdr.NumParts); ++i)
      {
        if (mesh->hdr.Parts[i] < 0 || (j > 4 && j != 12))
          continue;

        part = mesh->parts[ mesh->hdr.Parts[i] ];

        // n - internal vert index, k - vert order
        for (n = 0, k = 0; n < part->pvertices_len && k < part->PNumVertices; ++n)
        {
          if (part->PVertices[n] < 0)
            continue;

          vert = mesh->vertices[ part->PVertices[n] ];
          x_array[count_verts + k] = vert->VertPos.x + part->PartPos.x;
          y_array[count_verts + k] = vert->VertPos.y + part->PartPos.y;
          z_array[count_verts + k] = vert->VertPos.z + part->PartPos.z;

          ++k;
        }
        count_verts += k - 1;

        ++j;
      }

      qsort(x_array, (size_t)count_verts, (size_t)4, FCELIB_MISC_CompareFloats);
      qsort(y_array, (size_t)count_verts, (size_t)4, FCELIB_MISC_CompareFloats);
      qsort(z_array, (size_t)count_verts, (size_t)4, FCELIB_MISC_CompareFloats);

      x_array[0] = 0.5f * abs(x_array[count_verts - 1] - x_array[0]);
      y_array[0] = abs(y_array[0]) - 0.02f;
      z_array[0] = 0.5f * abs(z_array[count_verts - 1] - z_array[0]);

      memcpy(*outbuf + 0x0028, x_array, (size_t)4);
      memcpy(*outbuf + 0x002C, y_array, (size_t)4);
      memcpy(*outbuf + 0x0030, z_array, (size_t)4);

      free(x_array);
    }  /* Set HalfSizes */

    /* Dummies */
    buf = FCELIB_MISC_Min(16, mesh->hdr.NumDummies);
    memcpy(*outbuf + 0x0034, &buf, (size_t)4);
    for (i = 0; i < FCELIB_MISC_Min(16, mesh->hdr.NumDummies); ++i)
    {
      memcpy(*outbuf + 0x0038 + i * 12 + 0, &mesh->hdr.Dummies[i].x, (size_t)4);
      memcpy(*outbuf + 0x0038 + i * 12 + 4, &mesh->hdr.Dummies[i].y, (size_t)4);
      memcpy(*outbuf + 0x0038 + i * 12 + 8, &mesh->hdr.Dummies[i].z, (size_t)4);
    }
    
    /* PartPos */
    /* P1stVertices */
    /* PNumVertices */
    /* P1stTriangles */
    /* PNumTriangles */
    /* PartNames */
    buf = FCELIB_MISC_Min(64, mesh->hdr.NumParts);
    memcpy(*outbuf + 0x00F8, &buf, (size_t)4);
    for (i = 0, j = 0; (i < mesh->parts_len) && (j < FCELIB_MISC_Min(64, mesh->hdr.NumParts)); ++i)
    {
      if (mesh->hdr.Parts[i] < 0)
        continue;
      part = mesh->parts[ mesh->hdr.Parts[i] ];
      
      memcpy(*outbuf + 0x00FC + j * 12 + 0, &part->PartPos.x, (size_t)4);
      memcpy(*outbuf + 0x00FC + j * 12 + 4, &part->PartPos.y, (size_t)4);
      memcpy(*outbuf + 0x00FC + j * 12 + 8, &part->PartPos.z, (size_t)4);
      
      memcpy(*outbuf + 0x03FC + j * 4, &sum_verts, (size_t)4);
      sum_verts += part->PNumVertices;
      memcpy(*outbuf + 0x04FC + j * 4, &part->PNumVertices, (size_t)4);
      
      memcpy(*outbuf + 0x05FC + j * 4, &sum_triags, (size_t)4);
      sum_triags += part->PNumTriangles;
      memcpy(*outbuf + 0x06FC + j * 4, &part->PNumTriangles, (size_t)4);
      
      memcpy(*outbuf + 0x0E04 + j * 64, &part->PartName, (size_t)64);
      
      ++j;
    }

    /* PriColors */
    buf = FCELIB_MISC_Min(16, mesh->hdr.NumColors);
    memcpy(*outbuf + 0x07FC, &buf, (size_t)4);
    for (i = 0; i < FCELIB_MISC_Min(16, mesh->hdr.NumColors); ++i)
    {
      memcpy(*outbuf + 0x0800 + i * 16 +  0, &mesh->hdr.PriColors[i].hue, (size_t)1);
      memcpy(*outbuf + 0x0800 + i * 16 +  4, &mesh->hdr.PriColors[i].saturation, (size_t)1);
      memcpy(*outbuf + 0x0800 + i * 16 +  8, &mesh->hdr.PriColors[i].brightness, (size_t)1);
      memcpy(*outbuf + 0x0800 + i * 16 + 12, &mesh->hdr.PriColors[i].transparency, (size_t)1);
    }

    /* SecColors */
    buf = FCELIB_MISC_Min(16, mesh->hdr.NumSecColors);
    memcpy(*outbuf + 0x0900, &buf, (size_t)4);
    for (i = 0; i < FCELIB_MISC_Min(16, mesh->hdr.NumSecColors); ++i)
    {
      memcpy(*outbuf + 0x0904 + i * 16 +  0, &mesh->hdr.SecColors[i].hue, (size_t)1);
      memcpy(*outbuf + 0x0904 + i * 16 +  4, &mesh->hdr.SecColors[i].saturation, (size_t)1);
      memcpy(*outbuf + 0x0904 + i * 16 +  8, &mesh->hdr.SecColors[i].brightness, (size_t)1);
      memcpy(*outbuf + 0x0904 + i * 16 + 12, &mesh->hdr.SecColors[i].transparency, (size_t)1);
    }
    
    /* DummyNames */
    memcpy(*outbuf + 0x0A04, &mesh->hdr.DummyNames, (size_t)(64 * 16));

    /* Print vertices ------------------------------------------------------- */
    /* VertTblOffset = 0 */
    sum_verts = 0;
    for (i = 0, j = 0; (i < mesh->parts_len) && (j < FCELIB_MISC_Min(64, mesh->hdr.NumParts)); ++i)
    {
      if (mesh->hdr.Parts[i] < 0)
        continue;
      part = mesh->parts[ mesh->hdr.Parts[i] ];
      for (n = 0, k = 0; (n < part->pvertices_len) && (k < part->PNumVertices); ++n)
      {
        if (part->PVertices[n] < 0)
          continue;
        
        memcpy(*outbuf + 0x1F04 + (sum_verts + k) * 12 + 0x00, &mesh->vertices[ part->PVertices[n] ]->VertPos.x, (size_t)4);
        memcpy(*outbuf + 0x1F04 + (sum_verts + k) * 12 + 0x04, &mesh->vertices[ part->PVertices[n] ]->VertPos.y, (size_t)4);
        memcpy(*outbuf + 0x1F04 + (sum_verts + k) * 12 + 0x08, &mesh->vertices[ part->PVertices[n] ]->VertPos.z, (size_t)4);
        ++k;
      }
      sum_verts += part->PNumVertices;
      ++j;
    }
    
    /* Print normals ------------------------------------------------------- */
    buf = 0x1F04 + 12 * mesh->hdr.NumVertices;  // NormTblOffset
    sum_verts = 0;
    for (i = 0, j = 0; i < mesh->parts_len && j < FCELIB_MISC_Min(64, mesh->hdr.NumParts); ++i)
    {
      if (mesh->hdr.Parts[i] < 0)
        continue;
      part = mesh->parts[ mesh->hdr.Parts[i] ];
      for (n = 0, k = 0; n < part->pvertices_len && k < part->PNumVertices; ++n)
      {
        if (part->PVertices[n] < 0)
          continue;
        memcpy(*outbuf + buf + (sum_verts + k) * 12 + 0x00, &mesh->vertices[ part->PVertices[n] ]->NormPos.x, (size_t)4);
        memcpy(*outbuf + buf + (sum_verts + k) * 12 + 0x04, &mesh->vertices[ part->PVertices[n] ]->NormPos.y, (size_t)4);
        memcpy(*outbuf + buf + (sum_verts + k) * 12 + 0x08, &mesh->vertices[ part->PVertices[n] ]->NormPos.z, (size_t)4);
        ++k;
      }
      sum_verts += part->PNumVertices;
      ++j;
    }
    
    /* Print triangles ------------------------------------------------------ */
    tmp = 0xff00;
    sum_triags = 0;
    buf += 12 * mesh->hdr.NumVertices;  // TriaTblOffset
    for (i = 0, j = 0; i < mesh->parts_len && j < FCELIB_MISC_Min(64, mesh->hdr.NumParts); ++i)
    {
      if (mesh->hdr.Parts[i] < 0)
        continue;
      part = mesh->parts[ mesh->hdr.Parts[i] ];

      /* Create map: global vert index to local part idx (of used-in-this-part verts) */
      memset(global_mesh_to_local_fce_idxs, -1, (size_t)mesh->vertices_len * sizeof(*global_mesh_to_local_fce_idxs));
      for (n = 0, k = 0; n < part->pvertices_len && k < part->PNumVertices; ++n)
      {
        if (part->PVertices[n] < 0)
          continue;
        global_mesh_to_local_fce_idxs[
          part->PVertices[n]
//        ] = n;
        ] = k;
        ++k;
      }

      for (n = 0, k = 0; n < part->ptriangles_len && k < part->PNumTriangles; ++n)
      {
        if (part->PTriangles[n] < 0)
          continue;
        triag = mesh->triangles[ part->PTriangles[n] ];
        memcpy(*outbuf + buf + (sum_triags + k) * 56 + 0x00, &triag->tex_page, (size_t)4);
        memcpy(*outbuf + buf + (sum_triags + k) * 56 + 0x04, &global_mesh_to_local_fce_idxs[ triag->vidx[0] ], (size_t)4);
        memcpy(*outbuf + buf + (sum_triags + k) * 56 + 0x08, &global_mesh_to_local_fce_idxs[ triag->vidx[1] ], (size_t)4);
        memcpy(*outbuf + buf + (sum_triags + k) * 56 + 0x0C, &global_mesh_to_local_fce_idxs[ triag->vidx[2] ], (size_t)4);
        memcpy(*outbuf + buf + (sum_triags + k) * 56 + 0x10 + 0x00, &tmp, (size_t)4);
        memcpy(*outbuf + buf + (sum_triags + k) * 56 + 0x10 + 0x04, &tmp, (size_t)4);
        memcpy(*outbuf + buf + (sum_triags + k) * 56 + 0x10 + 0x08, &tmp, (size_t)4);
        memcpy(*outbuf + buf + (sum_triags + k) * 56 + 0x1C, &triag->flag, (size_t)4);
        memcpy(*outbuf + buf + (sum_triags + k) * 56 + 0x20, &triag->U, (size_t)12);
        memcpy(*outbuf + buf + (sum_triags + k) * 56 + 0x2C, &triag->V, (size_t)12);
        ++k;
      }
      sum_triags += part->PNumTriangles;
      ++j;
    }

    retv = 1;
    break;
  }/* for (;;) */
  
  free(global_mesh_to_local_fce_idxs);

  return retv;
}


/* Limited to 64 parts, 16 dummies, 16 colors. Returns boolean. 
   For FCE4M, call with fce_version = 0x00101015 */
int FCELIB_IO_EncodeFce4(unsigned char **buf, const int buf_size, FcelibMesh *mesh, 
                         const int center_parts,
                         const int fce_version)
{
  int retv = 0;
  int *global_mesh_to_local_fce_idxs = NULL;
  int tmp;
  
  for (;;)
  {
    if (!FCELIB_TYPES_ValidateMesh(*mesh))
      break;

    global_mesh_to_local_fce_idxs = (int *)malloc((size_t)mesh->vertices_len * sizeof(*global_mesh_to_local_fce_idxs));
    if (!global_mesh_to_local_fce_idxs)
    {
      fprintf(stderr, "EncodeFce4: Cannot allocate memory\n");
      break;
    }
  
    memset(*buf, 0, buf_size * sizeof(unsigned char));
    
    if (fce_version == 0x00101015) 
      tmp = 0x00101015;
    else
      tmp = 0x00101014;
    memcpy(*buf + 0x0000, &tmp, (size_t)4);  // Version
    /* memcpy(*buf + 0x0003, &tmp, (size_t)4);  // Unknown1 */
    memcpy(*buf + 0x0008, &mesh->hdr.NumTriangles, (size_t)4);
    memcpy(*buf + 0x000C, &mesh->hdr.NumVertices, (size_t)4);
    tmp = 1;
    memcpy(*buf + 0x0010, &tmp, (size_t)4);  // NumArts
    
    retv = 1;
    break;
  }/* for (;;) */

  return retv;
}

#ifdef __cplusplus
}  /* extern "C" */
#endif

#ifdef __cplusplus
}  /* namespace fcelib */
#endif

#endif  /* FCELIB_IO_H */
