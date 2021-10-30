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
        }
        memcpy(&mesh->hdr.DummyNames, buf + 0x0a28, (size_t)(16 * 64));
        for (i = 0; i < 15; ++i)
          mesh->hdr.DummyNames[(i + 1) * 64 - 1] = '\0';  /* ensure string */

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

            // if (fce_version == 0x00101014)
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
        }
        memcpy(&mesh->hdr.DummyNames, buf + 0x0A04, (size_t)(16 * 64));
        for (i = 0; i < 15; ++i)
          mesh->hdr.DummyNames[(i + 1) * 64 - 1] = '\0';  /* ensure string */

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
    fprintf(stderr, "FCELIB_IO_EncodeFce3_Fopen: Cannot allocate memory\n");
    return 0;
  }

  for (;;)
  {
    outf = fopen((char *)fcepath, "wb");
    if (!outf)
    {
      fprintf(stderr, "FCELIB_IO_EncodeFce3_Fopen: cannot create file '%s'\n", (char *)fcepath);
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
        fprintf(stderr, "FCELIB_IO_EncodeFce3_Fopen: Cannot allocate memory\n");
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
      fprintf(stderr, "FCELIB_IO_EncodeFce3_Fopen: cannot close file '%s'\n", (char *)fcepath);
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
    {
      fprintf(stderr, "EncodeFce3: Invalid mesh\n");
      break;
    }

    {
      const int fsize = FCELIB_FCETYPES_Fce3ComputeSize(mesh->hdr.NumVertices, mesh->hdr.NumTriangles);
      if (outbuf_size < fsize)
      {
        fprintf(stderr, "EncodeFce3: Buffer too small (expects buf_size >= %d)\n", fsize);
        break;
      }
    }

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
int FCELIB_IO_EncodeFce4(unsigned char **outbuf, const int buf_size, FcelibMesh *mesh,
                         const int center_parts,
                         const int fce_version)
{
  int retv = 0;
  int *global_mesh_to_local_fce_idxs = NULL;
  int tmp;
  FcelibPart *part;
  FcelibTriangle *triag;
  int i;
  int j;
  int k;
  int n;
  int sum_verts = 0;
  int sum_triags = 0;

  if (fce_version == 0x00101015)
    fprintf(stdout, "Warning: EncodeFce4: Fce4M output is experimental, untested, unsupported\n");

  for (;;)
  {
    if (!FCELIB_TYPES_ValidateMesh(*mesh))
    {
      fprintf(stderr, "EncodeFce4: Invalid mesh\n");
      break;
    }

    {
      const int fsize = FCELIB_FCETYPES_Fce4ComputeSize(fce_version, mesh->hdr.NumVertices, mesh->hdr.NumTriangles);
      if (buf_size < fsize)
      {
        fprintf(stderr, "EncodeFce4: Buffer too small (expects buf_size >= %d)\n", fsize);
        break;
      }
    }

    global_mesh_to_local_fce_idxs = (int *)malloc((size_t)mesh->vertices_len * sizeof(*global_mesh_to_local_fce_idxs));
    if (!global_mesh_to_local_fce_idxs)
    {
      fprintf(stderr, "EncodeFce4: Cannot allocate memory\n");
      break;
    }

    memset(*outbuf, 0, (size_t)buf_size * sizeof(unsigned char));

    /* Header --------------------------------------------------------------- */
    if (fce_version == 0x00101015)
      tmp = 0x00101015;
    else
      tmp = 0x00101014;
    memcpy(*outbuf + 0x0000, &tmp, (size_t)4);  // Version
    /* memcpy(*outbuf + 0x0003, &tmp, (size_t)4);  // Unknown1 */
    memcpy(*outbuf + 0x0008, &mesh->hdr.NumTriangles, (size_t)4);
    memcpy(*outbuf + 0x000C, &mesh->hdr.NumVertices, (size_t)4);
    tmp = 1;
    memcpy(*outbuf + 0x0010, &tmp, (size_t)4);  // NumArts

    // tmp = 0;
    // memcpy(*outbuf + 0x0014, &tmp, (size_t)4);
    tmp  = 12 * mesh->hdr.NumVertices;
    memcpy(*outbuf + 0x0018, &tmp, (size_t)4);
    tmp += 12 * mesh->hdr.NumVertices;
    memcpy(*outbuf + 0x001C, &tmp, (size_t)4);

    tmp += 56 * mesh->hdr.NumTriangles;
    memcpy(*outbuf + 0x0020, &tmp, (size_t)4);
    tmp += 32 * mesh->hdr.NumVertices;
    memcpy(*outbuf + 0x0024, &tmp, (size_t)4);
    tmp += 12 * mesh->hdr.NumVertices;
    memcpy(*outbuf + 0x0028, &tmp, (size_t)4);

    tmp += 12 * mesh->hdr.NumVertices;
    memcpy(*outbuf + 0x002c, &tmp, (size_t)4);
    tmp += 12 * mesh->hdr.NumVertices;
    memcpy(*outbuf + 0x0030, &tmp, (size_t)4);
    tmp += 12 * mesh->hdr.NumVertices;
    memcpy(*outbuf + 0x0034, &tmp, (size_t)4);
    tmp += 12 * mesh->hdr.NumVertices;
    memcpy(*outbuf + 0x0038, &tmp, (size_t)4);

    tmp += 12 * mesh->hdr.NumVertices;
    memcpy(*outbuf + 0x003c, &tmp, (size_t)4);
    tmp += 4 * mesh->hdr.NumVertices;
    memcpy(*outbuf + 0x0040, &tmp, (size_t)4);
    tmp += 4 * mesh->hdr.NumVertices;
    memcpy(*outbuf + 0x0044, &tmp, (size_t)4);

    tmp += 4 * mesh->hdr.NumVertices;
    memcpy(*outbuf + 0x0048, &tmp, (size_t)4);

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

    /* Compute HalfSize from high body parts */
    {
      float *x_array = NULL;
      float *y_array;
      float *z_array;
      FcelibVertex *vert;
      int count_verts = 0;

      x_array = (float *)malloc((size_t)(3 * (mesh->vertices_len + 1)) * sizeof(*x_array));
      if (!x_array)
      {
        fprintf(stderr, "EncodeFce4: Cannot allocate memory\n");
        retv = 0;
        break;
      }
      memset(x_array, '\0', (size_t)(3 * (mesh->vertices_len + 1)) * sizeof(*x_array));
      y_array = x_array + mesh->vertices_len;
      z_array = y_array + mesh->vertices_len;

      // i - internal part index, j - part order
      for (i = 0, j = 0; i < mesh->parts_len && j < FCELIB_MISC_Min(12, mesh->hdr.NumParts); ++i)
      {
        if (mesh->hdr.Parts[i] < 0)
          continue;

        part = mesh->parts[ mesh->hdr.Parts[i] ];
        if (fce_version == 0x00101014)  // use all parts for Fce4M (experimental)
          if (!FCELIB_MISC_StrIsInArray(part->PartName, kFce4HiBodyParts))
            continue;

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

      memcpy(*outbuf + 0x004c, x_array, (size_t)4);
      memcpy(*outbuf + 0x0050, y_array, (size_t)4);
      memcpy(*outbuf + 0x0054, z_array, (size_t)4);

      free(x_array);
    }  /* Set HalfSizes */

    /* Dummies */
    tmp = FCELIB_MISC_Min(16, mesh->hdr.NumDummies);
    memcpy(*outbuf + 0x0058, &tmp, (size_t)4);
    for (i = 0; i < FCELIB_MISC_Min(16, mesh->hdr.NumDummies); ++i)
    {
      memcpy(*outbuf + 0x005c + i * 12 + 0, &mesh->hdr.Dummies[i].x, (size_t)4);
      memcpy(*outbuf + 0x005c + i * 12 + 4, &mesh->hdr.Dummies[i].y, (size_t)4);
      memcpy(*outbuf + 0x005c + i * 12 + 8, &mesh->hdr.Dummies[i].z, (size_t)4);
    }

    /* PartPos */
    /* P1stVertices */
    /* PNumVertices */
    /* P1stTriangles */
    /* PNumTriangles */
    /* PartNames */
    tmp = FCELIB_MISC_Min(64, mesh->hdr.NumParts);
    memcpy(*outbuf + 0x011c, &tmp, (size_t)4);
    for (i = 0, j = 0; (i < mesh->parts_len) && (j < FCELIB_MISC_Min(64, mesh->hdr.NumParts)); ++i)
    {
      if (mesh->hdr.Parts[i] < 0)
        continue;
      part = mesh->parts[ mesh->hdr.Parts[i] ];

      memcpy(*outbuf + 0x0120 + j * 12 + 0, &part->PartPos.x, (size_t)4);
      memcpy(*outbuf + 0x0120 + j * 12 + 4, &part->PartPos.y, (size_t)4);
      memcpy(*outbuf + 0x0120 + j * 12 + 8, &part->PartPos.z, (size_t)4);

      memcpy(*outbuf + 0x0420 + j * 4, &sum_verts, (size_t)4);
      sum_verts += part->PNumVertices;
      memcpy(*outbuf + 0x0520 + j * 4, &part->PNumVertices, (size_t)4);

      memcpy(*outbuf + 0x0620 + j * 4, &sum_triags, (size_t)4);
      sum_triags += part->PNumTriangles;
      memcpy(*outbuf + 0x0720 + j * 4, &part->PNumTriangles, (size_t)4);

      memcpy(*outbuf + 0x0e28 + j * 64, &part->PartName, (size_t)64);

      ++j;
    }

    /* NumColors */
    /* PriColors */
    /* IntColors */
    /* SecColors */
    /* DriColors */
    tmp = FCELIB_MISC_Min(16, mesh->hdr.NumColors);
    memcpy(*outbuf + 0x0820, &tmp, (size_t)4);
    for (i = 0; i < FCELIB_MISC_Min(16, mesh->hdr.NumColors); ++i)
    {
      memcpy(*outbuf + 0x0824 + i * 4 + 0, &mesh->hdr.PriColors[i].hue, (size_t)1);
      memcpy(*outbuf + 0x0824 + i * 4 + 1, &mesh->hdr.PriColors[i].saturation, (size_t)1);
      memcpy(*outbuf + 0x0824 + i * 4 + 2, &mesh->hdr.PriColors[i].brightness, (size_t)1);
      memcpy(*outbuf + 0x0824 + i * 4 + 3, &mesh->hdr.PriColors[i].transparency, (size_t)1);

      memcpy(*outbuf + 0x0864 + i * 4 + 0, &mesh->hdr.IntColors[i].hue, (size_t)1);
      memcpy(*outbuf + 0x0864 + i * 4 + 1, &mesh->hdr.IntColors[i].saturation, (size_t)1);
      memcpy(*outbuf + 0x0864 + i * 4 + 2, &mesh->hdr.IntColors[i].brightness, (size_t)1);
      memcpy(*outbuf + 0x0864 + i * 4 + 3, &mesh->hdr.IntColors[i].transparency, (size_t)1);

      memcpy(*outbuf + 0x08a4 + i * 4 + 0, &mesh->hdr.SecColors[i].hue, (size_t)1);
      memcpy(*outbuf + 0x08a4 + i * 4 + 1, &mesh->hdr.SecColors[i].saturation, (size_t)1);
      memcpy(*outbuf + 0x08a4 + i * 4 + 2, &mesh->hdr.SecColors[i].brightness, (size_t)1);
      memcpy(*outbuf + 0x08a4 + i * 4 + 3, &mesh->hdr.SecColors[i].transparency, (size_t)1);

      memcpy(*outbuf + 0x08e4 + i * 4 + 0, &mesh->hdr.DriColors[i].hue, (size_t)1);
      memcpy(*outbuf + 0x08e4 + i * 4 + 1, &mesh->hdr.DriColors[i].saturation, (size_t)1);
      memcpy(*outbuf + 0x08e4 + i * 4 + 2, &mesh->hdr.DriColors[i].brightness, (size_t)1);
      memcpy(*outbuf + 0x08e4 + i * 4 + 3, &mesh->hdr.DriColors[i].transparency, (size_t)1);
    }

    // if (fce_version == 0x00101015)
    // {
    //   tmp = 123;
    //   memcpy(*outbuf + 0x0924, &tmp, (size_t)4);  // Unknown3 (FCE4M: ?)
    // }

    /* DummyNames */
    memcpy(*outbuf + 0x0a28, &mesh->hdr.DummyNames, (size_t)(64 * 16));

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

        memcpy(*outbuf + 0x2038 + (sum_verts + k) * 12 + 0x00, &mesh->vertices[ part->PVertices[n] ]->VertPos.x, (size_t)4);
        memcpy(*outbuf + 0x2038 + (sum_verts + k) * 12 + 0x04, &mesh->vertices[ part->PVertices[n] ]->VertPos.y, (size_t)4);
        memcpy(*outbuf + 0x2038 + (sum_verts + k) * 12 + 0x08, &mesh->vertices[ part->PVertices[n] ]->VertPos.z, (size_t)4);
        ++k;
      }
      sum_verts += part->PNumVertices;
      ++j;
    }
    memcpy(&tmp, *outbuf + 0x002c, 4);  // UndamgdVertTblOffset
    memcpy(*outbuf + 0x2038 + tmp, *outbuf + 0x2038, (size_t)(12 * mesh->hdr.NumVertices));

    memcpy(&tmp, *outbuf + 0x0034, 4);  // DamgdVertTblOffset
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

        memcpy(*outbuf + 0x2038 + tmp + (sum_verts + k) * 12 + 0x00, &mesh->vertices[ part->PVertices[n] ]->DamgdVertPos.x, (size_t)4);
        memcpy(*outbuf + 0x2038 + tmp + (sum_verts + k) * 12 + 0x04, &mesh->vertices[ part->PVertices[n] ]->DamgdVertPos.y, (size_t)4);
        memcpy(*outbuf + 0x2038 + tmp + (sum_verts + k) * 12 + 0x08, &mesh->vertices[ part->PVertices[n] ]->DamgdVertPos.z, (size_t)4);
        ++k;
      }
      sum_verts += part->PNumVertices;
      ++j;
    }

    /* Print normals -------------------------------------------------------- */
    memcpy(&tmp, *outbuf + 0x0018, 4);  // NormTblOffset
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
        memcpy(*outbuf + 0x2038 + tmp + (sum_verts + k) * 12 + 0x00, &mesh->vertices[ part->PVertices[n] ]->NormPos.x, (size_t)4);
        memcpy(*outbuf + 0x2038 + tmp + (sum_verts + k) * 12 + 0x04, &mesh->vertices[ part->PVertices[n] ]->NormPos.y, (size_t)4);
        memcpy(*outbuf + 0x2038 + tmp + (sum_verts + k) * 12 + 0x08, &mesh->vertices[ part->PVertices[n] ]->NormPos.z, (size_t)4);
        ++k;
      }
      sum_verts += part->PNumVertices;
      ++j;
    }
    memcpy(&tmp, *outbuf + 0x0030, 4);  // UndamgdNormTblOffset
    memcpy(*outbuf + 0x2038 + tmp, *outbuf + 0x2038 + 12 * mesh->hdr.NumVertices, (size_t)(12 * mesh->hdr.NumVertices));

    memcpy(&tmp, *outbuf + 0x0038, 4);  // DamgdNormTblOffset
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
        memcpy(*outbuf + 0x2038 + tmp + (sum_verts + k) * 12 + 0x00, &mesh->vertices[ part->PVertices[n] ]->DamgdNormPos.x, (size_t)4);
        memcpy(*outbuf + 0x2038 + tmp + (sum_verts + k) * 12 + 0x04, &mesh->vertices[ part->PVertices[n] ]->DamgdNormPos.y, (size_t)4);
        memcpy(*outbuf + 0x2038 + tmp + (sum_verts + k) * 12 + 0x08, &mesh->vertices[ part->PVertices[n] ]->DamgdNormPos.z, (size_t)4);
        ++k;
      }
      sum_verts += part->PNumVertices;
      ++j;
    }

    /* Print animation ------------------------------------------------------ */
    memcpy(&tmp, *outbuf + 0x0040, 4);  // AnimationTblOffset
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
        memcpy(*outbuf + 0x2038 + tmp + (sum_verts + k) * 4 + 0x00, &mesh->vertices[ part->PVertices[n] ]->Animation, (size_t)4);
        ++k;
      }
      sum_verts += part->PNumVertices;
      ++j;
    }

    /* Print triangles ------------------------------------------------------ */
    sum_triags = 0;
    memcpy(&tmp, *outbuf + 0x001c, 4);  // TriaTblOffset
    {
      const int padding = 0xff00;
      float V_tmp[3];

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
          ] = k;
          ++k;
        }

        for (n = 0, k = 0; n < part->ptriangles_len && k < part->PNumTriangles; ++n)
        {
          if (part->PTriangles[n] < 0)
            continue;
          triag = mesh->triangles[ part->PTriangles[n] ];

          memcpy(&V_tmp, &triag->V, (size_t)12);
          // if (fce_version == 0x00101014)
          {
            for (int h = 0; h < 3; ++h)
              V_tmp[h] = 1 - V_tmp[h];
          }

          memcpy(*outbuf + 0x2038 + tmp + (sum_triags + k) * 56 + 0x00, &triag->tex_page, (size_t)4);
          memcpy(*outbuf + 0x2038 + tmp + (sum_triags + k) * 56 + 0x04, &global_mesh_to_local_fce_idxs[ triag->vidx[0] ], (size_t)4);
          memcpy(*outbuf + 0x2038 + tmp + (sum_triags + k) * 56 + 0x08, &global_mesh_to_local_fce_idxs[ triag->vidx[1] ], (size_t)4);
          memcpy(*outbuf + 0x2038 + tmp + (sum_triags + k) * 56 + 0x0C, &global_mesh_to_local_fce_idxs[ triag->vidx[2] ], (size_t)4);
          memcpy(*outbuf + 0x2038 + tmp + (sum_triags + k) * 56 + 0x10 + 0x00, &padding, (size_t)4);
          memcpy(*outbuf + 0x2038 + tmp + (sum_triags + k) * 56 + 0x10 + 0x04, &padding, (size_t)4);
          memcpy(*outbuf + 0x2038 + tmp + (sum_triags + k) * 56 + 0x10 + 0x08, &padding, (size_t)4);
          memcpy(*outbuf + 0x2038 + tmp + (sum_triags + k) * 56 + 0x1C, &triag->flag, (size_t)4);
          memcpy(*outbuf + 0x2038 + tmp + (sum_triags + k) * 56 + 0x20, &triag->U, (size_t)12);
          memcpy(*outbuf + 0x2038 + tmp + (sum_triags + k) * 56 + 0x2C, &V_tmp, (size_t)12);
          ++k;
        }
        sum_triags += part->PNumTriangles;
        ++j;
      }  /* for i, j */
    }

    retv = 1;
    break;
  }/* for (;;) */

  return retv;
}

int FCELIB_IO_GeomDataToNewPart(FcelibMesh *mesh,
                                int *vert_idxs, const int vert_idxs_len,  // N*3, N triags
                                // int *triangles_flags, const int triangles_flags_len,  // N
                                float *vert_texcoords, const int vert_texcoords_len,  // N*6
                                float *vert_pos, const int vert_pos_len,  // M*3, M verts
                                float *normals, const int normals_len)  // M*3
{
  int new_pid = -1;
  FcelibPart *part = NULL;
  FcelibTriangle *triag;
  FcelibVertex *vert;
  int i;
  int vidx_1st;
  int tidx_1st;

  for (;;)
  {
    if (!mesh)
    {
      fprintf(stderr, "GeomDataToNewPart: mesh is NULL\n");
      break;
    }

    if (!FCELIB_TYPES_ValidateMesh(*mesh))
    {
      fprintf(stderr, "GeomDataToNewPart: invalid mesh\n");
      break;
    }

    if (vert_idxs_len % 3 != 0)
    {
      fprintf(stderr, "GeomDataToNewPart: Expects N*3 == vert_idxs_len, for N triangles.\n");
      break;
    }
    if (vert_pos_len % 3 != 0)
    {
      fprintf(stderr, "GeomDataToNewPart: Expects N*3 == vert_pos_len, for N triangles.\n");
      break;
    }

    // if (vert_idxs_len != triangles_flags_len * 3)
    // {
    //   fprintf(stderr, "GeomDataToNewPart: Expects N*3 == vert_idxs_len == triangles_flags_len * 3, for N triangles.\n");
    //   break;
    // }
    if (vert_idxs_len * 2 != vert_texcoords_len)
    {
      fprintf(stderr, "GeomDataToNewPart: Expects N*3 == vert_idxs_len == vert_texcoords_len * 2 == N*6, for N triangles.\n");
      break;
    }
    if (vert_pos_len != normals_len)
    {
      fprintf(stderr, "GeomDataToNewPart: Expects N*3 == vert_pos_len == normals_len, for N triangles.\n");
      break;
    }

    if (FCELIB_MISC_ArrMax(vert_idxs, vert_idxs_len) >= int(vert_pos_len / 3))
    {
      fprintf(stderr, "GeomDataToNewPart: Triangle vertice index(es) out of range (assumes zero-indexed)\n");
      break;
    }

    // Lengthen part index map only if necessary
    if (mesh->parts_len < 1)
    {
      if (!FCELIB_TYPES_AddParts(mesh, 1))
      {
        fprintf(stderr, "GeomDataToNewPart: Cannot add part\n");
        break;
      }
    }
    else if (mesh->hdr.Parts[mesh->parts_len - 1] >= 0)
    {
      if (!FCELIB_TYPES_AddParts(mesh, 1))
      {
        fprintf(stderr, "GeomDataToNewPart: Cannot add part\n");
        break;
      }
    }
    /* Get first unused index */
    i = mesh->parts_len - 1;
    while (mesh->hdr.Parts[i] < 0 && i >= 0)
      --i;
    ++i;
    new_pid = i;

    if (new_pid > 0)
    {
      tidx_1st = FCELIB_TYPES_GetFirstUnusedGlobalTriangleIdx(mesh);
      vidx_1st = FCELIB_TYPES_GetFirstUnusedGlobalVertexIdx(mesh);
    }
    else
    {
      tidx_1st = 0;
      vidx_1st = 0;
    }

    /* Add part */
    mesh->hdr.Parts[new_pid] = FCELIB_MISC_ArrMax(mesh->hdr.Parts, mesh->parts_len) + 1;

    part = (FcelibPart *)malloc(sizeof(FcelibPart));
    if (!part)
    {
      fprintf(stderr, "GeomDataToNewPart: Cannot allocate memory (part)\n");
      new_pid = -1;
      break;
    }
    memset(part, 0, sizeof(FcelibPart));

    mesh->parts[ mesh->hdr.Parts[new_pid] ] = part;


    sprintf(part->PartName, "FromGeomData_%d", new_pid);  // unlikely to exceed 63 characters
    part->PartPos.x = 0.0f;
    part->PartPos.y = 0.0f;
    part->PartPos.z = 0.0f;

    part->PNumVertices = int(vert_pos_len / 3);
    part->pvertices_len = part->PNumVertices;

    part->PNumTriangles = int(vert_idxs_len / 3);
    part->ptriangles_len = part->PNumTriangles;


    part->PVertices = (int *)malloc((size_t)(part->pvertices_len * sizeof(int)));
    if (!part->PVertices)
    {
      fprintf(stderr, "GeomDataToNewPart: Cannot allocate memory (vert list)\n");
      new_pid = -1;
      break;
    }
    memset(part->PVertices, -1, (size_t)(part->pvertices_len * sizeof(int)));

    part->PTriangles = (int *)malloc((size_t)(part->ptriangles_len * sizeof(int)));
    if (!part->PTriangles)
    {
      fprintf(stderr, "GeomDataToNewPart: Cannot allocate memory (triag list\n");
      new_pid = -1;
      break;
    }
    memset(part->PTriangles, -1, (size_t)(part->ptriangles_len * sizeof(int)));

    ++mesh->hdr.NumParts;

    /* Add triangles */
    if (mesh->triangles_len < mesh->hdr.NumTriangles + part->ptriangles_len)
    {
      if (!FCELIB_TYPES_AddTriangles2(mesh, part, part->ptriangles_len))
      {
        fprintf(stderr, "GeomDataToNewPart: Cannot add triangles\n");
        new_pid = -1;
        break;
      }
    }

    for (int j = 0; j < part->PNumTriangles; ++j)
    {
      part->PTriangles[j] = tidx_1st + j;

      mesh->triangles[tidx_1st + j] = (FcelibTriangle *)malloc(sizeof(FcelibTriangle));
      triag = mesh->triangles[tidx_1st + j];
      if (!triag)
      {
        fprintf(stderr, "GeomDataToNewPart: Cannot allocate memory (triag)\n");
        new_pid = -1;
        break;
      }
      triag->tex_page = 0x0;  // default: textured
      triag->vidx[0] = vidx_1st + vert_idxs[j * 3 + 0];
      triag->vidx[1] = vidx_1st + vert_idxs[j * 3 + 1];
      triag->vidx[2] = vidx_1st + vert_idxs[j * 3 + 2];
      // triag->flag = triangles_flags[j];
      triag->flag = 0x000;  // default
      memcpy(triag->U, vert_texcoords + j * 6 + 0, (size_t)3 * sizeof(float));
      memcpy(triag->V, vert_texcoords + j * 6 + 3, (size_t)3 * sizeof(float));
    }
    if (new_pid < 0)
      break;
    mesh->hdr.NumTriangles += part->PNumTriangles;

    /* Add vertices */
    if (mesh->vertices_len < mesh->hdr.NumVertices + part->pvertices_len)
    {
      if (!FCELIB_TYPES_AddVertices2(mesh, part, part->pvertices_len))
      {
        fprintf(stderr, "GeomDataToNewPart: Cannot add vertices\n");
        new_pid = -1;
        break;
      }
    }

    for (int j = 0; j < part->PNumVertices; ++j)
    {
      part->PVertices[j] = vidx_1st + j;

      mesh->vertices[vidx_1st + j] = (FcelibVertex *)malloc(sizeof(FcelibVertex));
      vert = mesh->vertices[vidx_1st + j];
      if (!vert)
      {
        fprintf(stderr, "GeomDataToNewPart: Cannot allocate memory (vert)\n");
        new_pid = -1;
        break;
      }
      memcpy(&vert->VertPos.x, vert_pos + j * 3 + 0, sizeof(float));
      memcpy(&vert->VertPos.y, vert_pos + j * 3 + 1, sizeof(float));
      memcpy(&vert->VertPos.z, vert_pos + j * 3 + 2, sizeof(float));
      memcpy(&vert->DamgdVertPos.x, vert_pos + j * 3 + 0, sizeof(float));
      memcpy(&vert->DamgdVertPos.y, vert_pos + j * 3 + 1, sizeof(float));
      memcpy(&vert->DamgdVertPos.z, vert_pos + j * 3 + 2, sizeof(float));
      memcpy(&vert->NormPos.x, normals + j * 3 + 0, sizeof(float));
      memcpy(&vert->NormPos.y, normals + j * 3 + 1, sizeof(float));
      memcpy(&vert->NormPos.z, normals + j * 3 + 2, sizeof(float));
      memcpy(&vert->DamgdNormPos.x, normals + j * 3 + 0, sizeof(float));
      memcpy(&vert->DamgdNormPos.y, normals + j * 3 + 1, sizeof(float));
      memcpy(&vert->DamgdNormPos.z, normals + j * 3 + 2, sizeof(float));
      vert->Animation = 0x0;  // default: not immovable
    }
    if (new_pid < 0)
      break;
    mesh->hdr.NumVertices += part->PNumVertices;

    new_pid = FCELIB_TYPES_GetOrderByInternalPartIdx(mesh, mesh->hdr.Parts[new_pid]);
    if (new_pid < 0)
    {
      fprintf(stderr, "GeomDataToNewPart: Cannot get new_pid\n");
      break;
    }

    break;
  }

  return new_pid;
}

#ifdef __cplusplus
}  /* extern "C" */
#endif

#ifdef __cplusplus
}  /* namespace fcelib */
#endif

#endif  /* FCELIB_IO_H */