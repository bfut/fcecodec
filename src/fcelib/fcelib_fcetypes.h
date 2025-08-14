/*
  fcelib_fcetypes.h
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
  documents FCE format.
  implements FCE types with format validations.

FCE4/FCE4M format theoretical limits (assuming signed int):
  min filesize:            0x2038, except FCE3: 0x1F04
  max Reserve6offset:      2147483647 0x7FFFFFFF
  triangle size:           56         0x38
  vertice size:            12         0xC
  max triangle count:      38347921   0x2492491 (3 verts)
  max vert count:          4880644    0x4A7904 (x triags, 3*x verts)
  min triangle count:      0
  min vert count:          0
**/

#ifndef FCELIB_FCETYPES_H_
#define FCELIB_FCETYPES_H_

#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "./fcelib_util.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __cplusplus
typedef struct tVector tVector;
typedef struct tColor3 tColor3;
typedef struct tColor4 tColor4;
typedef struct FceHeader3 FceHeader3;
typedef struct FceHeader4 FceHeader4;
#endif

/*
  FCE3   tTriangle->flag   4 bit
      0x0   default             body parts: reflection
0:    0x1   matte (no chrome)
1:    0x2   high chrome
2:    0x4   no cull             two-faced triangle
3:    0x8   semi-transparent    all parts

5 = 1 + 4
6 = 2 + 4
A = 2 + 8
E = 2 + 4 + 8

Name    Application example                     Application example
0x000     car.fce   body
0x001     car.fce   underbody
0x002     car.fce   windows
0x004     car.fce
0x005     car.fce
0x006     car.fce
0x008     car.fce
0x00A     car.fce   semi-transparent windows
0x00E     car.fce   semi-transparent windows
0x00F     car.fce

A triangle is visible behind a semi-transparent triangle, if its index is smaller.
*/

/*
  FCE4   tTriangle->flag   12 bit
      0x0000   default             body parts: reflection,
                                   interior etc.: no reflection
0:    0x0001   matte (no chrome)
1:    0x0002   high chrome         body parts: used for windows etc.
2:    0x0004   no cull             two-faced triangle
3:    0x0008   semi-transparent    body parts: used for windows etc.

4:    0x0010   ?                   elni/car.fce :OH :OD :OLM :ORM :H**W :M**W, partial :HB :MB :LB :TB
5:    0x0020   all windows
6:    0x0040   front window
7:    0x0080   left window

8:    0x0100   back window
9:    0x0200   right window
10:   0x0400   broken window
11:   0x0800   ?
  FCE4M  tTriangle->flag   13 bit
12:   0x1000   ?                   <model>/part.fst

car.fce   body               default
car.fce   underbody          no reflection
car.fce   body roof          no cull
car.fce   windows            all windows + high chrome + no cull + semi-transparent
dash.fce  mirror glass       high chrome + semi-transparent

6 = 2 + 4
A = 2 + 8
E = 2 + 4 + 8

Name    Application example                     Application example
0x000   car.fce   body                          dash.fce  not mirror glass
0x001   car.fce   underbody
0x002
0x004   car.fce   body roof
0x005   car.fce
0x006
0x008   car.fce
0x00A                                           dash.fce  mirror glass
0x00E
0x00F
0x022   car.fce   right/left mirror glass
0x06A   car.fce   elni engine glass cover
0x06E   car.fce   front window
0x46E   car.fce   front window broken
0x0AE   car.fce   left window
0x4AE   car.fce   left window broken
0x12E   car.fce   back window
0x52E   car.fce   back window broken
0x22E   car.fce   right window
0x62E   car.fce   right window broken
*/

/* Length = 56. Vertex indices are local. Values from 'P1stVertices' make them global.

  Each vertex index points to a position and a normal. (FCE4/FCE4M: also damage position and damage normal)
  Vert positions and normals are stored in global coordinates.
  Vert positions are offset by their part positions, respectively. Normals are not offset.
*/
#if 0
struct tTriangle {
/* 0x00 */  int   tex_page;        /* Texture page number; > 0 in FCE3/FCE4 officer models, FCE4 pursuit road objects, FCE4M damage textures. Requires NumArts=max(tex_pages)-1 apart from the last */
/* 0x04 */  int   vidx[3];         /* Vertices local index */
/* 0x10 */  char  unknown[3 * 4];  /* all items = 0xFF00 or 0xFFFF */
/* 0x1C */  int   flag;            /* triangle flag */
/* 0x20 */  float U[3];            /* Vertices texture U-coordinates */
/* 0x2C */  float V[3];            /* Vertices texture V-coordinates */
};
#endif

struct tVector {
  float x;  /* x->inf is to the right */
  float y;  /* y->inf is up */
  float z;  /* z->inf is to the front */
};

/*
  Valid values for all four components: 0..255
  hue<degrees>  / 360 * 255
  saturation<%> / 100 * 255
  brightness<%> / 100 * 255
*/
struct tColor3 {
  int hue;
  int saturation;
  int brightness;
  int transparency;
};

struct tColor4 {
  unsigned char hue;
  unsigned char saturation;
  unsigned char brightness;
  unsigned char transparency;
};

/* 0x1F04  -  size of this header */
struct FceHeader3 {
/* 0x0000 */  int      Unknown1           ;  /* nullable, sometimes 0x13101000 (ex. render/pc/cone.fce) */
/* 0x0004 */  int      NumTriangles       ;  /* Number of triangles in model */
/* 0x0008 */  int      NumVertices        ;  /* Number of vertices in model */
/* 0x000C */  int      NumArts            ;  /* Number of arts, == 1 unless non-zero tex_pages are used */
                    /* offsets from 0x1F04 */
/* 0x0010 */  int      VertTblOffset      ;  /* usually 0x00. len() = 12 * NumVertices */
/* 0x0014 */  int      NormTblOffset      ;  /* len() = len(VertTbl) */
/* 0x0018 */  int      TriaTblOffset      ;  /* len() = 56 * NumTriangles */

/* 0x001C */  int      Reserve1offset     ;  /* len() = 32 * NumVertices */
/* 0x0020 */  int      Reserve2offset     ;  /* len() = len(VertTbl) */
/* 0x0024 */  int      Reserve3offset     ;  /* len() = len(VertTbl) */

/* 0x0028 */  float    HalfSize[3]        ;  /* X,Y,Z half-size width of whole model, defines bounding box for collision detection */

/* 0x0034 */  int      NumDummies         ;  /* Number of light sources 0..16 */
/* 0x0038 */  float    Dummies[16 * 3]    ;  /* Coordinates of dummies */

/* 0x00F8 */  int      NumParts           ;  /* Number of car parts 0..64 */
/* 0x00FC */  float    PartPos[64 * 3]    ;  /* Global position of car parts */

/* 0x03FC */  int      P1stVertices [64]  ;  /* First vertex index for each part */
/* 0x04FC */  int      PNumVertices [64]  ;  /* Number of vertices for each part */

/* 0x05FC */  int      P1stTriangles[64]  ;  /* First triangle index for each part */
/* 0x06FC */  int      PNumTriangles[64]  ;  /* Number of triangles for each part */

/* 0x07FC */  int      NumPriColors       ;  /* Number of primary colors 0..16 */
/* 0x0800 */  tColor3  PriColors[16]      ;  /* Primary colors */
/* 0x0900 */  int      NumSecColors       ;  /* Number of secondary colors 0..16 */
/* 0x0904 */  tColor3  SecColors[16]      ;  /* Secondary colors */

/* 0x0A04 */  char     DummyNames[16 * 64];  /* Dummy object names (ASCIIZ, role only by name) */

/* 0x0E04 */  char     PartNames [64 * 64];  /* Part names (ASCIIZ, role only by order) */

/* 0x1E04 */  char     Unknown2[256]      ;  /* nullable */
};

static
const char *kFce3PartsNames[FCELIB_UTIL_Fce3PartsImplemented] = {
  "high body",            /* car.fce */
  "left front wheel",
  "right front wheel",
  "left rear wheel",
  "right rear wheel",
  "medium body",
  "medium r front wheel",
  "medium l front wheel",
  "medium r rear wheel",
  "medium l rear wheel",
  "small body",
  "tiny body",
  "high headlights"
};

/*
car.fce (FCE3)
Part role is determined by order, listed names are canonical but optional (and
in fact nullable). Existence of parts is optional.
NB1: front wheel order is different for high body/medium body
NB2: only part 12 can be hidden lights
Name/Description         Idx  UsesFlag  Light  Reflection
high body                0    Y         N      Y
left front wheel         1    N         N      N
right front wheel        2    N         N      N
left rear wheel          3    N         N      N
right rear wheel         4    N         N      N
medium body              5    Y         N      Y
medium r front wheel     6    N         N      N
medium l front wheel     7    N         N      N
medium r rear wheel      8    N         N      N
medium l rear wheel      9    N         N      N
small body               10   Y         N
tiny body                11   N         N      N
high headlights          12   Y         Y      Y

cop#.fce (FCE3) - officer (# = 0-4)
Description              Idx  UsesFlag  Light  Reflection
officer                  0

cone.fce (FCE3) - traffic cone, unused
Name/Description         Idx  HasFlag  Light  Reflection
ConeH                    0    Y
ConeM                    1    Y
ConeL                    2    Y

go0#.fce (FCE3) - track menu model (# = 0-8)
Name/Description         Idx  UsesFlag  Light  Reflection
Loft01                   0    Y
Loft02                   1    Y

gengo.viv->go00.fce (FCE3) - developer logo, unused
Name/Description         Idx  UsesFlag  Light  Reflection
Line06                   0    Y
*/

/* 0x2038  -  size of this header */
struct FceHeader4 {
/* 0x0000 */  int      Version             ;  /* FCE4: 0x00101014, FCE4M: 0x00101015 */
/* 0x0004 */  int      Unknown1            ;  /* nullable */
/* 0x0008 */  int      NumTriangles        ;  /* Number of triangles in model */
/* 0x000C */  int      NumVertices         ;  /* Number of vertices in model */
/* 0x0010 */  int      NumArts             ;  /* Number of arts, == 1 (FCE4: unless non-zero tex_pages are used) */
                    /* offsets from 0x2038 */
/* 0x0014 */  int      VertTblOffset       ;  /* usually 0x0000. len() = 12 * NumVertices */
/* 0x0018 */  int      NormTblOffset       ;  /* len() = len(VertTbl) */
/* 0x001C */  int      TriaTblOffset       ;  /* len() = 56 * NumTriangles */

/* 0x0020 */  int      Reserve1offset      ;  /* len() = 32 * NumVertices */
/* 0x0024 */  int      Reserve2offset      ;  /* len() = len(VertTbl) */
/* 0x0028 */  int      Reserve3offset      ;  /* len() = len(VertTbl) */

/* 0x002C */  int      UndamgdVertTblOffset;  /* UndamgdVertTbl should be copy of VertTbl, unused in FCE4 */
/* 0x0030 */  int      UndamgdNormTblOffset;  /* UndamgdNormTbl should be copy of NormTbl, unused in FCE4 */
/* 0x0034 */  int      DamgdVertTblOffset  ;  /* len() = len(VertTbl) */
/* 0x0038 */  int      DamgdNormTblOffset  ;  /* len() = len(VertTbl) */

/* 0x003C */  int      Reserve4offset      ;  /* len() = 4 * NumVertices, null */
/* 0x0040 */  int      AnimationTblOffset  ;  /* len() = 4 * NumVertices, flags (0x4 = immovable, 0x0 othw) */
/* 0x0044 */  int      Reserve5offset      ;  /* len() = 4 * NumVertices, null */

/* 0x0048 */  int      Reserve6offset      ;  /* len() = 12 * NumTriangles, null
                                                FCE4M: len() += NumVertices */
/* 0x004C */  float    HalfSize[3]         ;  /* X,Y,Z half-size width of whole model, defines bounding box for collision detection */

/* 0x0058 */  int      NumDummies          ;  /* Number of light sources */
/* 0x005C */  float    Dummies[16 * 3]     ;  /* Coordinates of dummies */

/* 0x011C */  int      NumParts            ;  /* Number of car parts */
/* 0x0120 */  float    PartPos[64 * 3]     ;  /* Global position of car parts */

/* 0x0420 */  int      P1stVertices[64]    ;  /* First vertex index for each part */
/* 0x0520 */  int      PNumVertices[64]    ;  /* Number of vertices for each part */

/* 0x0620 */  int      P1stTriangles[64]   ;  /* First triangle index for each part */
/* 0x0720 */  int      PNumTriangles[64]   ;  /* Number of triangles for each part */

/* 0x0820 */  int      NumColors           ;  /* FCE4: Number of colors 0..16
                                                FCE4M: unknown purpose */
/* 0x0824 */  tColor4  PriColors[16]       ;  /* Primary colors */
/* 0x0864 */  tColor4  IntColors[16]       ;  /* Interior colors */
/* 0x08A4 */  tColor4  SecColors[16]       ;  /* Secondary colors */
/* 0x08E4 */  tColor4  DriColors[16]       ;  /* Driver hair colors */

/* 0x0924 */  int      Unknown3            ;  /* FCE4: nullable; FCE4M: unknown purpose, nullable */
/* 0x0928 */  char     Unknown2[256]       ;  /* nullable */

/* 0x0A28 */  char     DummyNames[16 * 64] ;  /* Dummy object names (ASCIIZ, role only by name) */

/* 0x0E28 */  char     PartNames [64 * 64] ;  /* Part names (ASCIIZ, role only by name) */

/* 0x1E28 */  char     Unknown4[528]       ;  /* nullable */
};

static
const char *kFce4HiBodyParts[FCELIB_UTIL_Fce4PartsHighBody] = {
  ":HB",            /* car.fce */
  ":OT",
  ":OL",
  ":OS",
  ":OLB",
  ":ORB",
  ":OLM",
  ":ORM",
  ":OC",
  ":ODL",
  ":OH",
  ":OD",
  ":HLFW",
  ":HRFW",
  ":HLMW",
  ":HRMW",
  ":HLRW",
  ":HRRW",
};

#ifdef __cplusplus
}  /* extern "C" */
#endif

/*
car.fce (FCE4)
":HB" is the only mandatory part
Name    Description              Damage  FallOf  UsesFlag  Light  Animated   Pursuit
:HB     high body                Y       N       Y         N      N          N
:MB     mid body                 Y       N       Y         N      N          N
:LB     low body                 Y       N       Y         N      N          N
:TB     tiny body                N       N       N         N      N          N
:OT     top                      Y       N       Y         N      N          N
:OL     hidden lights (HB/MB)    Y       N       Y         Y      N          N
:OS     spoiler                  N       N       Y         N      special    N        enabled via carp.txt
:OLB    left brake front  (HB)   N       N       N         N      Y          N
:ORB    right brake front (HB)   N       N       N         N      Y          N
:OLM    left mirror       (HB)   N       Y       Y         N      N          N
:ORM    right mirror      (HB)   N       Y       Y         N      N          N
:OC     console, interior        N       N       N         N      N          N
:ODL    dashboard lights         N       N       N         Y      N          N
:OH     driver head              N       N       N         N      special    N        except flagged verts
:OD     driver, chair, wheel     N       N       N         N      special    N        except flagged verts
:OND    no driver, chair, wheel  N       N       N         N      N          Y
:HLFW   high left front wheel    N       N       N         N      Y          N
:HRFW   high right front wheel   N       N       N         N      Y          N
:HLMW   high left middle wheel   N       N       N         N      Y          N
:HRMW   high right middle wheel  N       N       N         N      Y          N
:HLRW   high left rear wheel     N       N       N         N      Y          N
:HRRW   high right rear wheel    N       N       N         N      Y          N
:MLFW   mid left front wheel     N       N       N         N      Y          N
:MRFW   mid right front wheel    N       N       N         N      Y          N
:MLMW   mid left middle wheel    N       N       N         N      Y          N
:MRMW   mid right middle wheel   N       N       N         N      Y          N
:MLRW   mid left rear wheel      N       N       N         N      Y          N
:MRRW   mid right rear wheel     N       N       N         N      Y          N

cop.fce (FCE4) - officer, in pursuit car.viv
Name        UsesFlag  Light  Reflection
<partname>  Y                N

hel.fce (FCE4) - helicopter
Name    Description     UsesFlag  Animated
'main'  rotor           Y         Y
'tail'  tail rotor      Y         Y
'body'  body            Y         N
:LB     low body        Y
:Lmain  low rotor       Y
:Ltail  low tail rotor  Y

<track>.fce (FCE4) - track menu model (trkgo.viv) - role by name
Name     Description       UsesFlag  Animated
DIAMOND                    N
TRACK0                     N
TRACK1                     N
TRACK2                     N
TRACK3   (all except gt3)  N

track.fce (FCE4M) - track menu model
Name        UsesFlag  Animated
<partname>

part.fce (FCE4M) - car
":PPLFwheel" and ":PPLRwheel" are the only mandatory parts
Name              Description                   Damage  FallOf  UsesFlag  Light  Animated
:Hboards          running boards                Y       N       Y         N      N
:Hbody            high body                     Y       N       Y         N      N
:Hbumper          front & rear bumpers          Y       N       Y         N      N
:Hconvertible     top
:Hdashlight       dashboard lights
:Hfenderlight     lights, no fender             Y       N       Y         N      N
:Hffender         lights & front fender         Y       N       Y         N      N
:Hrfender         rear fender                   Y       N       Y         N      N
:Hfirewall
:Hheadlight       hidden lights                                           Y
:Hhood            engine hood
:Hhoodhole        engine hood w/ hole
:Hinterior        interior
:Hlbrake          left front brake
:Hrbrake          right front brake
:Hlmirror         left mirror                   N       Y       Y         N      N
:Hrmirror         right mirror                  N       Y       Y         N      N
:Hscoopfact       hood with factory scoop
:Hscoopsmall      hood scoop small
:Hscooplarge      hood scoop large
:Hskirt           rear wheel fender skirt
:Hskirtwell       rear wheel skirt enclosing
:Hsteer           steering wheel                N       N       N         N      Y
:Htrans           transmission (underbody)
:Hwheelwell       wheel wells (HB)
:Mwheelwell       wheel wells (MB)

:Hcage            roll cage
:Hcagechop        roll cage (chopped roof)
:Hshield          windshields
:Hshieldchop      windshields (chopped roof)
:Hswin            side windows
:Hswinchop        side windows (chopped roof)
:Htop             roof
:Htopchop         chopped roof

  FCE4M loads meshes for wheels, drivers, and enhanced parts from central files.
  They are positioned via the following dummy-parts:
Name              Description                                         FoundIn          LinksTo
:PPdriver                                                             *.viv->part.fce  DRIVER##.viv->part.fce
:PPLFwheel        left front wheel                                    *.viv->part.fce
:PPRFwheel        right front wheel                                   *.viv->part.fce
:PPLRwheel        left rear wheel                                     *.viv->part.fce
:PPRRwheel        right rear wheel                                    *.viv->part.fce
:PPaircleaner                                                         *.viv->part.fce
:PPcarb                                                               blowlarg.viv->part.fce
:PPAdash          on top of console left-hand                         *.viv->part.fce
:PPBdash          on top of console center                            *.viv->part.fce
:PPCdash          on hood in front of driver in front of windshield   *.viv->part.fce
:PPengine                                                             *.viv->part.fce
:PPLfog           left fog headlight                                  *.viv->part.fce
:PPRfog           right fog headlight                                 *.viv->part.fce
:PPfrontsusp      front suspension                                    *.viv->part.fce
:PPfuzzydice      fuzzy dice for rearview mirror                      *.viv->part.fce
:PPfuzzydicechop  rear mirror fuzzy dice (chopped roof)               *.viv->part.fce
:PPhoodorn        hood ornament                                       *.viv->part.fce
:PPhoodpivot      hood scoop                                          *.viv->part.fce
:PPlicense        rear license plate                                  *.viv->part.fce
:PPrearsusp       rear suspension                                     *.viv->part.fce
:PPLpipetip       left rear exhaust pipe                              *.viv->part.fce
:PPRpipetip       right rear exhaust pipe                             *.viv->part.fce
:PPLsidepipe      left side exhaust pipe                              *.viv->part.fce
:PPRsidepipe      right side exhaust pipe                             *.viv->part.fce
:PPsiren          roof siren (from cancelled pursuit mode)            *.viv->part.fce  siren.viv->part.fce
:PPspot           spotlight (from cancelled pursuit mode)             *.viv->part.fce  spotlite.viv->part.fce
:PPspoiler        rear spoiler                                        *.viv->part.fce  *.viv->spoiler.fce
:PPwheelie        wheelie bar                                         *.viv->part.fce

DRIVER##.viv/part.fce (FCE4M) - driver (## = 1-33)
Name              Description     Animated
:PHdriverhead     driver head     Y
:PHdriver         driver          special   except flagged verts

engine.viv/part.fce (FCE4M) - engine
:PHengine
:PPvalve
:PPmanifold

fog$$.viv/part.fce (FCE4M) - fog headlight ($$ = amber, white)
:PHfog

fuzzdice.viv/part.fce (FCE4M) - fuzzy dice for rearview mirror
:PHfuzzydice

hc##.viv/part.fce (FCE4M) - hubcap (## = 00 - 58)
hca##.viv/part.fce (FCE4M) - hubcap (## = 00 - 27)
rim##.viv/part.fce (FCE4M) - rim (## = 00 - 12)
rima##.viv/part.fce (FCE4M) - rim (## = 00 - 29)
:PHhubcap
:PMhubcap

lakeblok.viv/part.fce (FCE4M) -
:PHblockpipe

manidual.viv/part.fce (FCE4M) -
manihigh.viv/part.fce (FCE4M) -
manilow.viv/part.fce (FCE4M) -
manising.viv/part.fce (FCE4M) -
manitri.viv/part.fce (FCE4M) -

pipelake.viv/part.fce (FCE4M) -
pikeside.viv/part.fce (FCE4M) -

scooprec.viv/part.fce (FCE4M) -
scooptri.viv/part.fce (FCE4M) -

siren.viv/part.fce (FCE4M) - siren
:PHsiren

SLIKBLAC.viv/part.fce (FCE4M) -
SLIKLETT.viv/part.fce (FCE4M) -
SLIKRED.viv/part.fce (FCE4M) -
SLIKWHIT.viv/part.fce (FCE4M) -

spoiler.viv/part.fce (FCE4M) -
spotlite.viv/part.fce (FCE4M) -
stack#$.viv/part.fce (FCE4M) - (# = 1-4, 8  ; $ = fh, f, s)
susp.viv/part.fce (FCE4M) -
tach.viv/part.fce (FCE4M) -
tipcirc.viv/part.fce (FCE4M) -
tipcone.viv/part.fce (FCE4M) -
tipcyl.viv/part.fce (FCE4M) -
tiprect.viv/part.fce (FCE4M) -

TIREBLAC.viv/part.fce (FCE4M) -
TIRELETT.viv/part.fce (FCE4M) -
TIRERED.viv/part.fce (FCE4M) -
TIREWHIT.viv/part.fce (FCE4M) -

valvefh.viv/part.fce (FCE4M) -
valvev8.viv/part.fce (FCE4M) -
wheelie.viv/part.fce (FCE4M) - wheelie bar

names are collections of :tags - not unique, not cAsE-sEnSiTiVe
dash.fce                                            FallOf  UsesFlag  Light  Animated
:L_DM                     left driver mirror        Y       Y
:R_PM                     right passenger mirror    Y       Y
:B                        visible in back  view
:F                        visible in front view
:L                        visible in left  view
:R                        visible in right view
:S                        (used on roof)
:B_TOP                    roof
:L_TOP                    roof
:S_TOP                    roof
:F_DASH                   unlit dash                                  N
:R_DASH                   unlit dash                                  N
:F_LDASH                  lit dash                                    Y
:R_LDASH                  lit dash                                    Y
:F_MPH (0.0 to 0.55/60)   dial                                        N      Y
:F_RPM (0.0 to 0.55)      dial                                        N      Y
:F_W                      steering wheel                                     Y
:R_W                      steering wheel                                     Y

FCE4 only:
:R_DM                     right driver mirror       Y       Y
:L_PM                     left passenger mirror     Y       Y
:reflectdriver            nr34/car.viv
:reflectpassenger         nr34/car.viv

FCE4M only:
:S_DM                     side driver mirror
:S_PM                     side passenger mirror
:R_TOP                    roof
:L_W
:S_W
:L_DASH                   unlit dash                                  N
:S_DASH
:B_LDASH                  lit dash                                    Y
:F_LDASH                  lit dash                                    Y
:L_LDASH                  lit dash                                    Y
:L_MPH (0.0 to 0.55/110)  dial                                               Y
:R_MPH (0.0 to 0.55/110)  dial                                               Y
:F_SHIFT                  shifter
*/

/*
car.fce - light objects (FCE3)
KDSF, KDSFU - Components: char kind, direction, side, flashing, unknown;
Valid values:
  K : "H" (Headlights); "T" (Taillights); "S" (Siren);
  D : "F" (Front/White); "R" (Rear/Red); "M" (Mounted);
  S : "L" (Left); "R" (Right)
  F : "O" (Flashing at moment 1); "E" (Flashing at moment 2); "N" (No flashing)
  U : "N"  ex. corv/car.viv->car.fce, has "TRLN" and "TRLNN"
Colors (in-game):
HF__ : headlights, visible from front, white
TR__ : taillights, visible from rear, red
HR__ : taillights, visible from rear, red, with fog glare (broken behavior)
TF__ : headlights, visible from front, white, w/o fog glare (broken behavior)
S_L_ : siren, red
S_R_ : siren, blue
Taillights and Sirens never flash. Dummies may appear differently between the
"Player Car" menu, and in-game. Unknown dummies are ignored.

car.fce, hel.fce - light objects (FCE4) (FCE4M)
KCBFI, KCBFITD - Components: char kind, color/direction, breakable, flashing, intensity, time, delay;
Valid values:
  K : "H" (Headlights); "T" (Taillights); "B" (Brake light); "R" (Reverse light); "P" (Parking lights); "S" (Siren);
  C : "W" (White); "R" (Red); "B" (Blue); "O" (Orange); "Y" (Yellow)
  B : "Y" (Yes); "N" (No)
  F : "O" (Flashing at moment 1); "E" (Flashing at moment 2); "N" (No flashing)
  I : Number between 0 and 9 with 0 being broken (normal max 5)
  Next only used with flashing lights:
  T : Number between 1 and 9 with 9 being longest time and 0 being constant (normal max 5)
  D : Number between 0 and 9 with 9 being longest delay and 0 no delay (normal max 2)

car.fce - fx objects - only (FCE4)
:WATER         water effect
:SMOKE         smoke effect
:SNOW          snowplow effect
:LICENSE       narrow plate
:LICENSE_EURO  wide plate
:LICMED
:LICLOW

part.fce - only (FCE4M)
  K : "I" inner headlights, "M" marker;
:ENGINE
:EXHAUST

dash.fce - only (FCE4)
:Omni01        POV

dash.fce - role determined by order - only (FCE4M)
Name     Idx   Description
Omni01     0   POV
:Omni01    0   POV
           0   POV
:W_AXIS    1   far-end of steering column
*/

/* Get header --------------------------------------------------------------- */

/* Assumes input has length >= 0x1F04 */
void FCELIB_FCETYPES_GetFceHeader3(FceHeader3 *hdr, const unsigned char * const buf)
{
  int i;
#ifdef __cplusplus
  *hdr = {};
#else
  memset(hdr, 0, sizeof(*hdr));
#endif

  memcpy(&hdr->Unknown1, buf + 0x0000, 4);
  memcpy(&hdr->NumTriangles, buf + 0x0004, 4);
  memcpy(&hdr->NumVertices, buf + 0x0008, 4);
  memcpy(&hdr->NumArts, buf + 0x000C, 4);

  memcpy(&hdr->VertTblOffset, buf + 0x0010, 4);
  memcpy(&hdr->NormTblOffset, buf + 0x0014, 4);
  memcpy(&hdr->TriaTblOffset, buf + 0x0018, 4);

  memcpy(&hdr->Reserve1offset, buf + 0x001C, 4);
  memcpy(&hdr->Reserve2offset, buf + 0x0020, 4);
  memcpy(&hdr->Reserve3offset, buf + 0x0024, 4);

  memcpy(&hdr->HalfSize, buf + 0x0028, 3 * 4);

  memcpy(&hdr->NumDummies, buf + 0x0034, 4);
  memcpy(&hdr->Dummies, buf + 0x0038, SCL_clamp(hdr->NumDummies, 0, 16) * 3 * 4);

  memcpy(&hdr->NumParts, buf + 0x00F8, 4);
  memcpy(&hdr->PartPos, buf + 0x00FC, SCL_clamp(hdr->NumParts, 0, 64) * 3 * 4);

  memcpy(&hdr->P1stVertices, buf + 0x03FC, 64 * 4);
  memcpy(&hdr->PNumVertices, buf + 0x04FC, 64 * 4);
  memcpy(&hdr->P1stTriangles, buf + 0x05FC, 64 * 4);
  memcpy(&hdr->PNumTriangles, buf + 0x06FC, 64 * 4);

  memcpy(&hdr->NumPriColors, buf + 0x07FC, 4);
  for (i = 0; i < SCL_min(hdr->NumPriColors, 16); i++)
  {
    memcpy(&hdr->PriColors[i].hue,          buf + 0x0800 + i * 16 + 0x00, 4);
    memcpy(&hdr->PriColors[i].saturation,   buf + 0x0800 + i * 16 + 0x04, 4);
    memcpy(&hdr->PriColors[i].brightness,   buf + 0x0800 + i * 16 + 0x08, 4);
    memcpy(&hdr->PriColors[i].transparency, buf + 0x0800 + i * 16 + 0x0C, 4);
  }

  memcpy(&hdr->NumSecColors, buf + 0x0900, 4);
  for (i = 0; i < SCL_min(hdr->NumSecColors, 16); i++)
  {
    memcpy(&hdr->SecColors[i].hue,          buf + 0x0904 + i * 16 + 0x00, 4);
    memcpy(&hdr->SecColors[i].saturation,   buf + 0x0904 + i * 16 + 0x04, 4);
    memcpy(&hdr->SecColors[i].brightness,   buf + 0x0904 + i * 16 + 0x08, 4);
    memcpy(&hdr->SecColors[i].transparency, buf + 0x0904 + i * 16 + 0x0C, 4);
  }

  memcpy(&hdr->DummyNames, buf + 0x0A04, 64 * 16);
  FCELIB_UTIL_EnsureStrings(hdr->DummyNames, 16, 64);
  FCELIB_UTIL_UnprintableToNul(hdr->DummyNames, 16, 64);
  FCELIB_UTIL_TidyUpNames(hdr->DummyNames, hdr->NumDummies, 16, 64);

  memcpy(&hdr->PartNames, buf + 0x0E04, 64 * 64);
  FCELIB_UTIL_EnsureStrings(hdr->PartNames, 64, 64);
  FCELIB_UTIL_UnprintableToNul(hdr->PartNames, 64, 64);
  FCELIB_UTIL_TidyUpNames(hdr->PartNames, hdr->NumParts, 64, 64);

#if defined(SCL_DEBUG) && SCL_DEBUG > 0
  memcpy(&hdr->Unknown2, buf + 0x1E04, 256);
#endif
}

/* Assumes valid FCE data */
void FCELIB_FCETYPES_GetFceHeader4(FceHeader4 *hdr, const unsigned char * const buf)
{
  int i;
#ifdef __cplusplus
  *hdr = {};
#else
  memset(hdr, 0, sizeof(*hdr));
#endif

  memcpy(&hdr->Version, buf + 0x0000, 4);
  memcpy(&hdr->Unknown1, buf + 0x0004, 4);
  memcpy(&hdr->NumTriangles, buf + 0x0008, 4);
  memcpy(&hdr->NumVertices, buf + 0x000C, 4);
  memcpy(&hdr->NumArts, buf + 0x0010, 4);

  memcpy(&hdr->VertTblOffset, buf + 0x0014, 4);
  memcpy(&hdr->NormTblOffset, buf + 0x0018, 4);
  memcpy(&hdr->TriaTblOffset, buf + 0x001C, 4);

  memcpy(&hdr->Reserve1offset, buf + 0x0020, 4);
  memcpy(&hdr->Reserve2offset, buf + 0x0024, 4);
  memcpy(&hdr->Reserve3offset, buf + 0x0028, 4);

  memcpy(&hdr->UndamgdVertTblOffset, buf + 0x002C, 4);
  memcpy(&hdr->UndamgdNormTblOffset, buf + 0x0030, 4);
  memcpy(&hdr->DamgdVertTblOffset, buf + 0x0034, 4);
  memcpy(&hdr->DamgdNormTblOffset, buf + 0x0038, 4);

  memcpy(&hdr->Reserve4offset, buf + 0x003C, 4);
  memcpy(&hdr->AnimationTblOffset, buf + 0x0040, 4);
  memcpy(&hdr->Reserve5offset, buf + 0x0044, 4);

  memcpy(&hdr->Reserve6offset, buf + 0x0048, 4);

  memcpy(&hdr->HalfSize, buf + 0x004C, 3 * 4);

  memcpy(&hdr->NumDummies, buf + 0x0058, 4);
  memcpy(&hdr->Dummies, buf + 0x005C, SCL_clamp(hdr->NumDummies, 0, 16) * 3 * 4);

  memcpy(&hdr->NumParts, buf + 0x011C, 4);
  memcpy(&hdr->PartPos, buf + 0x0120, SCL_clamp(hdr->NumParts, 0, 64) * 3 * 4);

  memcpy(&hdr->P1stVertices, buf + 0x0420, 64 * 4);
  memcpy(&hdr->PNumVertices, buf + 0x0520, 64 * 4);
  memcpy(&hdr->P1stTriangles, buf + 0x0620, 64 * 4);
  memcpy(&hdr->PNumTriangles, buf + 0x0720, 64 * 4);

  memcpy(&hdr->NumColors, buf + 0x0820, 4);
  for (i = 0; i < SCL_min(hdr->NumColors, 16); i++)
  {
    memcpy(&hdr->PriColors[i].hue,          buf + 0x0824 + i * 4 + 0, 1);
    memcpy(&hdr->PriColors[i].saturation,   buf + 0x0824 + i * 4 + 1, 1);
    memcpy(&hdr->PriColors[i].brightness,   buf + 0x0824 + i * 4 + 2, 1);
    memcpy(&hdr->PriColors[i].transparency, buf + 0x0824 + i * 4 + 3, 1);

    memcpy(&hdr->IntColors[i].hue,          buf + 0x0864 + i * 4 + 0, 1);
    memcpy(&hdr->IntColors[i].saturation,   buf + 0x0864 + i * 4 + 1, 1);
    memcpy(&hdr->IntColors[i].brightness,   buf + 0x0864 + i * 4 + 2, 1);
    memcpy(&hdr->IntColors[i].transparency, buf + 0x0864 + i * 4 + 3, 1);

    memcpy(&hdr->SecColors[i].hue,          buf + 0x08A4 + i * 4 + 0, 1);
    memcpy(&hdr->SecColors[i].saturation,   buf + 0x08A4 + i * 4 + 1, 1);
    memcpy(&hdr->SecColors[i].brightness,   buf + 0x08A4 + i * 4 + 2, 1);
    memcpy(&hdr->SecColors[i].transparency, buf + 0x08A4 + i * 4 + 3, 1);

    memcpy(&hdr->DriColors[i].hue,          buf + 0x08E4 + i * 4 + 0, 1);
    memcpy(&hdr->DriColors[i].saturation,   buf + 0x08E4 + i * 4 + 1, 1);
    memcpy(&hdr->DriColors[i].brightness,   buf + 0x08E4 + i * 4 + 2, 1);
    memcpy(&hdr->DriColors[i].transparency, buf + 0x08E4 + i * 4 + 3, 1);
  }

  memcpy(&hdr->Unknown3, buf + 0x0924, 4);
#if defined(SCL_DEBUG) && SCL_DEBUG > 0
  memcpy(&hdr->Unknown2, buf + 0x0928, 256);
#endif

  memcpy(&hdr->DummyNames, buf + 0x0A28, 64 * 16);
  FCELIB_UTIL_UnprintableToNul(hdr->DummyNames, 16, 64);
  FCELIB_UTIL_TidyUpNames(hdr->DummyNames, hdr->NumDummies, 16, 64);

  memcpy(&hdr->PartNames, buf + 0x0E28, 64 * 64);
  FCELIB_UTIL_UnprintableToNul(hdr->PartNames, 64, 64);
  FCELIB_UTIL_TidyUpNames(hdr->PartNames, hdr->NumParts, 64, 64);

#if defined(SCL_DEBUG) && SCL_DEBUG > 0
  memcpy(&hdr->Unknown4, buf + 0x1E28, 528);
#endif
}

/* Fce3 validation ---------------------------------------------------------- */

/* Bound-checks counts and offsets from FCE data */
int FCELIB_FCETYPES_MiniValidateHdr3(const unsigned char * const buf)
{
  int retv = 1;
  int tmp;
  int i;
  const int kHdrPos3[8] = {
    0x0004, 0x0008,
    0x0010, 0x0014, 0x0018,
    0x001C, 0x0020, 0x0024
  };
  for (i = 0; i < 8; ++i)
  {
    memcpy(&tmp, buf + kHdrPos3[i], sizeof(tmp));
    if ((tmp < INT_MIN / 80) || (tmp > INT_MAX / 80))
    {
      fprintf(stderr, "MiniValidateHdr3: Invalid value at %#06x (%d)\n", kHdrPos3[i], tmp);
      retv = 0;
    }
  }
  return retv;
}

int FCELIB_FCETYPES_Fce3ComputeSize(const int NumVertices, const int NumTriangles)
{
  int fsize = 0;
  fsize += 0x1F04;             /* FCE3 header size */
  fsize += 80 * NumVertices;   /* ((4*12) + 32) x NumVertices */
  fsize += 56 * NumTriangles;  /* 56 x NumTriangles*/
  return fsize;
}

/* Assumes sizeof(*buf) >= 0x1F04. Returns boolean. */
int FCELIB_FCETYPES_Fce3ValidateHeader(const FceHeader3 *hdr, const void * const buf, const int fce_size)
{
  int retv = 1;
  int i;
  int count_verts = 0;
  int count_triags = 0;
  int size;
  int dist_to_eof;

  /* parts: triangle indices within bounds?
   */

  for (;;)
  {
    if (!FCELIB_FCETYPES_MiniValidateHdr3((const unsigned char *)buf))
      retv = 0;

    if (hdr->NumTriangles < 0)
    {
      fprintf(stderr, "Fce3ValidateHeader: Invalid number of triangles (%d)\n", hdr->NumTriangles);
      retv = 0;
    }

    if (hdr->NumVertices < 0)
    {
      fprintf(stderr, "Fce3ValidateHeader: Invalid number of vertices (%d)\n", hdr->NumVertices);
      retv = 0;
    }

    if ((hdr->NumDummies > 16) || (hdr->NumDummies < 0))
    {
      fprintf(stderr, "Fce3ValidateHeader: Invalid number of dummies (%d)\n", hdr->NumDummies);
      retv = 0;
    }

    if ((hdr->NumParts > 64) || (hdr->NumParts < 0))
    {
      fprintf(stderr, "Fce3ValidateHeader: Invalid number of parts (%d)\n", hdr->NumParts);
      retv = 0;
    }

    if ((hdr->NumPriColors > 16) || (hdr->NumPriColors < 0))
    {
      fprintf(stderr, "Fce3ValidateHeader: Invalid number of primary colors (%d)\n",
                      hdr->NumPriColors);
      retv = 0;
    }
    if ((hdr->NumSecColors > 16) || (hdr->NumSecColors < 0))
    {
      fprintf(stderr, "Fce3ValidateHeader: Invalid number of secondary colors (%d)\n",
                      hdr->NumSecColors);
      retv = 0;
    }

    /* Vertices, triangles counts */
    for (i = 0; i < SCL_min(64, hdr->NumParts); ++i)
    {
      if ((hdr->PNumTriangles[i] > 0) && (hdr->PNumVertices[i] < 3))
      {
        fprintf(stderr, "Fce3ValidateHeader: Part %d requires at least 3 vertices in total, found %d\n",
                        i, hdr->PNumVertices[i]);
        retv = 0;
      }
      if ((hdr->PNumTriangles[i] < 0) || (hdr->PNumTriangles[i] > INT_MAX - count_triags) ||
               (hdr->PNumVertices[i] < 0) || (hdr->PNumVertices[i] > INT_MAX - count_verts))
      {
        fprintf(stderr, "Fce3ValidateHeader: Part %d number of triangles (%d) or vertices (%d) out of bounds.\n",
                        i, hdr->PNumTriangles[i], hdr->PNumVertices[i]);
        retv = 0;
        break;
      }

      count_verts += hdr->PNumVertices[i];
      count_triags += hdr->PNumTriangles[i];
    }
    if (hdr->NumVertices < count_verts)
    {
      fprintf(stderr, "Fce3ValidateHeader: Expects %d vertices in total, found %d\n",
                      hdr->NumVertices, count_verts);
      retv = 0;
    }
    if (hdr->NumTriangles < count_triags)
    {
      fprintf(stderr, "Fce3ValidateHeader: Expects %d triangles in total, found %d\n",
                      hdr->NumTriangles, count_triags);
      retv = 0;
    }
    if ((size = FCELIB_FCETYPES_Fce3ComputeSize(count_verts, count_triags)) > fce_size)
    {
      fprintf(stderr, "Fce3ValidateHeader: FCE filesize too small %d (requires %d) %d\n",
                      size,
                      fce_size, fce_size - size);
      retv = 0;
    }
    size = 0;

    /*
      Vertices, triangles areas: parts non-overlapping, within bounds (do nothing
      when zero verts, triags)
    */
    for (i = 0; i < SCL_min(64, hdr->NumParts) - 1; ++i)
    {
      /*
        Combined with other checks, guarantees verts, triags stay within their
        areas, respectively
      */
      if ((hdr->P1stVertices[i] < 0) ||
          (hdr->P1stVertices[i] + hdr->PNumVertices[i] > hdr->NumVertices))
      {
        fprintf(stderr, "Fce3ValidateHeader: Part out of bounds %d (vertices)\n", i);
        retv = 0;
        break;
      }
      if (hdr->P1stVertices[i] + hdr->PNumVertices[i] > hdr->P1stVertices[i + 1])
      {
        fprintf(stderr, "Fce3ValidateHeader: Overlapping parts %d, %d (vertices)\n", i, i + 1);
        retv = 0;
        break;
      }

      if ((hdr->P1stTriangles[i] < 0) ||
          (hdr->P1stTriangles[i] + hdr->PNumTriangles[i] > hdr->NumTriangles))
      {
        fprintf(stderr, "Fce3ValidateHeader: Part out of bounds %d (triangles)\n", i);
        retv = 0;
        break;
      }
      if (hdr->P1stTriangles[i] + hdr->PNumTriangles[i] > hdr->P1stTriangles[i + 1])
      {
        fprintf(stderr, "Fce3ValidateHeader: Overlapping parts %d, %d (triangles)\n", i, i + 1);
        retv = 0;
        break;
      }
    }
    if ((hdr->NumParts > 0) && (hdr->NumParts <= 64))
    {
      if ((hdr->P1stVertices[hdr->NumParts - 1] < 0) ||
          (hdr->P1stVertices[hdr->NumParts - 1] + hdr->PNumVertices[hdr->NumParts - 1] > hdr->NumVertices))
      {
        fprintf(stderr, "Fce3ValidateHeader: Part out of bounds %d (vertices)\n", hdr->NumParts - 1);
        retv = 0;
      }

      if ((hdr->P1stTriangles[hdr->NumParts - 1] < 0) ||
          (hdr->P1stTriangles[hdr->NumParts - 1] + hdr->PNumTriangles[hdr->NumParts - 1] > hdr->NumTriangles))
      {
        fprintf(stderr, "Fce3ValidateHeader: Part out of bounds %d (triangles)\n", hdr->NumParts - 1);
        retv = 0;
      }
    }

    /*
      Validate filesize, area offsets, area sizes, areas non-overlapping

      FCE3 allows NumVertices, and PNumTriangles to be larger than the truth.
      Fcecodec requires that area sizes relate to given values, which FCE3 allows
      (FCE3 is even less restrictive).

      Note: Fcecodec warns about, accepts (VertTblOffset > 0)
    */
    if ((size = FCELIB_FCETYPES_Fce3ComputeSize(hdr->NumVertices, hdr->NumTriangles)) != fce_size)
    {
      fprintf(stderr, "Fce3ValidateHeader: FCE filesize mismatch %d (expects %d) %d\n",
                      fce_size,
                      size, fce_size - size);
      retv = 0;
    }

    dist_to_eof = 0;
    dist_to_eof += 12 * hdr->NumVertices;
    if ((hdr->Reserve3offset < 0) || (fce_size - 0x1F04 - hdr->Reserve3offset) != dist_to_eof)
    {
      fprintf(stderr, "Fce3ValidateHeader: Reserve3offset invalid 0x%04x (expects 0x%04x)\n",
                      hdr->Reserve3offset, dist_to_eof);
      retv = 0;
    }
    dist_to_eof += 12 * hdr->NumVertices;
    if ((hdr->Reserve2offset < 0) || (fce_size - 0x1F04 - hdr->Reserve2offset) != dist_to_eof)
    {
      fprintf(stderr, "Fce3ValidateHeader: Reserve2offset invalid 0x%04x (expects 0x%04x)\n",
                      hdr->Reserve2offset, dist_to_eof);
      retv = 0;
    }
    dist_to_eof += 32 * hdr->NumVertices;
    if ((hdr->Reserve1offset < 0) || (fce_size - 0x1F04 - hdr->Reserve1offset) != dist_to_eof)
    {
      fprintf(stderr, "Fce3ValidateHeader: Reserve1offset invalid 0x%04x (expects 0x%04x)\n",
                      hdr->Reserve1offset, dist_to_eof);
      retv = 0;
    }

    dist_to_eof += 56 * hdr->NumTriangles;
    if ((hdr->TriaTblOffset < 0) || (fce_size - 0x1F04 - hdr->TriaTblOffset) != dist_to_eof)
    {
      fprintf(stderr, "Fce3ValidateHeader: TriaTblOffset invalid 0x%04x (expects 0x%04x)\n",
                      hdr->TriaTblOffset, dist_to_eof);
      retv = 0;
    }
    dist_to_eof += 12 * hdr->NumVertices;
    if ((hdr->NormTblOffset < 0) || (fce_size - 0x1F04 - hdr->NormTblOffset) != dist_to_eof)
    {
      fprintf(stderr, "Fce3ValidateHeader: NormTblOffset invalid 0x%04x (expects 0x%04x)\n",
                      hdr->NormTblOffset, dist_to_eof);
      retv = 0;
    }
    dist_to_eof += 12 * hdr->NumVertices;
    if ((hdr->VertTblOffset < 0) || (fce_size - 0x1F04 - hdr->VertTblOffset) != dist_to_eof)
    {
      fprintf(stderr, "Fce3ValidateHeader: VertTblOffset invalid 0x%04x (expects 0x%04x)\n",
                      hdr->VertTblOffset, dist_to_eof);
      retv = 0;
    }

    /* warnings */
    if (retv)
    {
      if (hdr->NumVertices != count_verts)
      {
        fprintf(stderr, "Fce3ValidateHeader: Warning Expects %d vertices in total, found %d\n",
                        hdr->NumVertices, count_verts);
      }
      if (hdr->NumTriangles != count_triags)
      {
        fprintf(stderr, "Fce3ValidateHeader: Warning Expects %d triangles in total, found %d\n",
                        hdr->NumTriangles, count_triags);
      }
    }

    if (hdr->NumArts != 1)
      fprintf(stderr, "Fce3ValidateHeader: Warning NumArts != 1 (%d)\n", hdr->NumArts);

    if (hdr->VertTblOffset)
    {
      fprintf(stderr, "Fce3ValidateHeader: Warning VertTblOffset = 0x%04x (expects 0x0000)\n",
                      hdr->VertTblOffset);
    }

    if (hdr->NumPriColors < hdr->NumSecColors)
    {
      fprintf(stderr, "Fce3ValidateHeader: Warning NumPriColors < NumSecColors (%d, %d)\n",
                      hdr->NumPriColors, hdr->NumSecColors);
    }

    if ((hdr->HalfSize[0] < 0.001) || (hdr->HalfSize[2] < 0.001) ||
        (hdr->HalfSize[0] * hdr->HalfSize[2] < 0.1) ||
        (hdr->HalfSize[1] < 0.0))
    {
      fprintf(stderr, "Fce3ValidateHeader: Warning HalfSizes may crash game\n");
    }
    break;
  }

  return retv;
}

/* Fce4 validation ---------------------------------------------------------- */

/* Bound-checks counts and offsets from FCE data */
int FCELIB_FCETYPES_MiniValidateHdr4(const unsigned char * const buf)
{
  int retv = 1;
  int tmp;
  int i;
  const int kHdrPos4[16] = {
    0x0008, 0x000c,
    0x0014, 0x0018, 0x001C,
    0x0020, 0x0024, 0x0028,
    0x002c, 0x0030, 0x0034, 0x0038,
    0x003c, 0x0040, 0x0044,
    0x0048
  };
  for (i = 0; i < 16; ++i)
  {
    memcpy(&tmp, buf + kHdrPos4[i], sizeof(tmp));
    if ((tmp < INT_MIN / 140) || (tmp > INT_MAX / 140))
    {
      fprintf(stderr, "MiniValidateHdr4: Invalid value at %#06x (%d)\n", kHdrPos4[i], tmp);
      retv = 0;
    }
  }
  return retv;
}

int FCELIB_FCETYPES_Fce4ComputeSize(const int Version, const int NumVertices, const int NumTriangles)
{
  int fsize = 0;
  fsize += 0x2038;             /* FCE4 / FCE4M header size */
  fsize += 140 * NumVertices;  /* ((8*12) + 32 + (3*4)) x NumVertices */
  fsize += 68 * NumTriangles;  /* (56 + 12) x NumTriangles*/
  if (Version == 0x00101015)
    fsize += NumVertices;  /* Reserve6 is larger */
  return fsize;
}

float FCELIB_FCETYPES_GetWheelbase4M(const FceHeader4 *hdr, int *count_wheels)
{
  int i;
  float wheelbase = 0.0;
  *count_wheels = 0;
  for (i = 0; i < SCL_min(hdr->NumParts, 64); i++)
  {
    if (!strcmp(":PPLFwheel", hdr->PartNames + (i * 64)) || !strcmp(":PPLRwheel", hdr->PartNames + (i * 64)))
    {
      if (!*count_wheels)
      {
        wheelbase = hdr->PartPos[i * 3 + 2];
        *count_wheels = 1;
      }
      else
      {
        wheelbase = SCL_abs(hdr->PartPos[i * 3 + 2] - wheelbase);
        *count_wheels = 2;
        break;
      }
    }
  }
  if (*count_wheels < 2)
    return 0.0;
  else
    return wheelbase;
}

/* Assumes sizeof(*buf) = 0x2038. Returns boolean */
int FCELIB_FCETYPES_Fce4ValidateHeader(const FceHeader4 *hdr, const void * const buf, const int fce_size)
{
  int retv = 1;
  int i;
  int count_verts = 0;
  int count_triags = 0;
  int size;
  int dist_to_eof;

  for (;;)
  {
    if (!FCELIB_FCETYPES_MiniValidateHdr4((const unsigned char *)buf))
      retv = 0;

    if (hdr->NumTriangles < 0)
    {
      fprintf(stderr, "Fce4ValidateHeader: Invalid number of triangles (%d)\n", hdr->NumTriangles);
      retv = 0;
    }

    if (hdr->NumVertices < 0)
    {
      fprintf(stderr, "Fce4ValidateHeader: Invalid number of vertices (%d)\n", hdr->NumVertices);
      retv = 0;
    }

    if ((hdr->NumDummies > 16) || (hdr->NumDummies < 0))
    {
      fprintf(stderr, "Fce4ValidateHeader: Invalid number of dummies (%d is not in [0, 16]\n", hdr->NumDummies);
      retv = 0;
    }

    if ((hdr->NumParts > 64) || (hdr->NumParts < 0))
    {
      fprintf(stderr, "Fce4ValidateHeader: Invalid number of parts (%d)\n", hdr->NumParts);
      retv = 0;
    }

    if ((hdr->NumColors > 16) || (hdr->NumColors < 0))
    {
      /* FCE4M does not use colors and may allow invalid values */
      if (hdr->Version == 0x00101014)
      {
        fprintf(stderr, "Fce4ValidateHeader: Invalid number of colors (%d is not in [0, 16])\n", hdr->NumColors);
        retv = 0;
      }
      else if (hdr->Version == 0x00101015)
        printf("Fce4ValidateHeader: Warning: Invalid number of colors (%d is not in [0, 16])\n", hdr->NumColors);
    }

    /* Vertices, triangles counts */
    for (i = 0; i < SCL_min(64, hdr->NumParts); ++i)
    {
      if ((hdr->PNumTriangles[i] > 0) && (hdr->PNumVertices[i] < 3))
      {
        fprintf(stderr, "Fce4ValidateHeader: Part %d requires at least 3 vertices in total, found %d\n",
                        i, hdr->PNumVertices[i]);
        retv = 0;
      }
      if ((hdr->PNumTriangles[i] < 0) || (hdr->PNumTriangles[i] > INT_MAX - count_triags) ||
               (hdr->PNumVertices[i] < 0) || (hdr->PNumVertices[i] > INT_MAX - count_verts))
      {
        fprintf(stderr, "Fce4ValidateHeader: Part %d number of triangles (%d) or vertices (%d) out of bounds.\n",
                        i, hdr->PNumTriangles[i], hdr->PNumVertices[i]);
        retv = 0;
        break;
      }

      count_verts += hdr->PNumVertices[i];
      count_triags += hdr->PNumTriangles[i];
    }
    if (hdr->NumVertices < count_verts)
    {
      fprintf(stderr, "Fce4ValidateHeader: Expects %d vertices in total, found %d\n",
                      hdr->NumVertices, count_verts);
      retv = 0;
    }
    if (hdr->NumTriangles < count_triags)
    {
      fprintf(stderr, "Fce4ValidateHeader: Expects %d triangles in total, found %d\n",
                      hdr->NumTriangles, count_triags);
      retv = 0;
    }
    if (!retv)
    {
      break;
    }
    if ((size = FCELIB_FCETYPES_Fce4ComputeSize(hdr->Version, count_verts, count_triags)) > fce_size)
    {
      /* Are just Reserve5, Reserve6 invalid? ex. 99viper/?.fce */

      int area_5_6_size = 4 * hdr->NumVertices + 12 * hdr->NumTriangles;
      if (hdr->Version == 0x00101015)
        area_5_6_size += hdr->NumVertices;

      if (size - area_5_6_size > fce_size - abs(fce_size - 0x2038 - hdr->Reserve5offset))
      {
        fprintf(stderr, "Fce4ValidateHeader: FCE filesize mismatch %d (expects %d) %d\n",
                        fce_size,
                        size, fce_size - size);
        fprintf(stderr, "Fce4ValidateHeader: count_verts=%d , count_triags=%d\n", count_verts, count_triags);
        fprintf(stderr, "Fce4ValidateHeader: until 5: %d (expects %d) %d\n",
                        fce_size - abs(fce_size - 0x2038 - hdr->Reserve5offset),
                        size - area_5_6_size,
                        fce_size - abs(fce_size - 0x2038 - hdr->Reserve5offset) - (size - area_5_6_size));
        retv = 0;
      }
      else
      {
        fprintf(stderr, "Fce4ValidateHeader: Warning FCE filesize mismatch (Reserve5offset, Reserve6offset invalid)\n");
      }
    }
    size = 0;

    /*
      Vertices, triangles areas: parts non-overlapping, within bounds (do nothing
      when zero verts, triags)
    */
    for (i = 0; i < SCL_min(64, hdr->NumParts) - 1; ++i)
    {
      /*
        Combined with other checks, guarantees verts, triags stay within their
        areas, respectively
      */
      if ((hdr->P1stVertices[i] < 0) ||
          (hdr->P1stVertices[i] + hdr->PNumVertices[i] > hdr->NumVertices))
      {
        fprintf(stderr, "Fce4ValidateHeader: Part out of bounds %d (vertices)\n", i);
        retv = 0;
        break;
      }
      if (hdr->P1stVertices[i] + hdr->PNumVertices[i] > hdr->P1stVertices[i + 1])
      {
        fprintf(stderr, "Fce4ValidateHeader: Overlapping parts %d, %d (vertices)\n", i, i + 1);
        retv = 0;
        break;
      }

      if ((hdr->P1stTriangles[i] < 0) ||
          (hdr->P1stTriangles[i] + hdr->PNumTriangles[i] > hdr->NumTriangles))
      {
        fprintf(stderr, "Fce4ValidateHeader: Part out of bounds %d (triangles)\n", i);
        retv = 0;
        break;
      }
      if (hdr->P1stTriangles[i] + hdr->PNumTriangles[i] > hdr->P1stTriangles[i + 1])
      {
        fprintf(stderr, "Fce4ValidateHeader: Overlapping parts %d, %d (triangles)\n", i, i + 1);
        retv = 0;
        break;
      }
    }
    if ((hdr->NumParts > 0) && (hdr->NumParts <= 64))
    {
      if ((hdr->P1stVertices[hdr->NumParts - 1] < 0) ||
          (hdr->P1stVertices[hdr->NumParts - 1] + hdr->PNumVertices[hdr->NumParts - 1] > hdr->NumVertices))
      {
        fprintf(stderr, "Fce4ValidateHeader: Part out of bounds %d (vertices)\n", hdr->NumParts - 1);
        retv = 0;
      }

      if ((hdr->P1stTriangles[hdr->NumParts - 1] < 0) ||
          (hdr->P1stTriangles[hdr->NumParts - 1] + hdr->PNumTriangles[hdr->NumParts - 1] > hdr->NumTriangles))
      {
        fprintf(stderr, "Fce4ValidateHeader: Part out of bounds %d (triangles)\n", hdr->NumParts - 1);
        retv = 0;
      }
    }

    /*
      Validate filesize, area offsets, area sizes, areas non-overlapping

      Fcecodec requires that area sizes relate to given NumVertices, and
      PNumTriangles

      Note: Fcecodec warns about, accepts (VertTblOffset > 0)
    */
    if ((size = FCELIB_FCETYPES_Fce4ComputeSize(hdr->Version, hdr->NumVertices, hdr->NumTriangles)) != fce_size)
    {
      /* Are just Reserve5, Reserve6 invalid? ex. 99viper/?.fce */

      int area_5_6_size = 4 * hdr->NumVertices + 12 * hdr->NumTriangles;
      if (hdr->Version == 0x00101015)
        area_5_6_size += hdr->NumVertices;

      if (size - area_5_6_size != fce_size - abs(fce_size - 0x2038 - hdr->Reserve5offset))
      {
        fprintf(stderr, "Fce4ValidateHeader: FCE filesize mismatch %d (expects %d) %d\n",
                        fce_size,
                        size, fce_size - size);
        fprintf(stderr, "Fce4ValidateHeader: NumVertices=%d , NumTriangles=%d\n", hdr->NumVertices, hdr->NumTriangles);
        fprintf(stderr, "Fce4ValidateHeader: until 5: %d (expects %d) %d\n",
                        fce_size - abs(fce_size - 0x2038 - hdr->Reserve5offset),
                        size - area_5_6_size,
                        fce_size - abs(fce_size - 0x2038 - hdr->Reserve5offset) - (size - area_5_6_size));
        retv = 0;
      }
      else
      {
        fprintf(stderr, "Fce4ValidateHeader: Warning FCE filesize mismatch (Reserve5offset, Reserve6offset invalid)\n");
      }
    }


    /*
      Warn about Reserve5offset, Reserve6offset mismatches
      Fcecodec allows, if all areas are within bounds
    */
    if ((hdr->Reserve5offset > hdr->Reserve6offset) ||
        (0x2038 + hdr->Reserve6offset > fce_size) ||
        (0x2038 + hdr->Reserve5offset > fce_size))
    {
      fprintf(stderr, "Fce4ValidateHeader: Reserve5offset or Reserve6offset out of bounds\n");
      fprintf(stderr, "Fce4ValidateHeader: Reserve5offset = 0x%04x (0x%x), Size = %d\n", hdr->Reserve5offset, 0x2038 + hdr->Reserve5offset, hdr->Reserve6offset - hdr->Reserve5offset);
      fprintf(stderr, "Fce4ValidateHeader: Reserve6offset = 0x%04x (0x%x), Size = %d\n", hdr->Reserve6offset, 0x2038 + hdr->Reserve6offset, fce_size - 0x2038 - hdr->Reserve6offset);
      retv = 0;
    }

    dist_to_eof = 12 * hdr->NumTriangles;
    if (hdr->Version == 0x00101015)
      dist_to_eof += hdr->NumVertices;
    if ((hdr->Reserve6offset < 0) || (fce_size - 0x2038 - hdr->Reserve6offset) != dist_to_eof)
    {
      fprintf(stderr, "Fce4ValidateHeader: Warning Reserve6offset invalid 0x%04x (expects 0x%04x) %d\n",
                      hdr->Reserve6offset, fce_size - 0x2038 - dist_to_eof,
                      hdr->Reserve6offset - (fce_size - 0x2038 - dist_to_eof));
    }

    dist_to_eof += 4 * hdr->NumVertices;
    if ((hdr->Reserve5offset < 0) || (fce_size - 0x2038 - hdr->Reserve5offset) != dist_to_eof)
    {
      fprintf(stderr, "Fce4ValidateHeader: Warning Reserve5offset invalid 0x%04x (expects 0x%04x)\n",
                      hdr->Reserve5offset, fce_size - 0x2038 - dist_to_eof);
    }

    /* Forget about Reserve5, Reserve6 */
    {
      int area_5_6_size = abs(fce_size - 0x2038 - hdr->Reserve5offset);
      dist_to_eof = 0;

      dist_to_eof += 4 * hdr->NumVertices;
      if ((hdr->AnimationTblOffset < 0) || (fce_size - 0x2038 - hdr->AnimationTblOffset - area_5_6_size) != dist_to_eof)
      {
        fprintf(stderr, "Fce4ValidateHeader: AnimationTblOffset invalid 0x%04x (expects 0x%04x)\n",
                        hdr->AnimationTblOffset, fce_size - 0x2038 - dist_to_eof - area_5_6_size);
        retv = 0;
      }
      dist_to_eof += 4 * hdr->NumVertices;
      if ((hdr->Reserve4offset < 0) || (fce_size - 0x2038 - hdr->Reserve4offset - area_5_6_size) != dist_to_eof)
      {
        fprintf(stderr, "Fce4ValidateHeader: Reserve4offset invalid 0x%04x (expects 0x%04x)\n",
                        hdr->Reserve4offset, fce_size - 0x2038 - dist_to_eof - area_5_6_size);
        retv = 0;
      }

      dist_to_eof += 12 * hdr->NumVertices;
      if ((hdr->DamgdNormTblOffset < 0) || (fce_size - 0x2038 - hdr->DamgdNormTblOffset - area_5_6_size) != dist_to_eof)
      {
        fprintf(stderr, "Fce4ValidateHeader: DamgdNormTblOffset invalid 0x%04x (expects 0x%04x)\n",
                        hdr->DamgdNormTblOffset, fce_size - 0x2038 - dist_to_eof - area_5_6_size);
        retv = 0;
      }
      dist_to_eof += 12 * hdr->NumVertices;
      if ((hdr->DamgdVertTblOffset < 0) || (fce_size - 0x2038 - hdr->DamgdVertTblOffset - area_5_6_size) != dist_to_eof)
      {
        fprintf(stderr, "Fce4ValidateHeader: DamgdVertTblOffset invalid 0x%04x (expects 0x%04x)\n",
                        hdr->DamgdVertTblOffset, fce_size - 0x2038 - dist_to_eof - area_5_6_size);
        retv = 0;
      }
      dist_to_eof += 12 * hdr->NumVertices;
      if ((hdr->UndamgdNormTblOffset < 0) || (fce_size - 0x2038 - hdr->UndamgdNormTblOffset - area_5_6_size) != dist_to_eof)
      {
        fprintf(stderr, "Fce4ValidateHeader: UndamgdNormTblOffset invalid 0x%04x (expects 0x%04x)\n",
                        hdr->UndamgdNormTblOffset, fce_size - 0x2038 - dist_to_eof - area_5_6_size);
        retv = 0;
      }
      dist_to_eof += 12 * hdr->NumVertices;
      if ((hdr->UndamgdVertTblOffset < 0) || (fce_size - 0x2038 - hdr->UndamgdVertTblOffset - area_5_6_size) != dist_to_eof)
      {
        fprintf(stderr, "Fce4ValidateHeader: UndamgdVertTblOffset invalid 0x%04x (expects 0x%04x)\n",
                        hdr->UndamgdVertTblOffset, fce_size - 0x2038 - dist_to_eof - area_5_6_size);
        retv = 0;
      }

      dist_to_eof += 12 * hdr->NumVertices;
      if ((hdr->Reserve3offset < 0) || (fce_size - 0x2038 - hdr->Reserve3offset - area_5_6_size) != dist_to_eof)
      {
        fprintf(stderr, "Fce4ValidateHeader: Reserve3offset invalid 0x%04x (expects 0x%04x)\n",
                        hdr->Reserve3offset, fce_size - 0x2038 - dist_to_eof - area_5_6_size);
        retv = 0;
      }
      dist_to_eof += 12 * hdr->NumVertices;
      if ((hdr->Reserve2offset < 0) || (fce_size - 0x2038 - hdr->Reserve2offset - area_5_6_size) != dist_to_eof)
      {
        fprintf(stderr, "Fce4ValidateHeader: Reserve2offset invalid 0x%04x (expects 0x%04x)\n",
                        hdr->Reserve2offset, fce_size - 0x2038 - dist_to_eof - area_5_6_size);
        retv = 0;
      }
      dist_to_eof += 32 * hdr->NumVertices;
      if ((hdr->Reserve1offset < 0) || (fce_size - 0x2038 - hdr->Reserve1offset - area_5_6_size) != dist_to_eof)
      {
        fprintf(stderr, "Fce4ValidateHeader: Reserve1offset invalid 0x%04x (expects 0x%04x)\n",
                        hdr->Reserve1offset, fce_size - 0x2038 - dist_to_eof - area_5_6_size);
        retv = 0;
      }

      dist_to_eof += 56 * hdr->NumTriangles;
      if ((hdr->TriaTblOffset < 0) || (fce_size - 0x2038 - hdr->TriaTblOffset - area_5_6_size) != dist_to_eof)
      {
        fprintf(stderr, "Fce4ValidateHeader: TriaTblOffset invalid 0x%04x (expects 0x%04x)\n",
                        hdr->TriaTblOffset, fce_size - 0x2038 - dist_to_eof - area_5_6_size);
        retv = 0;
      }
      dist_to_eof += 12 * hdr->NumVertices;
      if ((hdr->NormTblOffset < 0) || (fce_size - 0x2038 - hdr->NormTblOffset - area_5_6_size) != dist_to_eof)
      {
        fprintf(stderr, "Fce4ValidateHeader: NormTblOffset invalid 0x%04x (expects 0x%04x)\n",
                        hdr->NormTblOffset, fce_size - 0x2038 - dist_to_eof - area_5_6_size);
        retv = 0;
      }
      dist_to_eof += 12 * hdr->NumVertices;
      if ((hdr->VertTblOffset < 0) || (fce_size - 0x2038 - hdr->VertTblOffset - area_5_6_size) != dist_to_eof)
      {
        fprintf(stderr, "Fce4ValidateHeader: VertTblOffset invalid 0x%04x (expects 0x%04x)\n",
                        hdr->VertTblOffset, fce_size - 0x2038 - dist_to_eof - area_5_6_size);
        retv = 0;
      }
    }  /* end: Forget about Reserve5, Reserve6 */

    /* warnings */
    if (retv)
    {
      if (hdr->NumVertices != count_verts)
      {
        fprintf(stderr, "Fce4ValidateHeader: Warning Expects %d vertices in total, found %d\n",
                        hdr->NumVertices, count_verts);
      }
      if (hdr->NumTriangles != count_triags)
      {
        fprintf(stderr, "Fce4ValidateHeader: Warning Expects %d triangles in total, found %d\n",
                        hdr->NumTriangles, count_triags);
      }
    }

    if (hdr->NumArts != 1)
      fprintf(stderr, "Fce4ValidateHeader: Warning NumArts != 1 (%d)\n", hdr->NumArts);

    if (hdr->VertTblOffset)
    {
      fprintf(stderr, "Fce4ValidateHeader: Warning VertTblOffset = 0x%04x (expects 0x0000)\n",
                      hdr->VertTblOffset);
    }

    if ((hdr->HalfSize[0] < 0.001) || (hdr->HalfSize[2] < 0.001) ||
        (hdr->HalfSize[0] * hdr->HalfSize[2] < 0.1) ||
        (hdr->HalfSize[1] < 0.0))
    {
      fprintf(stderr, "Fce4ValidateHeader: Warning HalfSizes may crash game\n");
    }

    if (hdr->Version == 0x00101015)
    {
      int count_wheels;
      float wheelbase = FCELIB_FCETYPES_GetWheelbase4M(hdr, &count_wheels);
      if (wheelbase < 2.45 || wheelbase > 3.44)
        fprintf(stderr, "Fce4ValidateHeader: Warning Wheelbase may crash game (%f; %d)\n", wheelbase, count_wheels);
    }
    break;
  }

  return retv;
}

/* print info --------------------------------------------------------------- */

void FCELIB_FCETYPES_PrintHeaderFce3(const void * const buf, const int fce_size)
{
  int i;
  FceHeader3 hdr;
  int verts = 0;
  int triags = 0;

  FCELIB_FCETYPES_GetFceHeader3(&hdr, (const unsigned char *)buf);

  printf("Filesize = %d (0x%x)\n", fce_size, fce_size);
  printf("Version = FCE3\n");

  if (FCELIB_FCETYPES_MiniValidateHdr3((const unsigned char *)buf))
  {
    printf("NumTriangles = %d (* 56 = %d)\n", hdr.NumTriangles, 56 * hdr.NumTriangles);
    printf("NumVertices = %d (* 12 = %d)  (* 32 = %d)\n", hdr.NumVertices, 12 * hdr.NumVertices, 32 * hdr.NumVertices);
    printf("NumArts = %d\n", hdr.NumArts);

    printf("XHalfSize = %f\n", hdr.HalfSize[0]);
    printf("YHalfSize = %f\n", hdr.HalfSize[1]);
    printf("ZHalfSize = %f\n", hdr.HalfSize[2]);

    printf("NumParts = %d\n", hdr.NumParts);
    printf("NumDummies = %d\n", hdr.NumDummies);
    printf("NumPriColors = %d\n", hdr.NumPriColors);
    printf("NumSecColors = %d\n", hdr.NumSecColors);

    printf("VertTblOffset = 0x%04x (0x%x), Size = %d\n", hdr.VertTblOffset, 0x1F04 + hdr.VertTblOffset, hdr.NormTblOffset - hdr.VertTblOffset);
    printf("NormTblOffset = 0x%04x (0x%x), Size = %d\n", hdr.NormTblOffset, 0x1F04 + hdr.NormTblOffset, hdr.TriaTblOffset - hdr.NormTblOffset);
    printf("TriaTblOffset = 0x%04x (0x%x), Size = %d\n", hdr.TriaTblOffset, 0x1F04 + hdr.TriaTblOffset, hdr.Reserve1offset - hdr.TriaTblOffset);

    printf("Reserve1offset = 0x%04x (0x%x), Size = %d\n", hdr.Reserve1offset, 0x1F04 + hdr.Reserve1offset, hdr.Reserve2offset - hdr.Reserve1offset);
    printf("Reserve2offset = 0x%04x (0x%x), Size = %d\n", hdr.Reserve2offset, 0x1F04 + hdr.Reserve2offset, hdr.Reserve3offset - hdr.Reserve2offset);
    printf("Reserve3offset = 0x%04x (0x%x), Size = %d\n", hdr.Reserve3offset, 0x1F04 + hdr.Reserve3offset, fce_size - 0x1F04 - hdr.Reserve3offset);

    printf("Unknown1 (0x0004) = %d (0x%04x)\n", hdr.Unknown1, hdr.Unknown1);

    printf("Parts:\n"
           "Idx  Verts       Triags      (PartPos)                         Description          Name\n");
    for (i = 0; i < SCL_min(FCELIB_UTIL_Fce3PartsImplemented, hdr.NumParts); i++)
    {
      printf(" %2d  %5d %5d %5d %5d (%9f, %9f, %9f) %20s %s\n",
             i,
             hdr.P1stVertices[i],
             hdr.PNumVertices[i],
             hdr.P1stTriangles[i],
             hdr.PNumTriangles[i],
             hdr.PartPos[i * 3 + 0], hdr.PartPos[i * 3 + 1], hdr.PartPos[i * 3 + 2],
             kFce3PartsNames[i],
             hdr.PartNames + (i * 64));

      verts += hdr.PNumVertices[i];
      triags += hdr.PNumTriangles[i];
    }
    for (i = SCL_min(FCELIB_UTIL_Fce3PartsImplemented, hdr.NumParts); i < SCL_min(64, hdr.NumParts); i++)
    {
      printf(" %2d  %5d %5d %5d %5d (%9f, %9f, %9f) %20s %s\n",
             i,
             hdr.P1stVertices[i],
             hdr.PNumVertices[i],
             hdr.P1stTriangles[i],
             hdr.PNumTriangles[i],
             hdr.PartPos[i * 3 + 0], hdr.PartPos[i * 3 + 1], hdr.PartPos[i * 3 + 2],
             "",
             hdr.PartNames + (i * 64));

      verts += hdr.PNumVertices[i];
      triags += hdr.PNumTriangles[i];
    }
    printf("         = %5d     = %5d\n",
           verts, triags);

    printf("Filesize (verts, triags) = %d (0x%x), diff=%d\n",
           FCELIB_FCETYPES_Fce3ComputeSize(verts, triags),
           FCELIB_FCETYPES_Fce3ComputeSize(verts, triags),
           fce_size - FCELIB_FCETYPES_Fce3ComputeSize(verts, triags));

    printf("DummyNames (Position):\n");
    for (i = 0; i < SCL_min(hdr.NumDummies, 16); i++)
    {
      printf(" (%9f, %9f, %9f) %s\n",
             hdr.Dummies[i * 3 + 0], hdr.Dummies[i * 3 + 1], hdr.Dummies[i * 3 + 2],
             hdr.DummyNames + (i * 64));
    }

    printf("Car colors (hue, saturation, brightness, transparency):\n");
    for (i = 0; i < SCL_min(hdr.NumPriColors, 16); i++)
    {
      printf("%2d  Primary     %3d, %3d, %3d, %3d\n", i,
             hdr.PriColors[i].hue, hdr.PriColors[i].saturation,
             hdr.PriColors[i].brightness, hdr.PriColors[i].transparency);
      printf("%2d  Secondary   %3d, %3d, %3d, %3d\n", i,
             hdr.SecColors[i].hue, hdr.SecColors[i].saturation,
             hdr.SecColors[i].brightness, hdr.SecColors[i].transparency);
    }
  }
}

void FCELIB_FCETYPES_PrintHeaderFce4(const void * const buf, const int fce_size)
{
  int i;
  FceHeader4 hdr;
  int verts = 0;
  int triags = 0;

  FCELIB_FCETYPES_GetFceHeader4(&hdr, (const unsigned char *)buf);

  printf("Filesize = %d (0x%x)\n", fce_size, fce_size);
  if (hdr.Version == 0x00101014)
    printf("Version = FCE4\n");
  else  /* 0x00101015 */
    printf("Version = FCE4M\n");

  if (FCELIB_FCETYPES_MiniValidateHdr4((const unsigned char *)buf))
  {
    printf("NumTriangles = %d (* 12 = %d) (* 56 = %d)\n", hdr.NumTriangles, 12 * hdr.NumTriangles, 56 * hdr.NumTriangles);
    printf("NumVertices = %d (* 4 = %d)  (* 12 = %d)  (* 32 = %d)\n", hdr.NumVertices, 4 * hdr.NumVertices, 12 * hdr.NumVertices, 32 * hdr.NumVertices);
    printf("NumArts = %d\n", hdr.NumArts);

    printf("XHalfSize = %f\n", hdr.HalfSize[0]);
    printf("YHalfSize = %f\n", hdr.HalfSize[1]);
    printf("ZHalfSize = %f\n", hdr.HalfSize[2]);
    if (hdr.Version == 0x00101015)
    {
      int count_wheels;
      float wheelbase = FCELIB_FCETYPES_GetWheelbase4M(&hdr, &count_wheels);
      if (count_wheels < 1)
        printf("Wheelbase = %f (%d wheels)\n", wheelbase, count_wheels);
      if (count_wheels == 1)
        printf("Wheelbase = %f (%d wheel)\n", wheelbase, count_wheels);
      else
        printf("Wheelbase = %f (%d wheels)\n", wheelbase, count_wheels);
    }

    printf("NumParts = %d\n", hdr.NumParts);
    printf("NumDummies = %d\n", hdr.NumDummies);
    printf("NumColors = %d\n", hdr.NumColors);

    printf("VertTblOffset = 0x%04x (0x%x), Size = %d\n", hdr.VertTblOffset, 0x2038 + hdr.VertTblOffset, hdr.NormTblOffset - hdr.VertTblOffset);
    printf("NormTblOffset = 0x%04x (0x%x), Size = %d\n", hdr.NormTblOffset, 0x2038 + hdr.NormTblOffset, hdr.TriaTblOffset - hdr.NormTblOffset);
    printf("TriaTblOffset = 0x%04x (0x%x), Size = %d\n", hdr.TriaTblOffset, 0x2038 + hdr.TriaTblOffset, hdr.Reserve1offset - hdr.TriaTblOffset);

    printf("Reserve1offset = 0x%04x (0x%x), Size = %d\n", hdr.Reserve1offset, 0x2038 + hdr.Reserve1offset, hdr.Reserve2offset - hdr.Reserve1offset);
    printf("Reserve2offset = 0x%04x (0x%x), Size = %d\n", hdr.Reserve2offset, 0x2038 + hdr.Reserve2offset, hdr.Reserve3offset - hdr.Reserve2offset);
    printf("Reserve3offset = 0x%04x (0x%x), Size = %d\n", hdr.Reserve3offset, 0x2038 + hdr.Reserve3offset, hdr.UndamgdVertTblOffset - hdr.Reserve3offset);

    printf("UndamgdVertTblOffset = 0x%04x (0x%x), Size = %d\n", hdr.UndamgdVertTblOffset, 0x2038 + hdr.UndamgdVertTblOffset, hdr.UndamgdNormTblOffset - hdr.UndamgdVertTblOffset);
    printf("UndamgdNormTblOffset = 0x%04x (0x%x), Size = %d\n", hdr.UndamgdNormTblOffset, 0x2038 + hdr.UndamgdNormTblOffset, hdr.DamgdVertTblOffset - hdr.UndamgdNormTblOffset);
    printf("DamgdVertTblOffset = 0x%04x (0x%x), Size = %d\n", hdr.DamgdVertTblOffset, 0x2038 + hdr.DamgdVertTblOffset, hdr.DamgdNormTblOffset - hdr.DamgdVertTblOffset);
    printf("DamgdNormTblOffset = 0x%04x (0x%x), Size = %d\n", hdr.DamgdNormTblOffset, 0x2038 + hdr.DamgdNormTblOffset, hdr.Reserve4offset - hdr.DamgdNormTblOffset);

    printf("Reserve4offset = 0x%04x (0x%x), Size = %d\n", hdr.Reserve4offset, 0x2038 + hdr.Reserve4offset, hdr.AnimationTblOffset - hdr.Reserve4offset);
    printf("AnimationTblOffset = 0x%04x (0x%x), Size = %d\n", hdr.AnimationTblOffset, 0x2038 + hdr.AnimationTblOffset, hdr.Reserve5offset - hdr.AnimationTblOffset);
    printf("Reserve5offset = 0x%04x (0x%x), Size = %d\n", hdr.Reserve5offset, 0x2038 + hdr.Reserve5offset, hdr.Reserve6offset - hdr.Reserve5offset);

    printf("Reserve6offset = 0x%04x (0x%x), Size = %d\n", hdr.Reserve6offset, 0x2038 + hdr.Reserve6offset, fce_size - 0x2038 - hdr.Reserve6offset);

    printf("Unknown1 (0x0004) = %d (0x%04x)\n", hdr.Unknown1, hdr.Unknown1);
    printf("Unknown3 (0x0924) = %d (0x%04x)\n", hdr.Unknown3, hdr.Unknown3);

    printf("Parts:\n"
                    "Idx  Verts       Triangles   (PartPos)                         Name\n");
    for (i = 0; i < SCL_min(hdr.NumParts, 64); i++)
    {
      printf(" %2d  %5d %5d %5d %5d (%9f, %9f, %9f) %s\n",
             i,
             hdr.P1stVertices[i],
             hdr.PNumVertices[i],
             hdr.P1stTriangles[i],
             hdr.PNumTriangles[i],
             hdr.PartPos[i * 3 + 0], hdr.PartPos[i * 3 + 1], hdr.PartPos[i * 3 + 2],
             hdr.PartNames + (i * 64));

      verts += hdr.PNumVertices[i];
      triags += hdr.PNumTriangles[i];
    }
    printf("         = %5d     = %5d\n",
           verts, triags);

    printf("FCE4 Filesize (verts, triags) = %d (0x%x), diff=%d\n",
           FCELIB_FCETYPES_Fce4ComputeSize(0x00101014, verts, triags),
           FCELIB_FCETYPES_Fce4ComputeSize(0x00101014, verts, triags),
           fce_size - FCELIB_FCETYPES_Fce4ComputeSize(0x00101014, verts, triags));
    printf("FCE4M Filesize (verts, triags) = %d (0x%x), diff=%d\n",
           FCELIB_FCETYPES_Fce4ComputeSize(0x00101015, verts, triags),
           FCELIB_FCETYPES_Fce4ComputeSize(0x00101015, verts, triags),
           fce_size - FCELIB_FCETYPES_Fce4ComputeSize(0x00101015, verts, triags));

    printf("DummyNames (Position):\n");
    for (i = 0; i < SCL_min(hdr.NumDummies, 16); i++)
    {
      printf(" (%9f, %9f, %9f) %s\n",
             hdr.Dummies[i * 3 + 0], hdr.Dummies[i * 3 + 1], hdr.Dummies[i * 3 + 2],
             hdr.DummyNames + (i * 64));
    }

    printf("Car colors (hue, saturation, brightness, transparency):\n");
    for (i = 0; i < SCL_min(hdr.NumColors, 16); i++)
    {
      printf("%2d  Primary     %3d, %3d, %3d, %3d\n", i,
            hdr.PriColors[i].hue, hdr.PriColors[i].saturation,
            hdr.PriColors[i].brightness, hdr.PriColors[i].transparency);
      printf("%2d  Interior    %3d, %3d, %3d, %3d\n", i,
            hdr.IntColors[i].hue, hdr.IntColors[i].saturation,
            hdr.IntColors[i].brightness, hdr.IntColors[i].transparency);
      printf("%2d  Secondary   %3d, %3d, %3d, %3d\n", i,
            hdr.SecColors[i].hue, hdr.SecColors[i].saturation,
            hdr.SecColors[i].brightness, hdr.SecColors[i].transparency);
      printf("%2d  Driver hair %3d, %3d, %3d, %3d\n", i,
            hdr.DriColors[i].hue, hdr.DriColors[i].saturation,
            hdr.DriColors[i].brightness, hdr.DriColors[i].transparency);
    }
  }
}

/* Version ------------------------------------------------------------------ */

/* Returns 3 (FCE3), 4 (FCE4), 5 (FCE4M); negative (invalid); 0 input is NULL

  Note: For return values 4|5, true version can still be FCE3. See FCELIB_FCETYPES_ValidateFceVersion()
*/
int FCELIB_FCETYPES_GetFceVersion(const void * const buf, const int bufsz)
{
  if (buf && bufsz > 0)
  {
    int version;
    if (bufsz < 0x1F04)  return -3;
    memcpy(&version, buf, 4);
    if ((version != 0x00101014 && version != 0x00101015))  return 3;
    else if (version == 0x00101014)  return bufsz >= 0x2038 ? 4 : -4;  /* can still be FCE3 */
    else if (version == 0x00101015)  return bufsz >= 0x2038 ? 5 : -5;  /* can still be FCE3 */
  }
  return 0;
}

#endif  /* FCELIB_FCETYPES_H_ */
