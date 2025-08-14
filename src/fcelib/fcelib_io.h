/*
  fcelib_io.h
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
  import/export FCE3, FCE4, FCE4M, export OBJ/MTL, import geometric data
**/

#ifndef FCELIB_IO_H_
#define FCELIB_IO_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./fcelib_fcetypes.h"
#include "./fcelib_types.h"
#include "./fcelib_util.h"

/* decode formats ----------------------------------------------------------- */

/* From no parts follows, no triangles or vertices. */
void __FCELIB_IO_DECODE_HASPARTS(int *retv, FcelibMesh *mesh)
{
  if (mesh->parts_len == 0)  /** TODO: test */
  {
    mesh->triangles_len = 0;
    mesh->vertices_len = 0;
    *retv = 1;
  }
}

/* From no vertices follows, no triangles. */
void __FCELIB_IO_DECODE_HASVERTICES(int *retv, FcelibMesh *mesh)
{
  if (mesh->vertices_len == 0)
  {
    mesh->triangles_len = 0;
    *retv = 1;
  }
}

/*
  Returns 1 on success, 0 on failure. Same for FCE3, FCE4, FCE4M.
  Not bounds-checked.
*/
int __FCELIB_IO_DECODE_GETPARTS(FcelibMesh *mesh, const char *header_PartNames, const float *header_PartPos, const int *header_PNumVertices, const int *header_PNumTriangles)
{
  int retv = 0;
  int i;

  for (;;)
  {
    mesh->hdr.Parts = (int *)malloc(mesh->parts_len * sizeof(*mesh->hdr.Parts));
    if (!mesh->hdr.Parts)
    {
      fprintf(stderr, "DecodeFce: Cannot allocate memory\n");
      break;
    }
    for (i = 0; i < mesh->hdr.NumParts; ++i)
      mesh->hdr.Parts[i] = i;

    /* Parts ------------------------------------------------------------ */
    mesh->parts = (FcelibPart **)malloc(mesh->parts_len * sizeof(*mesh->parts));
    if (!mesh->parts)
    {
      fprintf(stderr, "DecodeFce: Cannot allocate memory\n");
      break;
    }
    memset(mesh->parts, 0, mesh->parts_len * sizeof(*mesh->parts));

    for (i = 0; i < mesh->hdr.NumParts; ++i)
    {
      mesh->parts[i] = (FcelibPart *)malloc(sizeof(**mesh->parts));
      if (!mesh->parts[i])
      {
        fprintf(stderr, "DecodeFce: Cannot allocate memory\n");
        break;
      }

      memcpy(mesh->parts[i]->PartName, header_PartNames + i * 64, 64);

      memcpy(&mesh->parts[i]->PartPos.x, header_PartPos + i * 3 + 0, 4);
      memcpy(&mesh->parts[i]->PartPos.y, header_PartPos + i * 3 + 1, 4);
      memcpy(&mesh->parts[i]->PartPos.z, header_PartPos + i * 3 + 2, 4);

      mesh->parts[i]->PNumVertices = header_PNumVertices[i];
      mesh->parts[i]->pvertices_len = mesh->parts[i]->PNumVertices;
      mesh->parts[i]->PVertices = NULL;

      mesh->parts[i]->PNumTriangles = header_PNumTriangles[i];
      mesh->parts[i]->ptriangles_len = mesh->parts[i]->PNumTriangles;
      mesh->parts[i]->PTriangles = NULL;

      /* update global counts */
      mesh->vertices_len += mesh->parts[i]->pvertices_len;
      mesh->triangles_len += mesh->parts[i]->ptriangles_len;

      mesh->parts[i]->PVertices = (int *)malloc(mesh->parts[i]->pvertices_len * sizeof(*mesh->parts[i]->PVertices));
      if (!mesh->parts[i]->PVertices)
      {
        fprintf(stderr, "DecodeFce: Cannot allocate memory\n");
        break;
      }
      memset(mesh->parts[i]->PVertices, 0xFF, mesh->parts[i]->pvertices_len * sizeof(*mesh->parts[i]->PVertices));

      mesh->parts[i]->PTriangles = (int *)malloc(mesh->parts[i]->ptriangles_len * sizeof(*mesh->parts[i]->PTriangles));
      if (!mesh->parts[i]->PTriangles)
      {
        fprintf(stderr, "DecodeFce: Cannot allocate memory\n");
        break;
      }
      memset(mesh->parts[i]->PTriangles, 0xFF, mesh->parts[i]->ptriangles_len * sizeof(*mesh->parts[i]->PTriangles));
    }  /* for i */

    retv = 1;
    break;
  }

  return retv;
}

/*
  Params: FCE buffer, FcelibMesh. Returns bool.
  Assumes (mesh != NULL). Silently releases and re-initializes existing mesh.
  Assumes valid FCE data.
*/
int FCELIB_IO_DecodeFce(FcelibMesh *mesh, const void *inbuf_, int inbufsz)
{
  int retv = 0;
  int i;
  int j;
  int n;
  int fce_version;
  const unsigned char *inbuf = (const unsigned char *)inbuf_;

  for (;;)
  {
    if (!inbuf || inbufsz <= 0)
    {
      fprintf(stderr, "DecodeFce: inbuf is NULL\n");
      break;
    }

    if (mesh->release == &FCELIB_TYPES_MeshRelease)
    {
      mesh->release(mesh);
      FCELIB_TYPES_MeshInit(mesh);
    }
#ifndef FCELIB_PYTHON_BINDINGS
    else if (!mesh->release || mesh->release != &FCELIB_TYPES_MeshRelease)
    {
      FCELIB_TYPES_MeshInit(mesh);
    }
#endif

    if (inbufsz < 0x1F04)
    {
      fprintf(stderr, "DecodeFce: Format error: header too small\n");
      break;
    }

    memcpy(&fce_version, inbuf, 4);

    switch (fce_version)
    {
      case 0x00101014: case 0x00101015:
      {
        const int kHdrSize = 0x2038;
        FceHeader4 hdr;

        if (inbufsz < kHdrSize)
        {
          fprintf(stderr, "DecodeFce: Format error: header too small\n");
          break;
        }

        FCELIB_FCETYPES_GetFceHeader4(&hdr, inbuf);

        if (!FCELIB_FCETYPES_Fce4ValidateHeader(&hdr, inbuf, inbufsz, 1))
          return 0;

        /* Header ----------------------------------------------------------- */
          /* NumTriangles - counted below */
          /* NumVertices - counted below */
        mesh->hdr.NumArts = hdr.NumArts;
        if (fce_version == 0x00101015)
          mesh->hdr.Unknown3 = hdr.Unknown3;  /* FCE4M experimental */
        mesh->hdr.NumParts = SCL_min(hdr.NumParts, 64);
        mesh->parts_len = mesh->hdr.NumParts;

        mesh->hdr.NumDummies = SCL_clamp(hdr.NumDummies, 0, 16);
        for (i = 0; i < mesh->hdr.NumDummies; ++i)
        {
          memcpy(&mesh->hdr.Dummies[i].x, inbuf + 0x005c + i * 12 + 0x0, 4);
          memcpy(&mesh->hdr.Dummies[i].y, inbuf + 0x005c + i * 12 + 0x4, 4);
          memcpy(&mesh->hdr.Dummies[i].z, inbuf + 0x005c + i * 12 + 0x8, 4);
        }
        memcpy(&mesh->hdr.DummyNames, hdr.DummyNames, 16 * 64);

        mesh->hdr.NumColors = SCL_clamp(hdr.NumColors, 0, 16);
        mesh->hdr.NumSecColors = mesh->hdr.NumColors;
        memcpy(&mesh->hdr.PriColors, hdr.PriColors, 16 * sizeof(tColor4));
        memcpy(&mesh->hdr.IntColors, hdr.IntColors, 16 * sizeof(tColor4));
        memcpy(&mesh->hdr.SecColors, hdr.SecColors, 16 * sizeof(tColor4));
        memcpy(&mesh->hdr.DriColors, hdr.DriColors, 16 * sizeof(tColor4));

        /* Parts ------------------------------------------------------------ */
        __FCELIB_IO_DECODE_HASPARTS(&retv, mesh);
        if (retv == 1)
          break;
        if (!__FCELIB_IO_DECODE_GETPARTS(mesh, hdr.PartNames, hdr.PartPos, hdr.PNumVertices, hdr.PNumTriangles))
          break;

        /* Triangles -------------------------------------------------------- */
        __FCELIB_IO_DECODE_HASVERTICES(&retv, mesh);
        if (retv == 1)
          break;
        if (mesh->triangles_len == 0)  /* TODO: to allow vertices w/o triangles, first decode the vertices */
        {
          retv = 1;
          break;
        }

        mesh->triangles = (FcelibTriangle **)malloc(mesh->triangles_len * sizeof(*mesh->triangles));
        if (!mesh->triangles)
        {
          fprintf(stderr, "DecodeFce: Cannot allocate memory\n");
          break;
        }
        memset(mesh->triangles, 0, mesh->triangles_len * sizeof(*mesh->triangles));

        mesh->hdr.NumTriangles = 0;
        mesh->hdr.NumVertices = 0;

        for (i = 0; i < mesh->hdr.NumParts; ++i)
        {
          for (j = 0; j < mesh->parts[i]->PNumTriangles; ++j)
          {
            mesh->parts[i]->PTriangles[j] = mesh->hdr.NumTriangles;

            mesh->triangles[mesh->hdr.NumTriangles] = (FcelibTriangle *)malloc(sizeof(**mesh->triangles));
            if (!mesh->triangles[mesh->hdr.NumTriangles])
            {
              fprintf(stderr, "DecodeFce: Cannot allocate memory\n");
              break;
            }

            memcpy(&mesh->triangles[mesh->hdr.NumTriangles]->tex_page, inbuf + kHdrSize + hdr.TriaTblOffset + (j + hdr.P1stTriangles[i]) * 56 + 0x00, 4);
            memcpy(&mesh->triangles[mesh->hdr.NumTriangles]->vidx,     inbuf + kHdrSize + hdr.TriaTblOffset + (j + hdr.P1stTriangles[i]) * 56 + 0x04, 12);

            /* Globalize vert index references */
            for (n = 0; n < 3; ++n)
              mesh->triangles[mesh->hdr.NumTriangles]->vidx[n] += mesh->hdr.NumVertices;

            memcpy(&mesh->triangles[mesh->hdr.NumTriangles]->flag, inbuf + kHdrSize + hdr.TriaTblOffset + (j + hdr.P1stTriangles[i]) * 56 + 0x1C, 4);
            memcpy(&mesh->triangles[mesh->hdr.NumTriangles]->U,    inbuf + kHdrSize + hdr.TriaTblOffset + (j + hdr.P1stTriangles[i]) * 56 + 0x20, 12);
            memcpy(&mesh->triangles[mesh->hdr.NumTriangles]->V,    inbuf + kHdrSize + hdr.TriaTblOffset + (j + hdr.P1stTriangles[i]) * 56 + 0x2C, 12);

            /* if (fce_version == 0x00101014) */
            {
              for (n = 0; n < 3; ++n)
                mesh->triangles[mesh->hdr.NumTriangles]->V[n] = 1 - mesh->triangles[mesh->hdr.NumTriangles]->V[n];
            }

            ++mesh->hdr.NumTriangles;
          }

          mesh->hdr.NumVertices += mesh->parts[i]->PNumVertices;
        }

        /* Vertices --------------------------------------------------------- */
        /* We already know that (mesh->vertices_len > 0) */
        mesh->vertices = (FcelibVertex **)malloc(mesh->vertices_len * sizeof(*mesh->vertices));
        if (!mesh->vertices)
        {
          fprintf(stderr, "DecodeFce: Cannot allocate memory\n");
          break;
        }
        memset(mesh->vertices, 0, mesh->vertices_len * sizeof(*mesh->vertices));

        mesh->hdr.NumVertices = 0;

        for (i = 0; i < mesh->hdr.NumParts; ++i)
        {
          /* Get vertices by global fce idx */
          for (j = 0; j < mesh->parts[i]->PNumVertices; ++j)
          {
            mesh->parts[i]->PVertices[j] = mesh->hdr.NumVertices;

            mesh->vertices[mesh->hdr.NumVertices] = (FcelibVertex *)malloc(sizeof(**mesh->vertices));
            if (!mesh->vertices[mesh->hdr.NumVertices])
            {
              fprintf(stderr, "DecodeFce: Cannot allocate memory\n");
              break;
            }

            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->VertPos.x, inbuf + kHdrSize + hdr.VertTblOffset + (j + hdr.P1stVertices[i]) * 12 + 0x0, 4);
            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->VertPos.y, inbuf + kHdrSize + hdr.VertTblOffset + (j + hdr.P1stVertices[i]) * 12 + 0x4, 4);
            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->VertPos.z, inbuf + kHdrSize + hdr.VertTblOffset + (j + hdr.P1stVertices[i]) * 12 + 0x8, 4);

            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->NormPos.x, inbuf + kHdrSize + hdr.NormTblOffset + (j + hdr.P1stVertices[i]) * 12 + 0x0, 4);
            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->NormPos.y, inbuf + kHdrSize + hdr.NormTblOffset + (j + hdr.P1stVertices[i]) * 12 + 0x4, 4);
            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->NormPos.z, inbuf + kHdrSize + hdr.NormTblOffset + (j + hdr.P1stVertices[i]) * 12 + 0x8, 4);

            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->DamgdVertPos.x, inbuf + kHdrSize + hdr.DamgdVertTblOffset + (j + hdr.P1stVertices[i]) * 12 + 0x0, 4);
            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->DamgdVertPos.y, inbuf + kHdrSize + hdr.DamgdVertTblOffset + (j + hdr.P1stVertices[i]) * 12 + 0x4, 4);
            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->DamgdVertPos.z, inbuf + kHdrSize + hdr.DamgdVertTblOffset + (j + hdr.P1stVertices[i]) * 12 + 0x8, 4);

            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->DamgdNormPos.x, inbuf + kHdrSize + hdr.DamgdNormTblOffset + (j + hdr.P1stVertices[i]) * 12 + 0x0, 4);
            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->DamgdNormPos.y, inbuf + kHdrSize + hdr.DamgdNormTblOffset + (j + hdr.P1stVertices[i]) * 12 + 0x4, 4);
            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->DamgdNormPos.z, inbuf + kHdrSize + hdr.DamgdNormTblOffset + (j + hdr.P1stVertices[i]) * 12 + 0x8, 4);

            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->Animation, inbuf + kHdrSize + hdr.AnimationTblOffset + (j + hdr.P1stVertices[i]) * 4, 4);

            ++mesh->hdr.NumVertices;
          }
        }
      }  /* case FCE4, FCEM */

      retv = 1;
      break;

      default:  /* FCE3 */
      {
        const int kHdrSize = 0x1F04;
        FceHeader3 hdr;
        FCELIB_FCETYPES_GetFceHeader3(&hdr, inbuf);

        if (!FCELIB_FCETYPES_Fce3ValidateHeader(&hdr, inbuf, inbufsz, 1))
          return 0;

        /* Header ----------------------------------------------------------- */
          /* NumTriangles - counted below */
          /* NumVertices - counted below */
        mesh->hdr.NumArts = hdr.NumArts;
        mesh->hdr.NumParts = hdr.NumParts;
        mesh->parts_len = mesh->hdr.NumParts;

        mesh->hdr.NumDummies = SCL_clamp(hdr.NumDummies, 0, 16);
        for (i = 0; i < mesh->hdr.NumDummies; ++i)
        {
          memcpy(&mesh->hdr.Dummies[i].x, inbuf + 0x0038 + i * 12 + 0x0, 4);
          memcpy(&mesh->hdr.Dummies[i].y, inbuf + 0x0038 + i * 12 + 0x4, 4);
          memcpy(&mesh->hdr.Dummies[i].z, inbuf + 0x0038 + i * 12 + 0x8, 4);
        }
        memcpy(&mesh->hdr.DummyNames, hdr.DummyNames, 16 * 64);

        mesh->hdr.NumColors = SCL_clamp(hdr.NumPriColors, 0, 16);
        FCELIB_TYPES_SetFceColors(mesh->hdr.PriColors, mesh->hdr.NumColors, inbuf + 0x0800, 4);  /* unsigned char from little-endian int */
        memcpy(&mesh->hdr.DriColors, &mesh->hdr.PriColors, sizeof(mesh->hdr.PriColors));

        mesh->hdr.NumSecColors = SCL_clamp(hdr.NumSecColors, 0, 16);
        FCELIB_TYPES_SetFceColors(mesh->hdr.SecColors, mesh->hdr.NumSecColors, inbuf + 0x0904, 4);  /* unsigned char from little-endian int */
        memcpy(&mesh->hdr.IntColors, &mesh->hdr.SecColors, sizeof(mesh->hdr.SecColors));

        /* Parts ------------------------------------------------------------ */
        __FCELIB_IO_DECODE_HASPARTS(&retv, mesh);
        if (retv == 1)
          break;
        if (!__FCELIB_IO_DECODE_GETPARTS(mesh, hdr.PartNames, hdr.PartPos, hdr.PNumVertices, hdr.PNumTriangles))
          break;

        /* Triangles -------------------------------------------------------- */
        __FCELIB_IO_DECODE_HASVERTICES(&retv, mesh);
        if (retv == 1)
          break;
        if (mesh->triangles_len == 0)  /* Allow vertices w/o triangles */
        {
          retv = 1;
          break;
        }

        mesh->triangles = (FcelibTriangle **)malloc(mesh->triangles_len * sizeof(*mesh->triangles));
        if (!mesh->triangles)
        {
          fprintf(stderr, "DecodeFce: Cannot allocate memory\n");
          break;
        }
        memset(mesh->triangles, 0, mesh->triangles_len * sizeof(*mesh->triangles));

        mesh->hdr.NumTriangles = 0;
        mesh->hdr.NumVertices = 0;

        for (i = 0; i < mesh->hdr.NumParts; ++i)
        {
          for (j = 0; j < mesh->parts[i]->PNumTriangles; ++j)
          {
            mesh->parts[i]->PTriangles[j] = mesh->hdr.NumTriangles;

            mesh->triangles[mesh->hdr.NumTriangles] = (FcelibTriangle *)malloc(sizeof(**mesh->triangles));
            if (!mesh->triangles[mesh->hdr.NumTriangles])
            {
              fprintf(stderr, "DecodeFce: Cannot allocate memory\n");
              break;
            }

            memcpy(&mesh->triangles[mesh->hdr.NumTriangles]->tex_page, inbuf + kHdrSize + hdr.TriaTblOffset + (j + hdr.P1stTriangles[i]) * 56 + 0x00, 4);
            memcpy(&mesh->triangles[mesh->hdr.NumTriangles]->vidx,     inbuf + kHdrSize + hdr.TriaTblOffset + (j + hdr.P1stTriangles[i]) * 56 + 0x04, 12);

            /* Globalize vert index references */
            for (n = 0; n < 3; ++n)
              mesh->triangles[mesh->hdr.NumTriangles]->vidx[n] += mesh->hdr.NumVertices;

            memcpy(&mesh->triangles[mesh->hdr.NumTriangles]->flag, inbuf + kHdrSize + hdr.TriaTblOffset + (j + hdr.P1stTriangles[i]) * 56 + 0x1C, 4);
            memcpy(&mesh->triangles[mesh->hdr.NumTriangles]->U,    inbuf + kHdrSize + hdr.TriaTblOffset + (j + hdr.P1stTriangles[i]) * 56 + 0x20, 12);
            memcpy(&mesh->triangles[mesh->hdr.NumTriangles]->V,    inbuf + kHdrSize + hdr.TriaTblOffset + (j + hdr.P1stTriangles[i]) * 56 + 0x2C, 12);

            ++mesh->hdr.NumTriangles;
          }

          mesh->hdr.NumVertices += mesh->parts[i]->PNumVertices;
        }

        /* Vertices --------------------------------------------------------- */
        /* We already know that (mesh->vertices_len > 0) */
        mesh->vertices = (FcelibVertex **)malloc(mesh->vertices_len * sizeof(*mesh->vertices));
        if (!mesh->vertices)
        {
          fprintf(stderr, "DecodeFce: Cannot allocate memory\n");
          break;
        }
        memset(mesh->vertices, 0, mesh->vertices_len * sizeof(*mesh->vertices));

        mesh->hdr.NumVertices = 0;

        for (i = 0; i < mesh->hdr.NumParts; ++i)
        {
          /* Get vertices by global fce idx */
          for (j = 0; j < mesh->parts[i]->PNumVertices; ++j)
          {
            mesh->parts[i]->PVertices[j] = mesh->hdr.NumVertices;

            mesh->vertices[mesh->hdr.NumVertices] = (FcelibVertex *)malloc(sizeof(**mesh->vertices));
            if (!mesh->vertices[mesh->hdr.NumVertices])
            {
              fprintf(stderr, "DecodeFce: Cannot allocate memory\n");
              break;
            }

            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->VertPos.x, inbuf + kHdrSize + hdr.VertTblOffset + (j + hdr.P1stVertices[i]) * 12 + 0x0, 4);
            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->VertPos.y, inbuf + kHdrSize + hdr.VertTblOffset + (j + hdr.P1stVertices[i]) * 12 + 0x4, 4);
            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->VertPos.z, inbuf + kHdrSize + hdr.VertTblOffset + (j + hdr.P1stVertices[i]) * 12 + 0x8, 4);

            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->NormPos.x, inbuf + kHdrSize + hdr.NormTblOffset + (j + hdr.P1stVertices[i]) * 12 + 0x0, 4);
            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->NormPos.y, inbuf + kHdrSize + hdr.NormTblOffset + (j + hdr.P1stVertices[i]) * 12 + 0x4, 4);
            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->NormPos.z, inbuf + kHdrSize + hdr.NormTblOffset + (j + hdr.P1stVertices[i]) * 12 + 0x8, 4);

            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->DamgdVertPos.x, inbuf + kHdrSize + hdr.VertTblOffset + (j + hdr.P1stVertices[i]) * 12 + 0x0, 4);
            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->DamgdVertPos.y, inbuf + kHdrSize + hdr.VertTblOffset + (j + hdr.P1stVertices[i]) * 12 + 0x4, 4);
            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->DamgdVertPos.z, inbuf + kHdrSize + hdr.VertTblOffset + (j + hdr.P1stVertices[i]) * 12 + 0x8, 4);

            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->DamgdNormPos.x, inbuf + kHdrSize + hdr.NormTblOffset + (j + hdr.P1stVertices[i]) * 12 + 0x0, 4);
            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->DamgdNormPos.y, inbuf + kHdrSize + hdr.NormTblOffset + (j + hdr.P1stVertices[i]) * 12 + 0x4, 4);
            memcpy(&mesh->vertices[mesh->hdr.NumVertices]->DamgdNormPos.z, inbuf + kHdrSize + hdr.NormTblOffset + (j + hdr.P1stVertices[i]) * 12 + 0x8, 4);

            mesh->vertices[mesh->hdr.NumVertices]->Animation = 0x0;

            ++mesh->hdr.NumVertices;
          }
        }
      }  /* case default */

      /* Tidy up names */
      for (i = 0; i < mesh->hdr.NumDummies; i++)
      {
        n = strlen(mesh->hdr.DummyNames + i * 64);
        memset(mesh->hdr.DummyNames + i * 64 + n, '\0', 64 - n);
      }
      memset(mesh->hdr.DummyNames + mesh->hdr.NumDummies * 64, '\0', (16 - mesh->hdr.NumDummies) * 64);
      for (i = 0; i < mesh->hdr.NumParts; ++i)
      {
        n = strlen(mesh->parts[i]->PartName);
        memset(mesh->parts[i]->PartName + n, '\0', 64 - n);
      }

      retv = 1;
      break;
    }

    break;
  }  /* for (;;) */

  if (retv != 1)
    mesh->release(mesh);

  return retv;
}

/* encode ------------------------------------------------------------------- */

/*
  FCE triangle flags are written to material names. Returns boolean.
  Assumes *objpath, *mtlpath, and *texture_name are strings.
*/
int FCELIB_IO_ExportObj(const FcelibMesh *mesh,
                        const char *objpath, const char *mtlpath,
                        const char *texture_name,
                        int print_damage, int print_dummies,
                        int use_part_positions,
                        int print_part_positions,
                        int filter_triagflags_0xfff)
{
  int retv = 1;
  int i;
  int j;
  int n;
  int k;
  FILE *outf = NULL;
  int sum_verts = 0;
  int sum_triags = 0;
  int *global_mesh_to_global_obj_idxs;
  FcelibPart *part;
  FcelibTriangle *triag;

  global_mesh_to_global_obj_idxs = (int *)malloc(mesh->vertices_len * sizeof(*global_mesh_to_global_obj_idxs));
  if (!global_mesh_to_global_obj_idxs)
  {
    fprintf(stderr, "ExportObj: Cannot allocate memory\n");
    return 0;
  }

  for (;;)
  {
    /* Print mtl (used triangle 12-bit flags as materials) ------------------ */
    {
      char mtls[4096] = {0};
      int count_mtls = 0;

      for (i = 0; i < mesh->triangles_len; ++i)
      {
        if (mesh->triangles[i])
        {
          if (mtls[mesh->triangles[i]->flag & 0xFFF] != '1')
          {
            mtls[mesh->triangles[i]->flag & 0xFFF] = '1';
            ++count_mtls;
          }
        }
      }

      outf = fopen(mtlpath, "wb");
      if (!outf)
      {
        fprintf(stderr, "ExportObj: cannot create file '%s'\n", mtlpath);
        retv = 0;
        break;
      }

      fprintf(outf,
              "# fcecodec MTL File: '%s'\n"
              "# Material Count: %d\n",
              FCELIB_UTIL_GetFileName(objpath), count_mtls);

      for (i = 0; i < 4096; ++i)
      {
        if (mtls[i] == '1')
        {
          fprintf(outf,
                  "\n"
                  "newmtl 0x%03x\n"
                  "Ka 1.000 1.000 1.000\n"
                  "Kd 1.000 1.000 1.000\n"
                  "Ks 0.000 0.000 0.000\n"
                  "d 0.7\n"
                  /* "Tr 0.3\n" */
                  "illum 2\n"
                  "map_Kd %s\n",
                  i, texture_name);
        }
      }
      fflush(outf);

      if (fclose(outf) != 0)
      {
        fprintf(stderr, "ExportObj: cannot close file '%s'\n", mtlpath);
        retv = 0;
        break;
      }
      outf = NULL;
    }

    /* Print obj ------------------------------------------------------------ */
    outf = fopen(objpath, "wb");
    if (!outf)
    {
      fprintf(stderr, "ExportObj: cannot create file '%s'\n", objpath);
      retv = 0;
      break;
    }

    fprintf(outf,
            "# fcecodec OBJ File: '%s'\n"
            "# github.com/bfut/fcecodec\n"
            "mtllib %s\n", FCELIB_UTIL_GetFileName(objpath), FCELIB_UTIL_GetFileName(mtlpath));
    fflush(outf);

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
      fprintf(outf, "\no %s\n", part->PartName);

      fprintf(outf, "#part position %f %f %f\n",
                    part->PartPos.x,
                    part->PartPos.y,
                    part->PartPos.z);
      fprintf(outf, "\n");
      fflush(outf);

      /* Verts */
      fprintf(outf, "#%d verts\n", part->PNumVertices);
      if (use_part_positions)
      {
        for (j = 0; j < part->pvertices_len; ++j)
        {
          if (part->PVertices[j] < 0)
            continue;

          fprintf(outf,
                  "v %f %f %f\n",
                  mesh->vertices[ part->PVertices[j] ]->VertPos.x + part->PartPos.x,
                  mesh->vertices[ part->PVertices[j] ]->VertPos.y + part->PartPos.y,
                  - ( mesh->vertices[ part->PVertices[j] ]->VertPos.z + part->PartPos.z )  /* flip sign in Z-coordinate */
          );
        }
      }
      else
      {
        for (j = 0; j < part->pvertices_len; ++j)
        {
          if (part->PVertices[j] < 0)
            continue;

          fprintf(outf,
                  "v %f %f %f\n",
                  mesh->vertices[ part->PVertices[j] ]->VertPos.x,
                  mesh->vertices[ part->PVertices[j] ]->VertPos.y,
                  - ( mesh->vertices[ part->PVertices[j] ]->VertPos.z )  /* flip sign in Z-coordinate */
          );
        }
      }  /* if (use_part_positions) */
      fprintf(outf, "\n");
      fflush(outf);

      /* Texture coordinates */
      fprintf(outf, "#%d vt\n", 3 * part->PNumTriangles);
      for (j = 0; j < part->ptriangles_len; ++j)
      {
        if (part->PTriangles[j] < 0)
          continue;

        for (n = 0; n < 3; ++n)
        {
          fprintf(outf,
                  "vt %f %f\n",
                  mesh->triangles[ part->PTriangles[j] ]->U[n],
                  mesh->triangles[ part->PTriangles[j] ]->V[n]
          );
        }  /* for n */
      }
      fprintf(outf, "\n");
      fflush(outf);

      /* Normals */
      fprintf(outf, "#%d normals\n", part->PNumVertices);
      for (j = 0; j < part->pvertices_len; ++j)
      {
        if (part->PVertices[j] < 0)
          continue;

        fprintf(outf,
                "vn %f %f %f\n",
                mesh->vertices[ part->PVertices[j] ]->NormPos.x,
                mesh->vertices[ part->PVertices[j] ]->NormPos.y,
                - ( mesh->vertices[ part->PVertices[j] ]->NormPos.z )  /* flip sign in Z-coordinate */
        );
      }
      fprintf(outf, "\n");
      fflush(outf);

      /* Triangles */
      /* Create map: global vert index to global obj idx (of used-in-this-part verts) */
      memset(global_mesh_to_global_obj_idxs, 0xFF, mesh->vertices_len * sizeof(*global_mesh_to_global_obj_idxs));
      for (n = 0, k = 0; n < part->pvertices_len && k < part->PNumVertices; ++n)
      {
        if (part->PVertices[n] < 0)
          continue;
        global_mesh_to_global_obj_idxs[ part->PVertices[n] ] = k + 1 + sum_verts;
        ++k;
      }

      fprintf(outf, "#%d faces (verts: %d..%d)\n", part->PNumTriangles, sum_verts + 1, sum_verts + part->PNumVertices);
      for (n = 0, k = 0; n < part->ptriangles_len && k < part->PNumTriangles; ++n)
      {
        if (part->PTriangles[n] < 0)
          continue;
        triag = mesh->triangles[ part->PTriangles[n] ];

        if (filter_triagflags_0xfff == 1)
        {
          fprintf(outf,
                  "usemtl 0x%03x\n"
                  "s 1\n",
                  triag->flag & 0xfff);
        }
        else
        {
          fprintf(outf,
                  "usemtl 0x%08x\n"
                  "s 1\n",
                  triag->flag);
        }

        fprintf(outf,
                "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
                global_mesh_to_global_obj_idxs[ triag->vidx[0] ],
                3 * (sum_triags + k) + 1 + 0,
                global_mesh_to_global_obj_idxs[ triag->vidx[0] ],

                global_mesh_to_global_obj_idxs[ triag->vidx[1] ],
                3 * (sum_triags + k) + 1 + 1,
                global_mesh_to_global_obj_idxs[ triag->vidx[1] ],

                global_mesh_to_global_obj_idxs[ triag->vidx[2] ],
                3 * (sum_triags + k) + 1 + 2,
                global_mesh_to_global_obj_idxs[ triag->vidx[2] ]
        );
        ++k;
      }  /* for n,k triangles */
      fprintf(outf, "\n");
      fflush(outf);

      sum_verts  += part->PNumVertices;
      sum_triags += part->PNumTriangles;
      /* END printing undamaged part */


      /* BEGIN printing damaged part */
      if (print_damage)
      {
        fprintf(outf, "\no DAMAGE_%s\n", part->PartName);

        fprintf(outf, "#part position %f %f %f\n",
                      part->PartPos.x,
                      part->PartPos.y,
                      part->PartPos.z);
        fprintf(outf, "\n");
        fflush(outf);

        /* Verts */
        fprintf(outf, "#%d verts\n", part->PNumVertices);
        if (use_part_positions)
        {
          for (j = 0; j < part->pvertices_len; ++j)
          {
            if (part->PVertices[j] < 0)
              continue;

            fprintf(outf,
                    "v %f %f %f\n",
                    mesh->vertices[ part->PVertices[j] ]->DamgdVertPos.x + part->PartPos.x,
                    mesh->vertices[ part->PVertices[j] ]->DamgdVertPos.y + part->PartPos.y,
                    - ( mesh->vertices[ part->PVertices[j] ]->DamgdVertPos.z + part->PartPos.z )  /* flip sign in Z-coordinate */
            );
          }
        }
        else
        {
          for (j = 0; j < part->pvertices_len; ++j)
          {
            if (part->PVertices[j] < 0)
              continue;

            fprintf(outf,
                    "v %f %f %f\n",
                    mesh->vertices[ part->PVertices[j] ]->DamgdVertPos.x,
                    mesh->vertices[ part->PVertices[j] ]->DamgdVertPos.y,
                    - ( mesh->vertices[ part->PVertices[j] ]->DamgdVertPos.z )  /* flip sign in Z-coordinate */
            );
          }
        }  /* if (use_part_positions) */
        fprintf(outf, "\n");
        fflush(outf);

        /* Texture coordinates */
        fprintf(outf, "#%d vt\n", 3 * part->PNumTriangles);
        for (j = 0; j < part->ptriangles_len; ++j)
        {
          if (part->PTriangles[j] < 0)
            continue;

          for (n = 0; n < 3; ++n)
          {
            fprintf(outf,
                    "vt %f %f\n",
                    mesh->triangles[ part->PTriangles[j] ]->U[n],
                    mesh->triangles[ part->PTriangles[j] ]->V[n]
            );
          }  /* for n */
        }
        fprintf(outf, "\n");
        fflush(outf);

        /* Normals */
        fprintf(outf, "#%d normals\n", part->PNumVertices);
        for (j = 0; j < part->pvertices_len; ++j)
        {
          if (part->PVertices[j] < 0)
            continue;

          fprintf(outf,
                  "vn %f %f %f\n",
                  mesh->vertices[ part->PVertices[j] ]->DamgdNormPos.x,
                  mesh->vertices[ part->PVertices[j] ]->DamgdNormPos.y,
                  - ( mesh->vertices[ part->PVertices[j] ]->DamgdNormPos.z )  /* flip sign in Z-coordinate */
          );
        }
        fprintf(outf, "\n");
        fflush(outf);

        /* Triangles */
        /* Create map: global vert index to local part idx (of used-in-this-part verts) */
        memset(global_mesh_to_global_obj_idxs, 0xFF, mesh->vertices_len * sizeof(*global_mesh_to_global_obj_idxs));
        for (n = 0, k = 0; n < part->pvertices_len && k < part->PNumVertices; ++n)
        {
          if (part->PVertices[n] < 0)
            continue;
          global_mesh_to_global_obj_idxs[ part->PVertices[n] ] = k + 1 + sum_verts;
          ++k;
        }
        fprintf(outf, "#%d faces (verts: %d..%d)\n", part->PNumTriangles, sum_verts + 1, sum_verts + part->PNumVertices);
        for (n = 0, k = 0; n < part->ptriangles_len && k < part->PNumTriangles; ++n)
      {
        if (part->PTriangles[n] < 0)
          continue;
        triag = mesh->triangles[ part->PTriangles[n] ];

        if (filter_triagflags_0xfff == 1)
        {
          fprintf(outf,
                  "usemtl 0x%03x\n"
                  "s 1\n",
                  triag->flag & 0xfff);
        }
        else
        {
          fprintf(outf,
                  "usemtl 0x%08x\n"
                  "s 1\n",
                  triag->flag);
        }

        fprintf(outf,
                "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
                global_mesh_to_global_obj_idxs[ triag->vidx[0] ],
                3 * (sum_triags + k) + 1 + 0,
                global_mesh_to_global_obj_idxs[ triag->vidx[0] ],

                global_mesh_to_global_obj_idxs[ triag->vidx[1] ],
                3 * (sum_triags + k) + 1 + 1,
                global_mesh_to_global_obj_idxs[ triag->vidx[1] ],

                global_mesh_to_global_obj_idxs[ triag->vidx[2] ],
                3 * (sum_triags + k) + 1 + 2,
                global_mesh_to_global_obj_idxs[ triag->vidx[2] ]
        );
        ++k;
      }  /* for n,k triangles */
        fprintf(outf, "\n");
        fflush(outf);

        sum_verts  += part->PNumVertices;
        sum_triags += part->PNumTriangles;
      }  /* if (print_damage) */
      /* END printing undamaged part */
    }  /* for i parts */

    if (print_dummies)
    {
      for (i = 0; i < mesh->hdr.NumDummies; ++i)
      {
        /* unique shape names */
        fprintf(outf, "\no DUMMY_%02d_%s\n", i, mesh->hdr.DummyNames + (i * 64));

        /* Vertices */
        fprintf(outf,
                "#position %f %f %f\n",
                mesh->hdr.Dummies[i].x,
                mesh->hdr.Dummies[i].y,
                mesh->hdr.Dummies[i].z);

        for (j = 0; j < 6; ++j)
        {
          fprintf(outf,
                  "v %f %f %f\n",
                  0.1f * kVertDiamond[3 * j + 0] + mesh->hdr.Dummies[i].x,
                  0.1f * kVertDiamond[3 * j + 1] + mesh->hdr.Dummies[i].y,
                  0.1f * kVertDiamond[3 * j + 2] + mesh->hdr.Dummies[i].z * (-1)
          );
        }

        /* Triangles */
        fprintf(outf, "\n#f %d..%d (%d)\n", sum_triags + 1, sum_triags + 8, 8);

        for (j = 0; j < 8; ++j)
        {
          fprintf(outf,
                  "f %d %d %d\n",
                  kTrianglesDiamond[3 * j + 0] + sum_verts,
                  kTrianglesDiamond[3 * j + 1] + sum_verts,
                  kTrianglesDiamond[3 * j + 2] + sum_verts);
        }
        fflush(outf);

        sum_verts  += 6;
        sum_triags += 8;
      }
    }  /* if (print_dummies) */

    if (print_part_positions)
    {
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

        /* unique shape names */
        fprintf(outf, "\no POSITION_%s\n", part->PartName);

        fprintf(outf, "#part position %f %f %f\n",
                    part->PartPos.x,
                    part->PartPos.y,
                    part->PartPos.z);
        fprintf(outf, "\n");
        fflush(outf);

        /* Vertices */
        for (j = 0; j < 6; ++j)
        {
          fprintf(outf,
                  "v %f %f %f\n",
                  0.1f * kVertDiamond[3 * j + 0] + part->PartPos.x,
                  0.1f * kVertDiamond[3 * j + 1] + part->PartPos.y,
                  0.1f * kVertDiamond[3 * j + 2] + part->PartPos.z * (-1)
          );
        }

        /* Triangles */
        fprintf(outf, "\n#f %d..%d (%d)\n", sum_triags + 1, sum_triags + 8, 8);

        for (j = 0; j < 8; ++j)
        {
          fprintf(outf,
                  "f %d %d %d\n",
                  kTrianglesDiamond[3 * j + 0] + sum_verts,
                  kTrianglesDiamond[3 * j + 1] + sum_verts,
                  kTrianglesDiamond[3 * j + 2] + sum_verts);
        }
        fflush(outf);

        sum_verts  += 6;
        sum_triags += 8;
      }
    }  /* if (print_part_positions) */

    if (fclose(outf) != 0)
    {
      fprintf(stderr, "ExportObj: cannot close file '%s'\n", objpath);
      retv = 0;
      break;
    }

    break;
  }  /* for (;;) */

  free(global_mesh_to_global_obj_idxs);
  return retv;
}

/*
  Limited to 64 parts. Returns boolean.

  If center_parts == 1, centroids and vert positions will be recalculated and reset for all parts. This would change *mesh.
*/
int FCELIB_IO_EncodeFce3(const FcelibMesh *mesh, unsigned char **outbuf, int outbufsz, int center_parts)
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
    {
      const int fsize = FCELIB_FCETYPES_Fce3ComputeSize(mesh->hdr.NumVertices, mesh->hdr.NumTriangles);
      if (outbufsz < fsize)
      {
        fprintf(stderr, "EncodeFce3: Buffer too small (expects outbufsz >= %d)\n", fsize);
        break;
      }
    }

    global_mesh_to_local_fce_idxs = (int *)malloc(mesh->vertices_len * sizeof(*global_mesh_to_local_fce_idxs));
    if (!global_mesh_to_local_fce_idxs)
    {
      fprintf(stderr, "EncodeFce3: Cannot allocate memory\n");
      break;
    }

    memset(*outbuf, 0, outbufsz * sizeof(**outbuf));

    /* Header --------------------------------------------------------------- */
    /* tmp = 0;
    memcpy(*buf + 0x0000, &tmp, 4); */
    memcpy(*outbuf + 0x0004, &mesh->hdr.NumTriangles, 4);
    memcpy(*outbuf + 0x0008, &mesh->hdr.NumVertices, 4);
    memcpy(*outbuf + 0x000C, &mesh->hdr.NumArts, 4);

/*    buf = 0; */
/*    memcpy(*outbuf + 0x0010, &buf, 4); */
    buf  = 12 * mesh->hdr.NumVertices;
    memcpy(*outbuf + 0x0014, &buf, 4);
    buf += 12 * mesh->hdr.NumVertices;
    memcpy(*outbuf + 0x0018, &buf, 4);

    buf += 56 * mesh->hdr.NumTriangles;
    memcpy(*outbuf + 0x001C, &buf, 4);
    buf += 32 * mesh->hdr.NumVertices;
    memcpy(*outbuf + 0x0020, &buf, 4);
    buf += 12 * mesh->hdr.NumVertices;
    memcpy(*outbuf + 0x0024, &buf, 4);

    /* Center parts around local centroid */
    if (center_parts == 1)
    {
      tVector centroid;
      /* i - internal part index, j - part order */
      for (i = 0, j = 0; i < mesh->parts_len && j < SCL_min(12, mesh->hdr.NumParts); ++i)
      {
        if (mesh->hdr.Parts[i] < 0)
          continue;
        part = mesh->parts[ mesh->hdr.Parts[i] ];
        FCELIB_TYPES_GetPartCentroid(mesh, part, &centroid);
        FCELIB_TYPES_ResetPartCenter(mesh, part, centroid);
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

      x_array = (float *)malloc(3 * (mesh->vertices_len + 1) * sizeof(*x_array));
      if (!x_array)
      {
        fprintf(stderr, "EncodeFce3: Cannot allocate memory\n");
        retv = 0;
        break;
      }
      memset(x_array, 0, 3 * (mesh->vertices_len + 1) * sizeof(*x_array));
      y_array = x_array + mesh->vertices_len;
      z_array = y_array + mesh->vertices_len;

      /* i - internal part index, j - part order */
      for (i = 0, j = 0; i < mesh->parts_len && j < SCL_min(12, mesh->hdr.NumParts); ++i)
      {
        if (mesh->hdr.Parts[i] < 0 || (j > 4 && j != 12))
          continue;

        part = mesh->parts[ mesh->hdr.Parts[i] ];

        /* n - internal vert index, k - vert order */
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
        count_verts += k;

        ++j;
      }

      qsort(x_array, count_verts, 4, FCELIB_UTIL_CompareFloats);
      qsort(y_array, count_verts, 4, FCELIB_UTIL_CompareFloats);
      qsort(z_array, count_verts, 4, FCELIB_UTIL_CompareFloats);

      x_array[0] = 0.5f * SCL_abs(x_array[count_verts - 1] - x_array[0]);
      y_array[0] = SCL_abs(y_array[0]) - 0.02f;
      z_array[0] = 0.5f * SCL_abs(z_array[count_verts - 1] - z_array[0]);

      memcpy(*outbuf + 0x0028, x_array, 4);
      memcpy(*outbuf + 0x002C, y_array, 4);
      memcpy(*outbuf + 0x0030, z_array, 4);

      free(x_array);
    }  /* Set HalfSizes */

    /* Dummies */
    buf = SCL_min(16, mesh->hdr.NumDummies);
    memcpy(*outbuf + 0x0034, &buf, 4);
    for (i = 0; i < SCL_min(16, mesh->hdr.NumDummies); ++i)
    {
      memcpy(*outbuf + 0x0038 + i * 12 + 0, &mesh->hdr.Dummies[i].x, 4);
      memcpy(*outbuf + 0x0038 + i * 12 + 4, &mesh->hdr.Dummies[i].y, 4);
      memcpy(*outbuf + 0x0038 + i * 12 + 8, &mesh->hdr.Dummies[i].z, 4);
    }

    /* PartPos */
    /* P1stVertices */
    /* PNumVertices */
    /* P1stTriangles */
    /* PNumTriangles */
    /* PartNames */
    buf = SCL_min(64, mesh->hdr.NumParts);
    memcpy(*outbuf + 0x00F8, &buf, 4);
    for (i = 0, j = 0; (i < mesh->parts_len) && (j < SCL_min(64, mesh->hdr.NumParts)); ++i)
    {
      if (mesh->hdr.Parts[i] < 0)
        continue;
      part = mesh->parts[ mesh->hdr.Parts[i] ];

      memcpy(*outbuf + 0x00FC + j * 12 + 0, &part->PartPos.x, 4);
      memcpy(*outbuf + 0x00FC + j * 12 + 4, &part->PartPos.y, 4);
      memcpy(*outbuf + 0x00FC + j * 12 + 8, &part->PartPos.z, 4);

      memcpy(*outbuf + 0x03FC + j * 4, &sum_verts, 4);
      sum_verts += part->PNumVertices;
      memcpy(*outbuf + 0x04FC + j * 4, &part->PNumVertices, 4);

      memcpy(*outbuf + 0x05FC + j * 4, &sum_triags, 4);
      sum_triags += part->PNumTriangles;
      memcpy(*outbuf + 0x06FC + j * 4, &part->PNumTriangles, 4);

      memcpy(*outbuf + 0x0E04 + j * 64, &part->PartName, 64);

      ++j;
    }
    FCELIB_UTIL_EnsureStrings((char *)*outbuf + 0x0E04, 64, 64);
    FCELIB_UTIL_UnprintableToNul((char *)*outbuf + 0x0E04, 64, 64);
    FCELIB_UTIL_TidyUpNames((char *)*outbuf + 0x0E04, 64, 64, 64);

    /* PriColors */
    buf = SCL_min(mesh->hdr.NumColors, 16);
    memcpy(*outbuf + 0x07FC, &buf, 4);
    FCELIB_TYPES_WriteFceColors(*outbuf + 0x0800, mesh->hdr.PriColors, SCL_min(mesh->hdr.NumColors, 16), 4);  /* little-endian int from unsigned char */

    /* SecColors */
    buf = SCL_min(mesh->hdr.NumSecColors, 16);
    memcpy(*outbuf + 0x0900, &buf, 4);
    FCELIB_TYPES_WriteFceColors(*outbuf + 0x0904, mesh->hdr.SecColors, SCL_min(mesh->hdr.NumSecColors, 16), 4);  /* little-endian int from unsigned char */

    /* DummyNames */
    memcpy(*outbuf + 0x0A04, &mesh->hdr.DummyNames, 16 * 64);
    FCELIB_UTIL_EnsureStrings((char *)*outbuf + 0x0A04, 16, 64);
    FCELIB_UTIL_UnprintableToNul((char *)*outbuf + 0x0A04, 16, 64);
    FCELIB_UTIL_TidyUpNames((char *)*outbuf + 0x0A04, 16, 16, 64);

    /* Print vertices ------------------------------------------------------- */
    /* VertTblOffset = 0 */
    sum_verts = 0;
    for (i = 0, j = 0; (i < mesh->parts_len) && (j < SCL_min(64, mesh->hdr.NumParts)); ++i)
    {
      if (mesh->hdr.Parts[i] < 0)
        continue;
      part = mesh->parts[ mesh->hdr.Parts[i] ];
      for (n = 0, k = 0; (n < part->pvertices_len) && (k < part->PNumVertices); ++n)
      {
        if (part->PVertices[n] < 0)
          continue;

        memcpy(*outbuf + 0x1F04 + (sum_verts + k) * 12 + 0x00, &mesh->vertices[ part->PVertices[n] ]->VertPos.x, 4);
        memcpy(*outbuf + 0x1F04 + (sum_verts + k) * 12 + 0x04, &mesh->vertices[ part->PVertices[n] ]->VertPos.y, 4);
        memcpy(*outbuf + 0x1F04 + (sum_verts + k) * 12 + 0x08, &mesh->vertices[ part->PVertices[n] ]->VertPos.z, 4);
        ++k;
      }
      sum_verts += part->PNumVertices;
      ++j;
    }

    /* Print normals ------------------------------------------------------- */
    buf = 0x1F04 + 12 * mesh->hdr.NumVertices;  /* NormTblOffset */
    sum_verts = 0;
    for (i = 0, j = 0; i < mesh->parts_len && j < SCL_min(64, mesh->hdr.NumParts); ++i)
    {
      if (mesh->hdr.Parts[i] < 0)
        continue;
      part = mesh->parts[ mesh->hdr.Parts[i] ];
      for (n = 0, k = 0; n < part->pvertices_len && k < part->PNumVertices; ++n)
      {
        if (part->PVertices[n] < 0)
          continue;
        memcpy(*outbuf + buf + (sum_verts + k) * 12 + 0x00, &mesh->vertices[ part->PVertices[n] ]->NormPos.x, 4);
        memcpy(*outbuf + buf + (sum_verts + k) * 12 + 0x04, &mesh->vertices[ part->PVertices[n] ]->NormPos.y, 4);
        memcpy(*outbuf + buf + (sum_verts + k) * 12 + 0x08, &mesh->vertices[ part->PVertices[n] ]->NormPos.z, 4);
        ++k;
      }
      sum_verts += part->PNumVertices;
      ++j;
    }

    /* Print triangles ------------------------------------------------------ */
    tmp = 0xff00;
    sum_triags = 0;
    buf += 12 * mesh->hdr.NumVertices;  /* TriaTblOffset */
    for (i = 0, j = 0; i < mesh->parts_len && j < SCL_min(64, mesh->hdr.NumParts); ++i)
    {
      if (mesh->hdr.Parts[i] < 0)
        continue;
      part = mesh->parts[ mesh->hdr.Parts[i] ];

      /* Create map: global vert index to local part idx (of used-in-this-part verts) */
      memset(global_mesh_to_local_fce_idxs, 0xFF, mesh->vertices_len * sizeof(*global_mesh_to_local_fce_idxs));
      for (n = 0, k = 0; n < part->pvertices_len && k < part->PNumVertices; ++n)
      {
        if (part->PVertices[n] < 0)
          continue;
        global_mesh_to_local_fce_idxs[ part->PVertices[n] ] = k;
        ++k;
      }

      for (n = 0, k = 0; n < part->ptriangles_len && k < part->PNumTriangles; ++n)
      {
        if (part->PTriangles[n] < 0)
          continue;
        triag = mesh->triangles[ part->PTriangles[n] ];
        memcpy(*outbuf + buf + (sum_triags + k) * 56 + 0x00, &triag->tex_page, 4);
        memcpy(*outbuf + buf + (sum_triags + k) * 56 + 0x04, &global_mesh_to_local_fce_idxs[ triag->vidx[0] ], 4);
        memcpy(*outbuf + buf + (sum_triags + k) * 56 + 0x08, &global_mesh_to_local_fce_idxs[ triag->vidx[1] ], 4);
        memcpy(*outbuf + buf + (sum_triags + k) * 56 + 0x0C, &global_mesh_to_local_fce_idxs[ triag->vidx[2] ], 4);
        memcpy(*outbuf + buf + (sum_triags + k) * 56 + 0x10 + 0x00, &tmp, 4);
        memcpy(*outbuf + buf + (sum_triags + k) * 56 + 0x10 + 0x04, &tmp, 4);
        memcpy(*outbuf + buf + (sum_triags + k) * 56 + 0x10 + 0x08, &tmp, 4);
        memcpy(*outbuf + buf + (sum_triags + k) * 56 + 0x1C, &triag->flag, 4);
        memcpy(*outbuf + buf + (sum_triags + k) * 56 + 0x20, &triag->U, 12);
        memcpy(*outbuf + buf + (sum_triags + k) * 56 + 0x2C, &triag->V, 12);
        ++k;
      }
      sum_triags += part->PNumTriangles;
      ++j;
    }

    retv = 1;
    break;
  }  /* for (;;) */

  free(global_mesh_to_local_fce_idxs);

  return retv;
}

/*
  Limited to 64 parts, 16 dummies, 16 colors. Returns boolean.
  For FCE4M, call with fce_version = 0x00101015

  If center_parts == 1, centroids and vert positions will be recalculated and reset for all parts. This would change *mesh.
*/
int FCELIB_IO_EncodeFce4(const FcelibMesh *mesh, unsigned char **outbuf, int outbufsz, int center_parts, int fce_version)
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

  for (;;)
  {
    {
      const int fsize = FCELIB_FCETYPES_Fce4ComputeSize(fce_version, mesh->hdr.NumVertices, mesh->hdr.NumTriangles);
      if (outbufsz < fsize)
      {
        fprintf(stderr, "EncodeFce4: Buffer too small (expects outbufsz >= %d)\n", fsize);
        break;
      }
    }

    global_mesh_to_local_fce_idxs = (int *)malloc(mesh->vertices_len * sizeof(*global_mesh_to_local_fce_idxs));
    if (!global_mesh_to_local_fce_idxs)
    {
      fprintf(stderr, "EncodeFce4: Cannot allocate memory\n");
      break;
    }

    memset(*outbuf, 0, outbufsz * sizeof(**outbuf));

    /* Header --------------------------------------------------------------- */
    if (fce_version == 0x00101015)
      tmp = 0x00101015;
    else
      tmp = 0x00101014;
    memcpy(*outbuf + 0x0000, &tmp, 4);  /* Version */
    /* memcpy(*outbuf + 0x0003, &tmp, 4);  */ /* Unknown1 */
    memcpy(*outbuf + 0x0008, &mesh->hdr.NumTriangles, 4);
    memcpy(*outbuf + 0x000C, &mesh->hdr.NumVertices, 4);
    memcpy(*outbuf + 0x0010, &mesh->hdr.NumArts, 4);

    /* tmp = 0; */
    /* memcpy(*outbuf + 0x0014, &tmp, 4); */
    tmp  = 12 * mesh->hdr.NumVertices;
    memcpy(*outbuf + 0x0018, &tmp, 4);
    tmp += 12 * mesh->hdr.NumVertices;
    memcpy(*outbuf + 0x001C, &tmp, 4);

    tmp += 56 * mesh->hdr.NumTriangles;
    memcpy(*outbuf + 0x0020, &tmp, 4);
    tmp += 32 * mesh->hdr.NumVertices;
    memcpy(*outbuf + 0x0024, &tmp, 4);
    tmp += 12 * mesh->hdr.NumVertices;
    memcpy(*outbuf + 0x0028, &tmp, 4);

    tmp += 12 * mesh->hdr.NumVertices;
    memcpy(*outbuf + 0x002c, &tmp, 4);
    tmp += 12 * mesh->hdr.NumVertices;
    memcpy(*outbuf + 0x0030, &tmp, 4);
    tmp += 12 * mesh->hdr.NumVertices;
    memcpy(*outbuf + 0x0034, &tmp, 4);
    tmp += 12 * mesh->hdr.NumVertices;
    memcpy(*outbuf + 0x0038, &tmp, 4);

    tmp += 12 * mesh->hdr.NumVertices;
    memcpy(*outbuf + 0x003c, &tmp, 4);
    tmp += 4 * mesh->hdr.NumVertices;
    memcpy(*outbuf + 0x0040, &tmp, 4);
    tmp += 4 * mesh->hdr.NumVertices;
    memcpy(*outbuf + 0x0044, &tmp, 4);

    tmp += 4 * mesh->hdr.NumVertices;
    memcpy(*outbuf + 0x0048, &tmp, 4);

    /* Center parts around local centroid */
    if (center_parts == 1)
    {
      tVector centroid;
      /* i - internal part index, j - part order */
      for (i = 0, j = 0; i < mesh->parts_len && j < SCL_min(12, mesh->hdr.NumParts); ++i)
      {
        if (mesh->hdr.Parts[i] < 0)
          continue;
        part = mesh->parts[ mesh->hdr.Parts[i] ];
        FCELIB_TYPES_GetPartCentroid(mesh, part, &centroid);
#if SCL_DEBUG >= 1
        printf("<%s> centroid: (%f, %f, %f) partpos: (%f, %f, %f)\n", part->PartName, centroid.x, centroid.y, centroid.z, part->PartPos.x, part->PartPos.y, part->PartPos.z);
#endif
        FCELIB_TYPES_ResetPartCenter(mesh, part, centroid);
#if SCL_DEBUG >= 1
        printf("<%s> new partpos: (%f, %f, %f)\n", part->PartName, part->PartPos.x, part->PartPos.y, part->PartPos.z);
#endif
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

      x_array = (float *)malloc(3 * (mesh->vertices_len + 1) * sizeof(*x_array));
      if (!x_array)
      {
        fprintf(stderr, "EncodeFce4: Cannot allocate memory\n");
        retv = 0;
        break;
      }
      memset(x_array, 0, 3 * (mesh->vertices_len + 1) * sizeof(*x_array));
      y_array = x_array + mesh->vertices_len;
      z_array = y_array + mesh->vertices_len;

      /* i - internal part index, j - part order */
      for (i = 0, j = 0; i < mesh->parts_len && j < SCL_min(12, mesh->hdr.NumParts); ++i)
      {
        if (mesh->hdr.Parts[i] < 0)
          continue;

        part = mesh->parts[ mesh->hdr.Parts[i] ];
        if (fce_version == 0x00101014)  /* use all parts for Fce4M (experimental) */
        {
          if (!FCELIB_UTIL_StrIsInArray(part->PartName, kFce4HiBodyParts))
            continue;
        }
#if SCL_DEBUG >= 1
        printf("HalfSize: <%s> partpos: (%f, %f, %f)\n", part->PartName, part->PartPos.x, part->PartPos.y, part->PartPos.z);
        printf("HalfSize: PNumVertices: %d\n", part->PNumVertices);
#endif

        /* n - internal vert index, k - vert order */
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
        count_verts += k;

        ++j;
      }

      qsort(x_array, count_verts, 4, FCELIB_UTIL_CompareFloats);
      qsort(y_array, count_verts, 4, FCELIB_UTIL_CompareFloats);
      qsort(z_array, count_verts, 4, FCELIB_UTIL_CompareFloats);
      x_array[0] = 0.5f * SCL_abs(x_array[count_verts - 1] - x_array[0]);
      y_array[0] = SCL_abs(y_array[0]) - 0.02f;
      z_array[0] = 0.5f * SCL_abs(z_array[count_verts - 1] - z_array[0]);

      memcpy(*outbuf + 0x004c, x_array, 4);
      memcpy(*outbuf + 0x0050, y_array, 4);
      memcpy(*outbuf + 0x0054, z_array, 4);

      free(x_array);
    }  /* Set HalfSizes */

    /* Dummies */
    tmp = SCL_min(16, mesh->hdr.NumDummies);
    memcpy(*outbuf + 0x0058, &tmp, 4);
    for (i = 0; i < SCL_min(16, mesh->hdr.NumDummies); ++i)
    {
      memcpy(*outbuf + 0x005c + i * 12 + 0, &mesh->hdr.Dummies[i].x, 4);
      memcpy(*outbuf + 0x005c + i * 12 + 4, &mesh->hdr.Dummies[i].y, 4);
      memcpy(*outbuf + 0x005c + i * 12 + 8, &mesh->hdr.Dummies[i].z, 4);
    }

    /* PartPos */
    /* P1stVertices */
    /* PNumVertices */
    /* P1stTriangles */
    /* PNumTriangles */
    /* PartNames */
    tmp = SCL_min(64, mesh->hdr.NumParts);
    memcpy(*outbuf + 0x011c, &tmp, 4);
    for (i = 0, j = 0; (i < mesh->parts_len) && (j < SCL_min(64, mesh->hdr.NumParts)); ++i)
    {
      if (mesh->hdr.Parts[i] < 0)
        continue;
      part = mesh->parts[ mesh->hdr.Parts[i] ];

      memcpy(*outbuf + 0x0120 + j * 12 + 0, &part->PartPos.x, 4);
      memcpy(*outbuf + 0x0120 + j * 12 + 4, &part->PartPos.y, 4);
      memcpy(*outbuf + 0x0120 + j * 12 + 8, &part->PartPos.z, 4);

      memcpy(*outbuf + 0x0420 + j * 4, &sum_verts, 4);
      sum_verts += part->PNumVertices;
      memcpy(*outbuf + 0x0520 + j * 4, &part->PNumVertices, 4);

      memcpy(*outbuf + 0x0620 + j * 4, &sum_triags, 4);
      sum_triags += part->PNumTriangles;
      memcpy(*outbuf + 0x0720 + j * 4, &part->PNumTriangles, 4);

      memcpy(*outbuf + 0x0e28 + j * 64, &part->PartName, 64);

      ++j;
    }
    FCELIB_UTIL_EnsureStrings((char *)*outbuf + 0x0E28, 64, 64);
    FCELIB_UTIL_UnprintableToNul((char *)*outbuf + 0x0E28, 64, 64);
    FCELIB_UTIL_TidyUpNames((char *)*outbuf + 0x0E28, 64, 64, 64);

    /* NumColors */
    /* PriColors */
    /* IntColors */
    /* SecColors */
    /* DriColors */
    tmp = SCL_min(mesh->hdr.NumColors, 16);
    memcpy(*outbuf + 0x0820, &tmp, 4);
    FCELIB_TYPES_WriteFceColors(*outbuf + 0x0824, mesh->hdr.PriColors, SCL_min(mesh->hdr.NumColors, 16), 1);
    FCELIB_TYPES_WriteFceColors(*outbuf + 0x0864, mesh->hdr.IntColors, SCL_min(mesh->hdr.NumColors, 16), 1);
    FCELIB_TYPES_WriteFceColors(*outbuf + 0x08A4, mesh->hdr.SecColors, SCL_min(mesh->hdr.NumColors, 16), 1);
    FCELIB_TYPES_WriteFceColors(*outbuf + 0x08E4, mesh->hdr.DriColors, SCL_min(mesh->hdr.NumColors, 16), 1);

    /* FCE4M experimental */
    if (fce_version == 0x00101015)  memcpy(*outbuf + 0x0924, &mesh->hdr.Unknown3, 4);

    /* DummyNames */
    memcpy(*outbuf + 0x0A28, &mesh->hdr.DummyNames, 16 * 64);
    FCELIB_UTIL_EnsureStrings((char *)*outbuf + 0x0A28, 16, 64);
    FCELIB_UTIL_UnprintableToNul((char *)*outbuf + 0x0A28, 16, 64);
    FCELIB_UTIL_TidyUpNames((char *)*outbuf + 0x0A28, 16, 16, 64);

    /* Print vertices ------------------------------------------------------- */
    /* VertTblOffset = 0 */
    sum_verts = 0;
    for (i = 0, j = 0; (i < mesh->parts_len) && (j < SCL_min(64, mesh->hdr.NumParts)); ++i)
    {
      if (mesh->hdr.Parts[i] < 0)
        continue;
      part = mesh->parts[ mesh->hdr.Parts[i] ];
      for (n = 0, k = 0; (n < part->pvertices_len) && (k < part->PNumVertices); ++n)
      {
        if (part->PVertices[n] < 0)
          continue;

        memcpy(*outbuf + 0x2038 + (sum_verts + k) * 12 + 0x00, &mesh->vertices[ part->PVertices[n] ]->VertPos.x, 4);
        memcpy(*outbuf + 0x2038 + (sum_verts + k) * 12 + 0x04, &mesh->vertices[ part->PVertices[n] ]->VertPos.y, 4);
        memcpy(*outbuf + 0x2038 + (sum_verts + k) * 12 + 0x08, &mesh->vertices[ part->PVertices[n] ]->VertPos.z, 4);
        ++k;
      }
      sum_verts += part->PNumVertices;
      ++j;
    }
    memcpy(&tmp, *outbuf + 0x002c, 4);  /* UndamgdVertTblOffset */
    memcpy(*outbuf + 0x2038 + tmp, *outbuf + 0x2038, 12 * mesh->hdr.NumVertices);

    memcpy(&tmp, *outbuf + 0x0034, 4);  /* DamgdVertTblOffset */
    sum_verts = 0;
    for (i = 0, j = 0; (i < mesh->parts_len) && (j < SCL_min(64, mesh->hdr.NumParts)); ++i)
    {
      if (mesh->hdr.Parts[i] < 0)
        continue;
      part = mesh->parts[ mesh->hdr.Parts[i] ];
      for (n = 0, k = 0; (n < part->pvertices_len) && (k < part->PNumVertices); ++n)
      {
        if (part->PVertices[n] < 0)
          continue;

        memcpy(*outbuf + 0x2038 + tmp + (sum_verts + k) * 12 + 0x00, &mesh->vertices[ part->PVertices[n] ]->DamgdVertPos.x, 4);
        memcpy(*outbuf + 0x2038 + tmp + (sum_verts + k) * 12 + 0x04, &mesh->vertices[ part->PVertices[n] ]->DamgdVertPos.y, 4);
        memcpy(*outbuf + 0x2038 + tmp + (sum_verts + k) * 12 + 0x08, &mesh->vertices[ part->PVertices[n] ]->DamgdVertPos.z, 4);
        ++k;
      }
      sum_verts += part->PNumVertices;
      ++j;
    }

    /* Print normals -------------------------------------------------------- */
    memcpy(&tmp, *outbuf + 0x0018, 4);  /* NormTblOffset */
    sum_verts = 0;
    for (i = 0, j = 0; i < mesh->parts_len && j < SCL_min(64, mesh->hdr.NumParts); ++i)
    {
      if (mesh->hdr.Parts[i] < 0)
        continue;
      part = mesh->parts[ mesh->hdr.Parts[i] ];
      for (n = 0, k = 0; n < part->pvertices_len && k < part->PNumVertices; ++n)
      {
        if (part->PVertices[n] < 0)
          continue;
        memcpy(*outbuf + 0x2038 + tmp + (sum_verts + k) * 12 + 0x00, &mesh->vertices[ part->PVertices[n] ]->NormPos.x, 4);
        memcpy(*outbuf + 0x2038 + tmp + (sum_verts + k) * 12 + 0x04, &mesh->vertices[ part->PVertices[n] ]->NormPos.y, 4);
        memcpy(*outbuf + 0x2038 + tmp + (sum_verts + k) * 12 + 0x08, &mesh->vertices[ part->PVertices[n] ]->NormPos.z, 4);
        ++k;
      }
      sum_verts += part->PNumVertices;
      ++j;
    }
    memcpy(&tmp, *outbuf + 0x0030, 4);  /* UndamgdNormTblOffset */
    memcpy(*outbuf + 0x2038 + tmp, *outbuf + 0x2038 + 12 * mesh->hdr.NumVertices, 12 * mesh->hdr.NumVertices);

    memcpy(&tmp, *outbuf + 0x0038, 4);  /* DamgdNormTblOffset */
    sum_verts = 0;
    for (i = 0, j = 0; i < mesh->parts_len && j < SCL_min(64, mesh->hdr.NumParts); ++i)
    {
      if (mesh->hdr.Parts[i] < 0)
        continue;
      part = mesh->parts[ mesh->hdr.Parts[i] ];
      for (n = 0, k = 0; n < part->pvertices_len && k < part->PNumVertices; ++n)
      {
        if (part->PVertices[n] < 0)
          continue;
        memcpy(*outbuf + 0x2038 + tmp + (sum_verts + k) * 12 + 0x00, &mesh->vertices[ part->PVertices[n] ]->DamgdNormPos.x, 4);
        memcpy(*outbuf + 0x2038 + tmp + (sum_verts + k) * 12 + 0x04, &mesh->vertices[ part->PVertices[n] ]->DamgdNormPos.y, 4);
        memcpy(*outbuf + 0x2038 + tmp + (sum_verts + k) * 12 + 0x08, &mesh->vertices[ part->PVertices[n] ]->DamgdNormPos.z, 4);
        ++k;
      }
      sum_verts += part->PNumVertices;
      ++j;
    }

    /* Print animation ------------------------------------------------------ */
    memcpy(&tmp, *outbuf + 0x0040, 4);  /* AnimationTblOffset */
    sum_verts = 0;
    for (i = 0, j = 0; i < mesh->parts_len && j < SCL_min(64, mesh->hdr.NumParts); ++i)
    {
      if (mesh->hdr.Parts[i] < 0)
        continue;
      part = mesh->parts[ mesh->hdr.Parts[i] ];
      for (n = 0, k = 0; n < part->pvertices_len && k < part->PNumVertices; ++n)
      {
        if (part->PVertices[n] < 0)
          continue;
        memcpy(*outbuf + 0x2038 + tmp + (sum_verts + k) * 4 + 0x00, &mesh->vertices[ part->PVertices[n] ]->Animation, 4);
        ++k;
      }
      sum_verts += part->PNumVertices;
      ++j;
    }

    /* Print triangles ------------------------------------------------------ */
    sum_triags = 0;
    memcpy(&tmp, *outbuf + 0x001c, 4);  /* TriaTblOffset */
    {
      const int padding = 0xff00;
      float V_tmp[3];
      int h;

      for (i = 0, j = 0; i < mesh->parts_len && j < SCL_min(64, mesh->hdr.NumParts); ++i)
      {
        if (mesh->hdr.Parts[i] < 0)
          continue;
        part = mesh->parts[ mesh->hdr.Parts[i] ];

        /* Create map: global vert index to local part idx (of used-in-this-part verts) */
        memset(global_mesh_to_local_fce_idxs, 0xFF, mesh->vertices_len * sizeof(*global_mesh_to_local_fce_idxs));
        for (n = 0, k = 0; n < part->pvertices_len && k < part->PNumVertices; ++n)
        {
          if (part->PVertices[n] < 0)
            continue;
          global_mesh_to_local_fce_idxs[ part->PVertices[n] ] = k;
          ++k;
        }

        for (n = 0, k = 0; n < part->ptriangles_len && k < part->PNumTriangles; ++n)
        {
          if (part->PTriangles[n] < 0)
            continue;
          triag = mesh->triangles[ part->PTriangles[n] ];

          memcpy(&V_tmp, &triag->V, 12);
          /* if (fce_version == 0x00101014) */
          {
            for (h = 0; h < 3; ++h)
              V_tmp[h] = 1 - V_tmp[h];
          }

          memcpy(*outbuf + 0x2038 + tmp + (sum_triags + k) * 56 + 0x00, &triag->tex_page, 4);
          memcpy(*outbuf + 0x2038 + tmp + (sum_triags + k) * 56 + 0x04, &global_mesh_to_local_fce_idxs[ triag->vidx[0] ], 4);
          memcpy(*outbuf + 0x2038 + tmp + (sum_triags + k) * 56 + 0x08, &global_mesh_to_local_fce_idxs[ triag->vidx[1] ], 4);
          memcpy(*outbuf + 0x2038 + tmp + (sum_triags + k) * 56 + 0x0C, &global_mesh_to_local_fce_idxs[ triag->vidx[2] ], 4);
          memcpy(*outbuf + 0x2038 + tmp + (sum_triags + k) * 56 + 0x10 + 0x00, &padding, 4);
          memcpy(*outbuf + 0x2038 + tmp + (sum_triags + k) * 56 + 0x10 + 0x04, &padding, 4);
          memcpy(*outbuf + 0x2038 + tmp + (sum_triags + k) * 56 + 0x10 + 0x08, &padding, 4);
          memcpy(*outbuf + 0x2038 + tmp + (sum_triags + k) * 56 + 0x1C, &triag->flag, 4);
          memcpy(*outbuf + 0x2038 + tmp + (sum_triags + k) * 56 + 0x20, &triag->U, 12);
          memcpy(*outbuf + 0x2038 + tmp + (sum_triags + k) * 56 + 0x2C, &V_tmp, 12);
          ++k;
        }
        sum_triags += part->PNumTriangles;
        ++j;
      }  /* for i, j */
    }

    retv = 1;
    break;
  }  /* for (;;) */

  free(global_mesh_to_local_fce_idxs);

  return retv;
}

/*
  Assumes (mesh != NULL).
  Otherwise, expects non-NULL parameters.

  vert_idxs: 012...
  vert_texcoords: uuuvvv...
  vert_pos: xyzxyzxyz...
  normals: xyzxyzxyz...

  C API: Assumes mesh must have been initialized
*/
int FCELIB_IO_GeomDataToNewPart(FcelibMesh *mesh,
                                int *vert_idxs, const int vert_idxs_len,  /* N*3, N triags */
                                float *vert_texcoords, int vert_texcoords_len,  /* N*6 */
                                float *vert_pos, int vert_pos_len,  /* M*3, M verts */
                                float *normals, int normals_len)  /* M*3 */
{
  int pid_new = -1;
  int internal_pid_new = -1;
  FcelibPart *part = NULL;
  FcelibTriangle *triag;
  FcelibVertex *vert;
  int j;
  int vidx_1st;
  int tidx_1st;

  for (;;)
  {
#ifndef FCELIB_PYTHON_BINDINGS
    if (!vert_idxs || vert_idxs_len <= 0 ||
        !vert_texcoords || vert_texcoords_len <= 0 ||
        !vert_pos || vert_pos_len <= 0 ||
        !normals || normals_len <= 0)
    {
      fprintf(stderr, "DecodeFce: an input is NULL\n");
      break;
    }
#endif

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

    if (FCELIB_UTIL_ArrMax(vert_idxs, vert_idxs_len) >= (int)(vert_pos_len / 3))  /* allow adding vertices not referenced by triangles */
    {
      fprintf(stderr, "GeomDataToNewPart: Triangle vertice index(es) out of range (assumes zero-indexed)\n");
      break;
    }

    /* Lengthen part index map only if necessary */
    if (mesh->parts_len < 1 || mesh->hdr.Parts[mesh->parts_len - 1] >= 0)
    {
      if (!FCELIB_TYPES_AddParts(mesh, 1))
      {
        fprintf(stderr, "GeomDataToNewPart: Cannot add part\n");
        break;
      }
    }

    /* Get first unused index */
    internal_pid_new = FCELIB_TYPES_GetFirstUnusedGlobalPartIdx(mesh);
    if (internal_pid_new > 0)
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
    mesh->hdr.Parts[internal_pid_new] = 1 + FCELIB_UTIL_ArrMax(mesh->hdr.Parts, mesh->parts_len);
    if (mesh->hdr.Parts[internal_pid_new] < 0)
    {
      fprintf(stderr, "GeomDataToNewPart: Cannot set new part index\n");
      internal_pid_new = -1;
      break;
    }

    part = (FcelibPart *)malloc(sizeof(*part));
    if (!part)
    {
      fprintf(stderr, "GeomDataToNewPart: Cannot allocate memory (part)\n");
      internal_pid_new = -1;
      break;
    }
    memset(part, 0, sizeof(*part));

    mesh->parts[ mesh->hdr.Parts[internal_pid_new] ] = part;

    sprintf(part->PartName, "IoGeomDataToNewPart_%d", internal_pid_new);  /* unlikely to exceed 63 characters */

    part->PNumVertices = (int)(vert_pos_len / 3);
    part->PNumTriangles = (int)(vert_idxs_len / 3);

    ++mesh->hdr.NumParts;

    /* Add triangles */
    if (!FCELIB_TYPES_AddTrianglesToPart(part, part->PNumTriangles))
    {
      internal_pid_new = -1;
      break;
    }
    if (mesh->triangles_len < tidx_1st + part->PNumTriangles)
    {
      if (!FCELIB_TYPES_AddTrianglesToMesh(mesh, tidx_1st + part->PNumTriangles - mesh->triangles_len))
      {
        /* this is a fatal error */
        fprintf(stderr, "GeomDataToNewPart: Cannot add triangles\n");
        internal_pid_new = -1;
        break;
      }
    }
    mesh->hdr.NumTriangles += part->PNumTriangles;

    for (j = 0; j < part->PNumTriangles; ++j)
    {
      part->PTriangles[j] = tidx_1st + j;

      mesh->triangles[tidx_1st + j] = (FcelibTriangle *)malloc(sizeof(**mesh->triangles));
      triag = mesh->triangles[tidx_1st + j];
      if (!triag)
      {
        /* this is a fatal error */
        fprintf(stderr, "GeomDataToNewPart: Cannot allocate memory (triag)\n");
        internal_pid_new = -1;
        break;
      }
      triag->tex_page = 0x0;  /* default: textured */
      triag->vidx[0] = vidx_1st + vert_idxs[j * 3 + 0];
      triag->vidx[1] = vidx_1st + vert_idxs[j * 3 + 1];
      triag->vidx[2] = vidx_1st + vert_idxs[j * 3 + 2];
      triag->flag = 0x000;  /* default */
      memcpy(triag->U, vert_texcoords + j * 6 + 0, 3 * sizeof(float));
      memcpy(triag->V, vert_texcoords + j * 6 + 3, 3 * sizeof(float));
    }
    if (internal_pid_new < 0)
      break;

    /* Add vertices */
    if (!FCELIB_TYPES_AddVerticesToPart(part, part->PNumVertices))
    {
      internal_pid_new = -1;
      break;
    }
    if (mesh->vertices_len < vidx_1st + part->PNumVertices)
    {
      if (!FCELIB_TYPES_AddVerticesToMesh(mesh, vidx_1st + part->PNumVertices - mesh->vertices_len))
      {
        /* this is a fatal error */
        fprintf(stderr, "GeomDataToNewPart: Cannot add vertices\n");
        internal_pid_new = -1;
        break;
      }
    }
    mesh->hdr.NumVertices += part->PNumVertices;

    for (j = 0; j < part->PNumVertices; ++j)
    {
      part->PVertices[j] = vidx_1st + j;

      mesh->vertices[vidx_1st + j] = (FcelibVertex *)malloc(sizeof(**mesh->vertices));
      vert = mesh->vertices[vidx_1st + j];
      if (!vert)
      {
        fprintf(stderr, "GeomDataToNewPart: Cannot allocate memory (vert)\n");
        internal_pid_new = -1;
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
      vert->Animation = 0x0;  /* default: not immovable */
    }
    if (internal_pid_new < 0)
      break;

    pid_new = FCELIB_TYPES_GetOrderByInternalPartIdx(mesh, mesh->hdr.Parts[internal_pid_new]);
    if (pid_new < 0)
    {
      /* this is inconvenient but not a fatal error */
      printf("Warning:GeomDataToNewPart: Cannot get new part idx\n");
      break;
    }
    break;
  }

  return pid_new;
}

#endif  /* FCELIB_IO_H_ */
