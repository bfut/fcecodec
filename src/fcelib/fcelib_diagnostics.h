/*
  fcelib_diagnostics.h
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

#ifndef FCELIB_DIAGNOSTICS_H_
#define FCELIB_DIAGNOSTICS_H_

#include <stdio.h>

#include "./fcelib_types.h"

/* dev -------------------------------------------------------------------------------------------------------------- */

/* Debug: Prints ref'ed global part indexes. */
void FCELIB_DIAGNOSTICS_PrintMeshParts(const FcelibMesh * const mesh)
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
void FCELIB_DIAGNOSTICS_PrintMeshTriangles(const FcelibMesh * const mesh)
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
void FCELIB_DIAGNOSTICS_PrintMeshVertices(const FcelibMesh * const mesh)
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

#endif  /* FCELIB_DIAGNOSTICS_H_ */
