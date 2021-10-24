/*
  fcelib_fcetypes.h
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
  implements FCE types, format validations. documents FCE format.
 **/


#ifndef FCELIB_FCETYPES_H
#define FCELIB_FCETYPES_H

#include <stdio.h>
#include <string.h>

#include "fcelib_misc.h"

#ifdef __cplusplus
namespace fcelib {
#endif

#ifdef __cplusplus
extern "C" {
#endif


/* FCE3   tTriangle->flag   4 bit
      0x0   default           body parts: reflection
0:    0x1   no reflection
1:    0x2   high chrome
2:    0x4   no cull           two-faced triangle
3:    0x8   semi-transparent  all parts

5 = 1 + 4
6 = 2 + 4
A = 2 + 8
E = 2 + 4 + 8

fcelib supports the following OBJ material names:
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

A triangle is visible behind a semi-transparent triangle, if
its index is smaller.
*/

/* FCE4   tTriangle->flag   12 bit
      0x000   default           body parts: reflection,
                                interior etc.: no reflection
0:    0x001   no reflection
1:    0x002   high chrome       body parts: used for windows etc.
2:    0x004   no cull           two-faced triangle
3:    0x008   semi-transparent  body parts: used for windows etc.

4:    0x010   ?                 elni/car.fce :OH :OD :OLM :ORM :H**W :M**W, partial :HB :MB :LB :TB
5:    0x020   all windows
6:    0x040   front window
7:    0x080   left window

8:    0x100   back window
9:    0x200   right window
10:   0x400   broken window
11:   0x800   ?

car.fce   body               default
car.fce   underbody          no reflection
car.fce   body roof          no cull
car.fce   windows            all windows + high chrome + no cull + semi-transparent
dash.fce  mirror glass       high chrome + semi-transparent

6 = 2 + 4
A = 2 + 8
E = 2 + 4 + 8

fcelib supports the following OBJ material names:
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

/* FCE4M   tTriangle->flag   32 bit

todo
*/


/* Length = 56. Vertex indices are local. Values from 'P1stVertices' make them global. */
typedef struct {
/* 0x00 */  int   tex_page;     /* Texture page number. FCE3, FCE4: == 0,
                                                        FCE4M:      >= 0 */
/* 0x04 */  int   vidx1;        /* Vertex #1 local index */
/* 0x08 */  int   vidx2;
/* 0x0C */  int   vidx3;
/* 0x10 */  char  unknown[12];  /* all items = 0xFF00 */
/* 0x1C */  int   flag;         /* triangle flag */
/* 0x20 */  float U1;           /* Vertex #1 texture U-coordinate */
/* 0x24 */  float U2;
/* 0x28 */  float U3;
/* 0x2C */  float V1;           /* Vertex #1 texture V-coordinate */
/* 0x30 */  float V2;
/* 0x34 */  float V3;
} tTriangle;

typedef struct {
  float x;  /* x->inf is to the right */
  float y;  /* y->inf is up */
  float z;  /* z->inf is to the front */
} tVector;

/* Valid values for all four components: 0..255
  hue<degrees>  / 360 * 255
  saturation<%> / 100 * 255
  brightness<%> / 100 * 255               */
typedef struct {
  int hue;
  int saturation;
  int brightness;
  int transparency;
} tColor3;

typedef struct {
  unsigned char hue;
  unsigned char saturation;
  unsigned char brightness;
  unsigned char transparency;
} tColor4;

/* 0x1F04  -  size of this header */
typedef struct {
/* 0x0000 */  int      Unknown1           ;  /* != 0x14101000 && != 0x15101000, nullable, sometimes 0x13101000 (ex. render/pc/cone.fce) */
/* 0x0004 */  int      NumTriangles       ;  /* Number of triangles in model */
/* 0x0008 */  int      NumVertices        ;  /* Number of vertices in model */
/* 0x000C */  int      NumArts            ;  /* Number of arts */
                    /* offsets from 0x1F04 */
/* 0x0010 */  int      VertTblOffset      ;  /* usually 0x00. len() = 12 * NumVertices */
/* 0x0014 */  int      NormTblOffset      ;  /* len() = len(VertTbl) */
/* 0x0018 */  int      TriaTblOffset      ;  /* len() = 56 * NumTriangles */

/* 0x001C */  int      Reserve1offset     ;  /* len() = 32 * NumVertices */
/* 0x0020 */  int      Reserve2offset     ;  /* len() = len(VertTbl) */
/* 0x0024 */  int      Reserve3offset     ;  /* len() = len(VertTbl) */

/* 0x0028 */  float    XHalfSize          ;  /* X half-size width of whole model */
/* 0x002C */  float    YHalfSize          ;  /* Y half-size heigth of whole model */
/* 0x0030 */  float    ZHalfSize          ;  /* Z half-size length of whole model */

/* 0x0034 */  int      NumDummies         ;  /* Number of light sources 0..16 */
/* 0x0038 */  tVector  Dummies[16]        ;  /* Coordinates of dummies */

/* 0x00F8 */  int      NumParts           ;  /* Number of car parts 0..64 */
/* 0x00FC */  tVector  PartPos[64]        ;  /* Global position of car parts */

/* 0x03FC */  int      P1stVertices [64]  ;  /* First vertex index for each part */
/* 0x04FC */  int      PNumVertices [64]  ;  /* Number of vertices for each part */

/* 0x05FC */  int      P1stTriangles[64]  ;  /* First triangle index for each part */
/* 0x06FC */  int      PNumTriangles[64]  ;  /* Number of triangles for each part */

/* 0x07FC */  int      NumPriColors       ;  /* Number of primary colors 0..16 */
/* 0x0800 */  tColor3  PriColors[16]      ;  /* Primary colors */
/* 0x0900 */  int      NumSecColors       ;  /* Number of secondary colors 0..16 */
/* 0x0904 */  tColor3  SecColors[16]      ;  /* Secondary colors */

/* 0x0A04 */  char     DummyNames[64 * 16];  /* Dummy object names (ASCIIZ, role only by name) */

/* 0x0E04 */  char     PartNames [64 * 64];  /* Part names (ASCIIZ, role only by order) */

/* 0x1E04 */  char     Unknown2[256]      ;  /* nullable */
} FceHeader3;

static
const char *kFce3PartsNames[kFceLibImplementedFce3Parts] = {
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

cone.fce (FCE3) - traffic cone
Name/Description         Idx  UsesFlag  Light  Reflection
ConeH                    0
ConeM                    1
ConeL                    2

go0#.fce (FCE3) - track menu model (# = 0-8)
Name/Description         Idx  UsesFlag  Light  Reflection
Loft01                   0
Loft02                   1    Y

*/

/* 0x2038  -  size of this header */
typedef struct {
/* 0x0000 */  int      Version             ;  /* FCE4: 0x00101014, FCE4M: 0x00101015 */
/* 0x0004 */  int      Unknown1            ;  /* nullable */
/* 0x0008 */  int      NumTriangles        ;  /* Number of triangles in model */
/* 0x000c */  int      NumVertices         ;  /* Number of vertices in model */
/* 0x0010 */  int      NumArts             ;  /* Number of arts */
                    /* offsets from 0x2038 */
/* 0x0014 */  int      VertTblOffset       ;  /* usually 0x0000. len() = 12 * NumVertices */
/* 0x0018 */  int      NormTblOffset       ;  /* len() = len(VertTbl) */
/* 0x001c */  int      TriaTblOffset       ;  /* len() = 56 * NumTriangles */

/* 0x0020 */  int      Reserve1offset      ;  /* len() = 32 * NumVertices */
/* 0x0024 */  int      Reserve2offset      ;  /* len() = len(VertTbl) */
/* 0x0028 */  int      Reserve3offset      ;  /* len() = len(VertTbl) */

/* 0x002c */  int      UndamgdVertTblOffset;  /* UndamgdVertTbl should be copy of VertTbl, unused in FCE4 */
/* 0x0030 */  int      UndamgdNormTblOffset;  /* UndamgdNormTbl should be copy of NormTbl, unused in FCE4 */
/* 0x0034 */  int      DamgdVertTblOffset  ;  /* len() = len(VertTbl) */
/* 0x0038 */  int      DamgdNormTblOffset  ;  /* len() = len(VertTbl) */

/* 0x003c */  int      Reserve4offset      ;  /* len() = 4 * NumVertices, null */
/* 0x0040 */  int      AnimationTblOffset  ;  /* len() = 4 * NumVertices, flags (0x4 = immovable, 0x0 othw) */
/* 0x0044 */  int      Reserve5offset      ;  /* len() = 4 * NumVertices, null */

/* 0x0048 */  int      Reserve6offset      ;  /* len() = 12 * NumTriangles, null
                                                FCE4M: len() += NumVertices */
/* 0x004c */  float    XHalfSize           ;  /* X half-size width of whole model */
/* 0x0050 */  float    YHalfSize           ;  /* Y half-size heigth of whole model */
/* 0x0054 */  float    ZHalfSize           ;  /* Z half-size length of whole model */

/* 0x0058 */  int      NumDummies          ;  /* Number of light sources */
/* 0x005c */  tVector  Dummies[16]         ;  /* Coordinates of dummies */

/* 0x011c */  int      NumParts            ;  /* Number of car parts */
/* 0x0120 */  tVector  PartPos[64]         ;  /* Global position of car parts */

/* 0x0420 */  int      P1stVertices[64]    ;  /* First vertex index for each part */
/* 0x0520 */  int      PNumVertices[64]    ;  /* Number of vertices for each part */

/* 0x0620 */  int      P1stTriangles[64]   ;  /* First triangle index for each part */
/* 0x0720 */  int      PNumTriangles[64]   ;  /* Number of triangles for each part */

/* 0x0820 */  int      NumColors           ;  /* FCE4: Number of colors 0..16
                                                FCE4M: unknown */
/* 0x0824 */  tColor4  PriColors[16]       ;  /* Primary colors */
/* 0x0864 */  tColor4  IntColors[16]       ;  /* Interior colors */
/* 0x08a4 */  tColor4  SecColors[16]       ;  /* Secondary colors */
/* 0x08e4 */  tColor4  DriColors[16]       ;  /* Driver hair colors */

/* 0x0924 */  int      Unknown3            ;  /* FCE4: nullable, FCE4M: ? */
/* 0x0928 */  char     Unknown2[256]       ;  /* nullable */

/* 0x0a28 */  char     DummyNames[64 * 16] ;  /* Dummy object names (ASCIIZ, role only by name) */

/* 0x0e28 */  char     PartNames [64 * 64] ;  /* Part names (ASCIIZ, role only by name) */

/* 0x1e28 */  char     Unknown4[528]       ;  /* nullable */
} FceHeader4;

/*
car.fce (FCE4)
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
:OC     interior                 N       N       N         N      N          N
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

hel.fce (FCE4) - helicopter
Name    Description               UsesFlag  Animated
'body'  body                      Y         N
'main'  rotor                     Y         Y
'tail'  tail rotor                Y         Y
:LB     low body                  Y
:Lmain  low rotor                 Y
:Ltail  low tail rotor            Y

part.fce (FCE4M) - car
Name              Description                   Damage  FallOf  UsesFlag  Light  Animated   Pursuit
:Hbody            high body                     Y       N       Y         N      N          N
:Hconvertible     top
:Hdashlight       dashboard lights
:Hfenderlight
:Hfirewall
:Hheadlight       hidden lights
:Hhood            engine hood
:Hhoodhole        engine hood w/ hole
:Hinterior        interior
:Hlbrake          left brake front
:Hrbrake          right brake front
:Hlmirror         left mirror
:Hrmirror         right mirror
:Hscoopsmall      hood scoop small
:Hscooplarge      hood scoop large
:Hskirt
:Hskirtwell
:Hsteer           steering wheel
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

  FCE4M places meshes for wheels, drivers, and enhanced parts in central files.
  They are positioned via the following dummy-parts:
Name              Description                                         FoundIn         LinksTo
:PPdriver                                                             *.viv/part.fce  DRIVER##.viv/part.fce
:PPLFwheel        left front wheel                                    *.viv/part.fce
:PPRFwheel        right front wheel                                   *.viv/part.fce
:PPLRwheel        left rear wheel                                     *.viv/part.fce
:PPRRwheel        right rear wheel                                    *.viv/part.fce
:PPaircleaner                                                         *.viv/part.fce
:PPcarb                                                               blowlarg.viv/part.fce
:PPAdash          on top of console left-hand                         *.viv/part.fce
:PPBdash          on top of console center                            *.viv/part.fce
:PPCdash          on hood in front of driver in front of windshield   *.viv/part.fce
:PPengine                                                             *.viv/part.fce
:PPLfog           left fog headlight                                  *.viv/part.fce
:PPRfog           right fog headlight                                 *.viv/part.fce
:PPfrontsusp      front suspension                                    *.viv/part.fce
:PPfuzzydice      fuzzy dice for rearview mirror                      *.viv/part.fce
:PPfuzzydicechop  rear mirror fuzzy dice (chopped roof)               *.viv/part.fce
:PPhoodorn        hood ornament                                       *.viv/part.fce
:PPhoodpivot      hood scoop                                          *.viv/part.fce
:PPlicense        rear license plate                                  *.viv/part.fce
:PPrearsusp       rear suspension                                     *.viv/part.fce
:PPLpipetip       left rear exhaust pipe                              *.viv/part.fce
:PPRpipetip       right rear exhaust pipe                             *.viv/part.fce
:PPLsidepipe      left side exhaust pipe                              *.viv/part.fce
:PPRsidepipe      right side exhaust pipe                             *.viv/part.fce
:PPspot           spotlight                                           *.viv/part.fce
:PPspoiler        rear spoiler                                        *.viv/part.fce  *.viv/spoiler.fce
:PPwheelie        wheelie bar                                         *.viv/part.fce

DRIVER##.viv/part.fce (FCE4M) - driver (## = 1-33)
:PHdriverhead
:PHdriver

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

names are collections of :tags - not necessarily unique
dash.fce                                        FallOf  UsesFlag  Light  Animated
:F L_DM               left driver mirror        Y       Y
:F R_DM               right driver mirror       Y       Y
:L_PM                 left passenger mirror     Y       Y
:R_PM                 right passenger mirror    Y       Y
:F                    visible in front view
:L                    visible in left  view
:R                    visible in right view
:B                    visible in back  view
:B_TOP                roof
:S                    (used on roof)
:F_W                  steering wheel                                     Y
:F_RPM (0.0 to 0.55)  dial                                        N      Y
:F_MPH (0.0 to 0.55)  dial                                        N      Y

FCE4 only:
:R_DASH               unlit dash                                  N
:R_LDASH              lit dash                                    Y

FCE4M only:
:S_DM                 side driver mirror
:S_PM                 side passenger mirror
:L_W                  steering wheel
:S_W                  steering wheel
:F_DASH               unlit dash                                  N
:L_DASH               unlit dash                                  N
:F_LDASH              lit dash                                    Y
:L_LDASH              lit dash                                    Y
:R_MPH (0.0 to 0.55)  dial                                        N      Y
:F_SHIFT              shifter

*/

/* car.fce - light objects (FCE3)
KDSF, KDSFU - Components: char kind, direction, side, flashing, unknown;
Valid values:
  K : "H" (Headlights); "T" (Taillights); "S" (Siren);
  D : "F" (Front/White); "R" (Rear/Read); "M" (Mounted);
  S : "L" (Left); "R" (Right)
  F : "O" (Flashing at moment 1); "E" (Flashing at moment 2); "N" (No flashing)
  U : "N"  ex. corv/car.viv->car.fce, has "TRLN" and "TRLNN"
Colors:
HR__ : headlights, visible from rear, red
HF__ : headlights, visible from front, white
TR__ : taillights, visible from rear, red
TF__ : taillights, visible from front, white
S_L_ : siren, red
S_R_ : siren, blue
Taillights and Sirens never flash. Dummies may appear differently between the
"Player Car" menu, and in-game. Unknown dummies are ignored.

car.fce, hel.fce - light objects (FCE4) (FCE4M) (source: OpenNFS/NFS4Loader.h)
KCBFI, KCBFITD - Components: char kind, color/direction, breakable, flashing, intensity, time, delay;
Valid values:
  K : "H" (Headlights); "T" (Taillights); "B" (Brakelight); "R" (Reverse light); "P" (Parking lights); "S" (Siren);
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
:W_AXIS    1   far-end of steering column (sets angle of rotation)

*/

/* Get header --------------------------------------------------------------- */

/* Assumes valid FCE data */
FceHeader3 FCELIB_FCETYPES_GetFceHeader3(const unsigned char *header)
{
  FceHeader3 hdr;
  int i;

  memcpy(&hdr.Unknown1, header + 0x0000, (size_t)4);
  memcpy(&hdr.NumTriangles, header + 0x0004, (size_t)4);
  memcpy(&hdr.NumVertices, header + 0x0008, (size_t)4);
  memcpy(&hdr.NumArts, header + 0x000C, (size_t)4);

  memcpy(&hdr.VertTblOffset, header + 0x0010, (size_t)4);
  memcpy(&hdr.NormTblOffset, header + 0x0014, (size_t)4);
  memcpy(&hdr.TriaTblOffset, header + 0x0018, (size_t)4);

  memcpy(&hdr.Reserve1offset, header + 0x001C, (size_t)4);
  memcpy(&hdr.Reserve2offset, header + 0x0020, (size_t)4);
  memcpy(&hdr.Reserve3offset, header + 0x0024, (size_t)4);

  memcpy(&hdr.XHalfSize, header + 0x0028, (size_t)4);
  memcpy(&hdr.YHalfSize, header + 0x002C, (size_t)4);
  memcpy(&hdr.ZHalfSize, header + 0x0030, (size_t)4);

  memcpy(&hdr.NumDummies, header + 0x0034, (size_t)4);
  for (i = 0; i < FCELIB_MISC_Min(hdr.NumDummies, 16); ++i)
  {
    memcpy(&hdr.Dummies[i].x, header + 0x0038 + i * 12 + 0x0, (size_t)4);
    memcpy(&hdr.Dummies[i].y, header + 0x0038 + i * 12 + 0x4, (size_t)4);
    memcpy(&hdr.Dummies[i].z, header + 0x0038 + i * 12 + 0x8, (size_t)4);
  }

  memcpy(&hdr.NumParts, header + 0x00F8, (size_t)4);
  for (i = 0; i < FCELIB_MISC_Min(hdr.NumParts, 64); ++i)
  {
    memcpy(&hdr.PartPos[i].x, header + 0x00FC + i * 12 + 0x0, (size_t)4);
    memcpy(&hdr.PartPos[i].y, header + 0x00FC + i * 12 + 0x4, (size_t)4);
    memcpy(&hdr.PartPos[i].z, header + 0x00FC + i * 12 + 0x8, (size_t)4);
  }

  memcpy(&hdr.P1stVertices, header + 0x03FC, (size_t)(64 * 4));
  memcpy(&hdr.PNumVertices, header + 0x04FC, (size_t)(64 * 4));
  memcpy(&hdr.P1stTriangles, header + 0x05FC, (size_t)(64 * 4));
  memcpy(&hdr.PNumTriangles, header + 0x06FC, (size_t)(64 * 4));

  memcpy(&hdr.NumPriColors, header + 0x07FC, (size_t)4);
  for (i = 0; i < FCELIB_MISC_Min(hdr.NumPriColors, 16); ++i)
  {
    memcpy(&hdr.PriColors[i].hue,          header + 0x0800 + i * 16 + 0x00, (size_t)4);
    memcpy(&hdr.PriColors[i].saturation,   header + 0x0800 + i * 16 + 0x04, (size_t)4);
    memcpy(&hdr.PriColors[i].brightness,   header + 0x0800 + i * 16 + 0x08, (size_t)4);
    memcpy(&hdr.PriColors[i].transparency, header + 0x0800 + i * 16 + 0x0C, (size_t)4);
  }

  memcpy(&hdr.NumSecColors, header + 0x0900, (size_t)4);
  for (i = 0; i < FCELIB_MISC_Min(hdr.NumSecColors, 16); ++i)
  {
    memcpy(&hdr.SecColors[i].hue,          header + 0x0904 + i * 16 + 0x00, (size_t)4);
    memcpy(&hdr.SecColors[i].saturation,   header + 0x0904 + i * 16 + 0x04, (size_t)4);
    memcpy(&hdr.SecColors[i].brightness,   header + 0x0904 + i * 16 + 0x08, (size_t)4);
    memcpy(&hdr.SecColors[i].transparency, header + 0x0904 + i * 16 + 0x0C, (size_t)4);
  }

  memcpy(&hdr.DummyNames, header + 0x0A04, (size_t)(64 * 16));

  memcpy(&hdr.PartNames, header + 0x0E04, (size_t)(64 * 64));

  memcpy(&hdr.Unknown2, header + 0x1E04, (size_t)256);

  return hdr;
}

/* Assumes valid FCE data */
FceHeader4 FCELIB_FCETYPES_GetFceHeader4(const unsigned char *header)
{
  FceHeader4 hdr;
  int i;

  memcpy(&hdr.Version, header + 0x0000, (size_t)4);
  memcpy(&hdr.Unknown1, header + 0x0004, (size_t)4);
  memcpy(&hdr.NumTriangles, header + 0x0008, (size_t)4);
  memcpy(&hdr.NumVertices, header + 0x000c, (size_t)4);
  memcpy(&hdr.NumArts, header + 0x0010, (size_t)4);

  memcpy(&hdr.VertTblOffset, header + 0x0014, (size_t)4);
  memcpy(&hdr.NormTblOffset, header + 0x0018, (size_t)4);
  memcpy(&hdr.TriaTblOffset, header + 0x001c, (size_t)4);

  memcpy(&hdr.Reserve1offset, header + 0x0020, (size_t)4);
  memcpy(&hdr.Reserve2offset, header + 0x0024, (size_t)4);
  memcpy(&hdr.Reserve3offset, header + 0x0028, (size_t)4);

  memcpy(&hdr.UndamgdVertTblOffset, header + 0x002c, (size_t)4);
  memcpy(&hdr.UndamgdNormTblOffset, header + 0x0030, (size_t)4);
  memcpy(&hdr.DamgdVertTblOffset, header + 0x0034, (size_t)4);
  memcpy(&hdr.DamgdNormTblOffset, header + 0x0038, (size_t)4);

  memcpy(&hdr.Reserve4offset, header + 0x003c, (size_t)4);
  memcpy(&hdr.AnimationTblOffset, header + 0x0040, (size_t)4);
  memcpy(&hdr.Reserve5offset, header + 0x0044, (size_t)4);

  memcpy(&hdr.Reserve6offset, header + 0x0048, (size_t)4);

  memcpy(&hdr.XHalfSize, header + 0x004c, (size_t)4);
  memcpy(&hdr.YHalfSize, header + 0x0050, (size_t)4);
  memcpy(&hdr.ZHalfSize, header + 0x0054, (size_t)4);

  memcpy(&hdr.NumDummies, header + 0x0058, (size_t)4);
  for (i = 0; i < FCELIB_MISC_Min(hdr.NumDummies, 16); ++i)
  {
    memcpy(&hdr.Dummies[i].x, header + 0x005c + i * 12 + 0x0, (size_t)4);
    memcpy(&hdr.Dummies[i].y, header + 0x005c + i * 12 + 0x4, (size_t)4);
    memcpy(&hdr.Dummies[i].z, header + 0x005c + i * 12 + 0x8, (size_t)4);
  }

  memcpy(&hdr.NumParts, header + 0x011c, (size_t)4);
  for (i = 0; i < FCELIB_MISC_Min(hdr.NumParts, 64); ++i)
  {
    memcpy(&hdr.PartPos[i].x, header + 0x0120 + i * 12 + 0x0, (size_t)4);
    memcpy(&hdr.PartPos[i].y, header + 0x0120 + i * 12 + 0x4, (size_t)4);
    memcpy(&hdr.PartPos[i].z, header + 0x0120 + i * 12 + 0x8, (size_t)4);
  }

  memcpy(&hdr.P1stVertices, header + 0x0420, (size_t)(64 * 4));
  memcpy(&hdr.PNumVertices, header + 0x0520, (size_t)(64 * 4));
  memcpy(&hdr.P1stTriangles, header + 0x0620, (size_t)(64 * 4));
  memcpy(&hdr.PNumTriangles, header + 0x0720, (size_t)(64 * 4));

  memcpy(&hdr.NumColors, header + 0x0820, (size_t)4);
  for (i = 0; i < FCELIB_MISC_Min(hdr.NumColors, 16); ++i)
  {
    memcpy(&hdr.PriColors[i].hue,          header + 0x0824 + i * 4 + 0x00, (size_t)1);
    memcpy(&hdr.PriColors[i].saturation,   header + 0x0824 + i * 4 + 0x04, (size_t)1);
    memcpy(&hdr.PriColors[i].brightness,   header + 0x0824 + i * 4 + 0x08, (size_t)1);
    memcpy(&hdr.PriColors[i].transparency, header + 0x0824 + i * 4 + 0x0C, (size_t)1);

    memcpy(&hdr.IntColors[i].hue,          header + 0x0864 + i * 4 + 0x00, (size_t)1);
    memcpy(&hdr.IntColors[i].saturation,   header + 0x0864 + i * 4 + 0x04, (size_t)1);
    memcpy(&hdr.IntColors[i].brightness,   header + 0x0864 + i * 4 + 0x08, (size_t)1);
    memcpy(&hdr.IntColors[i].transparency, header + 0x0864 + i * 4 + 0x0C, (size_t)1);

    memcpy(&hdr.SecColors[i].hue,          header + 0x08a4 + i * 4 + 0x00, (size_t)1);
    memcpy(&hdr.SecColors[i].saturation,   header + 0x08a4 + i * 4 + 0x04, (size_t)1);
    memcpy(&hdr.SecColors[i].brightness,   header + 0x08a4 + i * 4 + 0x08, (size_t)1);
    memcpy(&hdr.SecColors[i].transparency, header + 0x08a4 + i * 4 + 0x0C, (size_t)1);

    memcpy(&hdr.DriColors[i].hue,          header + 0x08e4 + i * 4 + 0x00, (size_t)1);
    memcpy(&hdr.DriColors[i].saturation,   header + 0x08e4 + i * 4 + 0x04, (size_t)1);
    memcpy(&hdr.DriColors[i].brightness,   header + 0x08e4 + i * 4 + 0x08, (size_t)1);
    memcpy(&hdr.DriColors[i].transparency, header + 0x08e4 + i * 4 + 0x0C, (size_t)1);
  }

  memcpy(&hdr.Unknown3, header + 0x0924, (size_t)4);
  memcpy(&hdr.Unknown2, header + 0x0928, (size_t)256);

  memcpy(&hdr.DummyNames, header + 0x0a28, (size_t)(64 * 16));

  memcpy(&hdr.PartNames, header + 0x0e28, (size_t)(64 * 64));
  memcpy(&hdr.Unknown4, header + 0x1e28, (size_t)528);

  return hdr;
}


/* Fce4 validation ---------------------------------------------------------- */

int FCELIB_FCETYPES_Fce3ComputeSize(const int NumVertices, const int NumTriangles)
{
  int fsize = 0;

  fsize += 0x1F04;             /* (int)sizeof(struct FceHeader3); */
  fsize += 80 * NumVertices;   /* ((4*12) + 32) x NumVertices */
  fsize += 56 * NumTriangles;  /* 56 x NumTriangles*/

  return fsize;
}

/* Assumes sizeof(*header) = 0x1F04. Returns boolean */
int FCELIB_FCETYPES_Fce3ValidateHeader(const void *header, const int infilesize)
{
  int retv = 1;
  int i;
  int count_verts = 0;
  int count_triags = 0;
  int size;
  int dist_to_eof;
  FceHeader3 hdr = FCELIB_FCETYPES_GetFceHeader3((unsigned char *)header);

  /*
    parts: triangle indices within bounds?
   */

  /* aborts */
  if (hdr.NumTriangles < 0)
  {
    fprintf(stderr, "Invalid number of triangles (%d)\n", hdr.NumTriangles);
    retv = 0;
  }

  if (hdr.NumVertices < 0)
  {
    fprintf(stderr, "Invalid number of vertices (%d)\n", hdr.NumVertices);
    retv = 0;
  }

  if ((hdr.NumDummies > 16) || (hdr.NumDummies < 0))
  {
    fprintf(stderr, "Invalid number of dummies (%d)\n", hdr.NumDummies);
    retv = 0;
  }

  if ((hdr.NumParts > 64) || (hdr.NumParts < 0))
  {
    fprintf(stderr, "Invalid number of parts (%d)\n", hdr.NumParts);
    retv = 0;
  }

  if ((hdr.NumPriColors > 16) || (hdr.NumPriColors < 0))
  {
    fprintf(stderr, "Invalid number of primary colors (%d)\n",
                    hdr.NumPriColors);
    retv = 0;
  }
  if ((hdr.NumSecColors > 16) || (hdr.NumSecColors < 0))
  {
    fprintf(stderr, "Invalid number of secondary colors (%d)\n",
                    hdr.NumSecColors);
    retv = 0;
  }

  /* Vertices, triangles counts */
  for (i = 0; i < hdr.NumParts; ++i)
  {
    if ((hdr.PNumTriangles[i] > 0) && (hdr.PNumVertices[i] < 3))
    {
      fprintf(stderr, "Part %d requires at least 3 vertices in total, counted %d\n\n",
                      i, hdr.PNumVertices[i]);
      retv = 0;
    }

    count_verts += hdr.PNumVertices[i];
    count_triags += hdr.PNumTriangles[i];
  }
  if (hdr.NumVertices < count_verts)
  {
    fprintf(stderr, "Expects %d vertices in total, counted %d\n",
                    hdr.NumVertices, count_verts);
    retv = 0;
  }
  if (hdr.NumTriangles < count_triags)
  {
    fprintf(stderr, "Expects %d triangles in total, counted %d\n",
                    hdr.NumTriangles, count_triags);
    retv = 0;
  }
  if ((size = FCELIB_FCETYPES_Fce3ComputeSize(count_verts, count_triags)) > infilesize)
  {
    fprintf(stderr, "FCE filesize too small %d (requires %d) %d\n",
                    size,
                    infilesize, infilesize - size);
    retv = 0;
  }
  size = 0;

  /* Vertices, triangles areas: parts non-overlapping, within bounds (do nothing
     when zero verts, triags) */
  for (i = 0; i < hdr.NumParts - 1; ++i)
  {
    /* Combined with other checks, guarantees verts, triags stay within their
       areas, respectively */
    if ((hdr.P1stVertices[i] < 0) ||
        (hdr.P1stVertices[i] + hdr.PNumVertices[i] > hdr.NumVertices))
    {
      fprintf(stderr, "Part out of bounds %d (vertices)\n", i);
      retv = 0;
      break;
    }
    if (hdr.P1stVertices[i] + hdr.PNumVertices[i] > hdr.P1stVertices[i + 1])
    {
      fprintf(stderr, "Overlapping parts %d, %d (vertices)\n", i, i + 1);
      retv = 0;
      break;
    }

    if ((hdr.P1stTriangles[i] < 0) ||
        (hdr.P1stTriangles[i] + hdr.PNumTriangles[i] > hdr.NumTriangles))
    {
      fprintf(stderr, "Part out of bounds %d (triangles)\n", i);
      retv = 0;
      break;
    }
    if (hdr.P1stTriangles[i] + hdr.PNumTriangles[i] > hdr.P1stTriangles[i + 1])
    {
      fprintf(stderr, "Overlapping parts %d, %d (triangles)\n", i, i + 1);
      retv = 0;
      break;
    }
  }
  if ((hdr.NumParts > 0) && (hdr.NumParts <= 64))
  {
    if ((hdr.P1stVertices[hdr.NumParts - 1] < 0) ||
        (hdr.P1stVertices[hdr.NumParts - 1] + hdr.PNumVertices[hdr.NumParts - 1] > hdr.NumVertices))
    {
      fprintf(stderr, "Part out of bounds %d (vertices)\n", hdr.NumParts - 1);
      retv = 0;
    }

    if ((hdr.P1stTriangles[hdr.NumParts - 1] < 0) ||
        (hdr.P1stTriangles[hdr.NumParts - 1] + hdr.PNumTriangles[hdr.NumParts - 1] > hdr.NumTriangles))
    {
      fprintf(stderr, "Part out of bounds %d (triangles)\n", hdr.NumParts - 1);
      retv = 0;
    }
  }

  /* Validate filesize, area offsets, area sizes, areas non-overlapping

     FCE3 allows NumVertices, and PNumTriangles to be larger than the truth.
     Fcecodec requires that area sizes relate to given values, which FCE3 allows
     (FCE3 is even less restrictive).

     Note: Fcecodec warns about, accepts (VertTblOffset > 0)
  */
  if ((size = FCELIB_FCETYPES_Fce3ComputeSize(hdr.NumVertices, hdr.NumTriangles)) != infilesize)
  {
    fprintf(stderr, "FCE filesize mismatch %d (expects %d) %d\n",
                    infilesize,
                    size, infilesize - size);
    retv = 0;
  }

  dist_to_eof = 0;
  dist_to_eof += 12 * hdr.NumVertices;
  if ((hdr.Reserve3offset < 0) || (infilesize - 0x1F04 - hdr.Reserve3offset) != dist_to_eof)
  {
    fprintf(stderr, "Reserve3offset invalid 0x%04x (expects 0x%04x)\n",
                    hdr.Reserve3offset, dist_to_eof);
    retv = 0;
  }
  dist_to_eof += 12 * hdr.NumVertices;
  if ((hdr.Reserve2offset < 0) || (infilesize - 0x1F04 - hdr.Reserve2offset) != dist_to_eof)
  {
    fprintf(stderr, "Reserve2offset invalid 0x%04x (expects 0x%04x)\n",
                    hdr.Reserve2offset, dist_to_eof);
    retv = 0;
  }
  dist_to_eof += 32 * hdr.NumVertices;
  if ((hdr.Reserve1offset < 0) || (infilesize - 0x1F04 - hdr.Reserve1offset) != dist_to_eof)
  {
    fprintf(stderr, "Reserve1offset invalid 0x%04x (expects 0x%04x)\n",
                    hdr.Reserve1offset, dist_to_eof);
    retv = 0;
  }

  dist_to_eof += 56 * hdr.NumTriangles;
  if ((hdr.TriaTblOffset < 0) || (infilesize - 0x1F04 - hdr.TriaTblOffset) != dist_to_eof)
  {
    fprintf(stderr, "TriaTblOffset invalid 0x%04x (expects 0x%04x)\n",
                    hdr.TriaTblOffset, dist_to_eof);
    retv = 0;
  }
  dist_to_eof += 12 * hdr.NumVertices;
  if ((hdr.NormTblOffset < 0) || (infilesize - 0x1F04 - hdr.NormTblOffset) != dist_to_eof)
  {
    fprintf(stderr, "NormTblOffset invalid 0x%04x (expects 0x%04x)\n",
                    hdr.NormTblOffset, dist_to_eof);
    retv = 0;
  }
  dist_to_eof += 12 * hdr.NumVertices;
  if ((hdr.VertTblOffset < 0) || (infilesize - 0x1F04 - hdr.VertTblOffset) != dist_to_eof)
  {
    fprintf(stderr, "VertTblOffset invalid 0x%04x (expects 0x%04x)\n",
                    hdr.VertTblOffset, dist_to_eof);
    retv = 0;
  }

  /* warnings */
  if (retv)
  {
    if (hdr.NumVertices != count_verts)
    {
      fprintf(stderr, "Warning Expects %d vertices in total, counted %d\n",
                      hdr.NumVertices, count_verts);
    }
    if (hdr.NumTriangles != count_triags)
    {
      fprintf(stderr, "Warning Expects %d triangles in total, counted %d\n",
                      hdr.NumTriangles, count_triags);
    }
  }

  if (hdr.NumArts != 1)
    fprintf(stderr, "Warning NumArts != 1 (%d)\n", hdr.NumArts);

  if (hdr.VertTblOffset)
  {
    fprintf(stderr, "Warning VertTblOffset = 0x%04x (expects 0x0000)\n",
                    hdr.VertTblOffset);
  }

  if (hdr.NumPriColors < hdr.NumSecColors)
  {
    fprintf(stderr, "Warning NumPriColors < NumSecColors (%d, %d)\n",
                    hdr.NumPriColors, hdr.NumSecColors);
  }

  if ((hdr.XHalfSize < 0.001) || (hdr.ZHalfSize < 0.001) ||
      (hdr.XHalfSize * hdr.ZHalfSize < 0.1) ||
      (hdr.YHalfSize < 0.0))
  {
    fprintf(stderr, "Warning HalfSizes may crash game\n");
  }

  return retv;
}


/* Fce4 validation ---------------------------------------------------------- */

int FCELIB_FCETYPES_Fce4ComputeSize(const int Version,
                                 const int NumVertices, const int NumTriangles)
{
  int fsize = 0;

  fsize += 0x2038;             /* (int)sizeof(struct FceHeader4); */
  fsize += 140 * NumVertices;  /* ((8*12) + 32 + (3*4)) x NumVertices */
  fsize += 68 * NumTriangles;  /* (56 + 12) x NumTriangles*/

  if (Version == 0x00101015)
    fsize += NumVertices;  /* Reserve6 is larger */

  return fsize;
}

/* Assumes sizeof(*header) = 0x2038. Returns boolean */
int FCELIB_FCETYPES_Fce4ValidateHeader(const void *header, const int infilesize)
{
  int retv = 1;
  int i;
  int count_verts = 0;
  int count_triags = 0;
  int size;
  int dist_to_eof;
  FceHeader4 hdr = FCELIB_FCETYPES_GetFceHeader4((unsigned char *)header);

  /* aborts */
  if (hdr.NumTriangles < 0)
  {
    fprintf(stderr, "Invalid number of triangles (%d)\n", hdr.NumTriangles);
    retv = 0;
  }

  if (hdr.NumVertices < 0)
  {
    fprintf(stderr, "Invalid number of vertices (%d)\n", hdr.NumVertices);
    retv = 0;
  }

  if ((hdr.NumDummies > 16) || (hdr.NumDummies < 0))
  {
    fprintf(stderr, "Invalid number of dummies (%d)\n", hdr.NumDummies);
    retv = 0;
  }

  if ((hdr.NumParts > 64) || (hdr.NumParts < 0))
  {
    fprintf(stderr, "Invalid number of parts (%d)\n", hdr.NumParts);
    retv = 0;
  }

  if (hdr.Version == 0x00101014)
  {
    if ((hdr.NumColors > 16) || (hdr.NumColors < 0))
    {
      fprintf(stderr, "Invalid number of colors (%d)\n", hdr.NumColors);
      retv = 0;
    }
  }

  /* Vertices, triangles counts */
  for (i = 0; i < hdr.NumParts; ++i)
  {
    if ((hdr.PNumTriangles[i] > 0) && (hdr.PNumVertices[i] < 3))
    {
      fprintf(stderr, "Part %d requires at least 3 vertices in total, counted %d\n\n",
                      i, hdr.PNumVertices[i]);
      retv = 0;
    }

    count_verts += hdr.PNumVertices[i];
    count_triags += hdr.PNumTriangles[i];
  }
  if (hdr.NumVertices < count_verts)
  {
    fprintf(stderr, "Expects %d vertices in total, counted %d\n",
                    hdr.NumVertices, count_verts);
    retv = 0;
  }
  if (hdr.NumTriangles < count_triags)
  {
    fprintf(stderr, "Expects %d triangles in total, counted %d\n",
                    hdr.NumTriangles, count_triags);
    retv = 0;
  }
  if ((size = FCELIB_FCETYPES_Fce4ComputeSize(hdr.Version, count_verts, count_triags)) > infilesize)
  {
    /* Are just Reserve5, Reserve6 invalid? ex. 99viper/?.fce */

    int area_5_6_size = 4 * hdr.NumVertices + 12 * hdr.NumTriangles;
    if (hdr.Version == 0x00101015)
      area_5_6_size += hdr.NumVertices;

    if (size - area_5_6_size > infilesize - abs(infilesize - 0x2038 - hdr.Reserve5offset))
    {
      fprintf(stderr, "FCE filesize mismatch %d (expects %d) %d\n",
                      infilesize,
                      size, infilesize - size);
      fprintf(stderr, "count_verts=%d , count_triags=%d\n", count_verts, count_triags);
      fprintf(stderr, "until 5: %d (expects %d) %d\n",
                      infilesize - abs(infilesize - 0x2038 - hdr.Reserve5offset),
                      size - area_5_6_size,
                      infilesize - abs(infilesize - 0x2038 - hdr.Reserve5offset) - (size - area_5_6_size));
      retv = 0;
    }
    else
    {
      fprintf(stderr, "Warning FCE filesize mismatch (Reserve5offset, Reserve6offset invalid)\n");
    }
  }
  size = 0;

  /* Vertices, triangles areas: parts non-overlapping, within bounds (do nothing
     when zero verts, triags) */
  for (i = 0; i < hdr.NumParts - 1; ++i)
  {
    /* Combined with other checks, guarantees verts, triags stay within their
       areas, respectively */
    if ((hdr.P1stVertices[i] < 0) ||
        (hdr.P1stVertices[i] + hdr.PNumVertices[i] > hdr.NumVertices))
    {
      fprintf(stderr, "Part out of bounds %d (vertices)\n", i);
      retv = 0;
      break;
    }
    if (hdr.P1stVertices[i] + hdr.PNumVertices[i] > hdr.P1stVertices[i + 1])
    {
      fprintf(stderr, "Overlapping parts %d, %d (vertices)\n", i, i + 1);
      retv = 0;
      break;
    }

    if ((hdr.P1stTriangles[i] < 0) ||
        (hdr.P1stTriangles[i] + hdr.PNumTriangles[i] > hdr.NumTriangles))
    {
      fprintf(stderr, "Part out of bounds %d (triangles)\n", i);
      retv = 0;
      break;
    }
    if (hdr.P1stTriangles[i] + hdr.PNumTriangles[i] > hdr.P1stTriangles[i + 1])
    {
      fprintf(stderr, "Overlapping parts %d, %d (triangles)\n", i, i + 1);
      retv = 0;
      break;
    }
  }
  if ((hdr.NumParts > 0) && (hdr.NumParts <= 64))
  {
    if ((hdr.P1stVertices[hdr.NumParts - 1] < 0) ||
        (hdr.P1stVertices[hdr.NumParts - 1] + hdr.PNumVertices[hdr.NumParts - 1] > hdr.NumVertices))
    {
      fprintf(stderr, "Part out of bounds %d (vertices)\n", hdr.NumParts - 1);
      retv = 0;
    }

    if ((hdr.P1stTriangles[hdr.NumParts - 1] < 0) ||
        (hdr.P1stTriangles[hdr.NumParts - 1] + hdr.PNumTriangles[hdr.NumParts - 1] > hdr.NumTriangles))
    {
      fprintf(stderr, "Part out of bounds %d (triangles)\n", hdr.NumParts - 1);
      retv = 0;
    }
  }

  /* Validate filesize, area offsets, area sizes, areas non-overlapping

     Fcecodec requires that area sizes relate to given NumVertices, and
     PNumTriangles

     Note: Fcecodec warns about, accepts (VertTblOffset > 0)
  */
 if ((size = FCELIB_FCETYPES_Fce4ComputeSize(hdr.Version, hdr.NumVertices, hdr.NumTriangles)) != infilesize)
  {
    /* Are just Reserve5, Reserve6 invalid? ex. 99viper/?.fce */

    int area_5_6_size = 4 * hdr.NumVertices + 12 * hdr.NumTriangles;
    if (hdr.Version == 0x00101015)
      area_5_6_size += hdr.NumVertices;

    if (size - area_5_6_size != infilesize - abs(infilesize - 0x2038 - hdr.Reserve5offset))
    {
      fprintf(stderr, "FCE filesize mismatch %d (expects %d) %d\n",
                      infilesize,
                      size, infilesize - size);
      fprintf(stderr, "NumVertices=%d , NumTriangles=%d\n", hdr.NumVertices, hdr.NumTriangles);
      fprintf(stderr, "until 5: %d (expects %d) %d\n",
                      infilesize - abs(infilesize - 0x2038 - hdr.Reserve5offset),
                      size - area_5_6_size,
                      infilesize - abs(infilesize - 0x2038 - hdr.Reserve5offset) - (size - area_5_6_size));
      retv = 0;
    }
    else
    {
      fprintf(stderr, "Warning FCE filesize mismatch (Reserve5offset, Reserve6offset invalid)\n");
    }
  }


  /* Warn about Reserve5offset, Reserve6offset mismatches
     Fcecodec allows, if all areas are within bounds */
  if ((hdr.Reserve5offset > hdr.Reserve6offset) ||
      (0x2038 + hdr.Reserve6offset > infilesize) ||
      (0x2038 + hdr.Reserve5offset > infilesize))
  {
    fprintf(stderr, "Reserve5offset or Reserve6offset out of bounds\n");
    printf("Reserve5offset = 0x%04x (0x%x), Size = %d\n", hdr.Reserve5offset, 0x2038 + hdr.Reserve5offset, hdr.Reserve6offset - hdr.Reserve5offset);
    printf("Reserve6offset = 0x%04x (0x%x), Size = %d\n", hdr.Reserve6offset, 0x2038 + hdr.Reserve6offset, infilesize - 0x2038 - hdr.Reserve6offset);
    retv = 0;
  }

  dist_to_eof = 12 * hdr.NumTriangles;
  if (hdr.Version == 0x00101015)
    dist_to_eof += hdr.NumVertices;
  if ((hdr.Reserve6offset < 0) || (infilesize - 0x2038 - hdr.Reserve6offset) != dist_to_eof)
  {
    fprintf(stderr, "Warning Reserve6offset invalid 0x%04x (expects 0x%04x) %d\n",
                    hdr.Reserve6offset, infilesize - 0x2038 - dist_to_eof,
                    hdr.Reserve6offset - (infilesize - 0x2038 - dist_to_eof));
  }

  dist_to_eof += 4 * hdr.NumVertices;
  if ((hdr.Reserve5offset < 0) || (infilesize - 0x2038 - hdr.Reserve5offset) != dist_to_eof)
  {
    fprintf(stderr, "Warning Reserve5offset invalid 0x%04x (expects 0x%04x)\n",
                    hdr.Reserve5offset, infilesize - 0x2038 - dist_to_eof);
  }

  /* Forget about Reserve5, Reserve6 */
  {
    int area_5_6_size = abs(infilesize - 0x2038 - hdr.Reserve5offset);
    dist_to_eof = 0;

    dist_to_eof += 4 * hdr.NumVertices;
    if ((hdr.AnimationTblOffset < 0) || (infilesize - 0x2038 - hdr.AnimationTblOffset - area_5_6_size) != dist_to_eof)
    {
      fprintf(stderr, "AnimationTblOffset invalid 0x%04x (expects 0x%04x)\n",
                      hdr.AnimationTblOffset, infilesize - 0x2038 - dist_to_eof - area_5_6_size);
      retv = 0;
    }
    dist_to_eof += 4 * hdr.NumVertices;
    if ((hdr.Reserve4offset < 0) || (infilesize - 0x2038 - hdr.Reserve4offset - area_5_6_size) != dist_to_eof)
    {
      fprintf(stderr, "Reserve4offset invalid 0x%04x (expects 0x%04x)\n",
                      hdr.Reserve4offset, infilesize - 0x2038 - dist_to_eof - area_5_6_size);
      retv = 0;
    }

    dist_to_eof += 12 * hdr.NumVertices;
    if ((hdr.DamgdNormTblOffset < 0) || (infilesize - 0x2038 - hdr.DamgdNormTblOffset - area_5_6_size) != dist_to_eof)
    {
      fprintf(stderr, "DamgdNormTblOffset invalid 0x%04x (expects 0x%04x)\n",
                      hdr.DamgdNormTblOffset, infilesize - 0x2038 - dist_to_eof - area_5_6_size);
      retv = 0;
    }
    dist_to_eof += 12 * hdr.NumVertices;
    if ((hdr.DamgdVertTblOffset < 0) || (infilesize - 0x2038 - hdr.DamgdVertTblOffset - area_5_6_size) != dist_to_eof)
    {
      fprintf(stderr, "DamgdVertTblOffset invalid 0x%04x (expects 0x%04x)\n",
                      hdr.DamgdVertTblOffset, infilesize - 0x2038 - dist_to_eof - area_5_6_size);
      retv = 0;
    }
    dist_to_eof += 12 * hdr.NumVertices;
    if ((hdr.UndamgdNormTblOffset < 0) || (infilesize - 0x2038 - hdr.UndamgdNormTblOffset - area_5_6_size) != dist_to_eof)
    {
      fprintf(stderr, "UndamgdNormTblOffset invalid 0x%04x (expects 0x%04x)\n",
                      hdr.UndamgdNormTblOffset, infilesize - 0x2038 - dist_to_eof - area_5_6_size);
      retv = 0;
    }
    dist_to_eof += 12 * hdr.NumVertices;
    if ((hdr.UndamgdVertTblOffset < 0) || (infilesize - 0x2038 - hdr.UndamgdVertTblOffset - area_5_6_size) != dist_to_eof)
    {
      fprintf(stderr, "UndamgdVertTblOffset invalid 0x%04x (expects 0x%04x)\n",
                      hdr.UndamgdVertTblOffset, infilesize - 0x2038 - dist_to_eof - area_5_6_size);
      retv = 0;
    }

    dist_to_eof += 12 * hdr.NumVertices;
    if ((hdr.Reserve3offset < 0) || (infilesize - 0x2038 - hdr.Reserve3offset - area_5_6_size) != dist_to_eof)
    {
      fprintf(stderr, "Reserve3offset invalid 0x%04x (expects 0x%04x)\n",
                      hdr.Reserve3offset, infilesize - 0x2038 - dist_to_eof - area_5_6_size);
      retv = 0;
    }
    dist_to_eof += 12 * hdr.NumVertices;
    if ((hdr.Reserve2offset < 0) || (infilesize - 0x2038 - hdr.Reserve2offset - area_5_6_size) != dist_to_eof)
    {
      fprintf(stderr, "Reserve2offset invalid 0x%04x (expects 0x%04x)\n",
                      hdr.Reserve2offset, infilesize - 0x2038 - dist_to_eof - area_5_6_size);
      retv = 0;
    }
    dist_to_eof += 32 * hdr.NumVertices;
    if ((hdr.Reserve1offset < 0) || (infilesize - 0x2038 - hdr.Reserve1offset - area_5_6_size) != dist_to_eof)
    {
      fprintf(stderr, "Reserve1offset invalid 0x%04x (expects 0x%04x)\n",
                      hdr.Reserve1offset, infilesize - 0x2038 - dist_to_eof - area_5_6_size);
      retv = 0;
    }

    dist_to_eof += 56 * hdr.NumTriangles;
    if ((hdr.TriaTblOffset < 0) || (infilesize - 0x2038 - hdr.TriaTblOffset - area_5_6_size) != dist_to_eof)
    {
      fprintf(stderr, "TriaTblOffset invalid 0x%04x (expects 0x%04x)\n",
                      hdr.TriaTblOffset, infilesize - 0x2038 - dist_to_eof - area_5_6_size);
      retv = 0;
    }
    dist_to_eof += 12 * hdr.NumVertices;
    if ((hdr.NormTblOffset < 0) || (infilesize - 0x2038 - hdr.NormTblOffset - area_5_6_size) != dist_to_eof)
    {
      fprintf(stderr, "NormTblOffset invalid 0x%04x (expects 0x%04x)\n",
                      hdr.NormTblOffset, infilesize - 0x2038 - dist_to_eof - area_5_6_size);
      retv = 0;
    }
    dist_to_eof += 12 * hdr.NumVertices;
    if ((hdr.VertTblOffset < 0) || (infilesize - 0x2038 - hdr.VertTblOffset - area_5_6_size) != dist_to_eof)
    {
      fprintf(stderr, "VertTblOffset invalid 0x%04x (expects 0x%04x)\n",
                      hdr.VertTblOffset, infilesize - 0x2038 - dist_to_eof - area_5_6_size);
      retv = 0;
    }
  }  /* end: Forget about Reserve5, Reserve6 */

  /* warnings */
  if (retv)
  {
    if (hdr.NumVertices != count_verts)
    {
      fprintf(stderr, "Warning Expects %d vertices in total, counted %d\n",
                      hdr.NumVertices, count_verts);
    }
    if (hdr.NumTriangles != count_triags)
    {
      fprintf(stderr, "Warning Expects %d triangles in total, counted %d\n",
                      hdr.NumTriangles, count_triags);
    }
  }

  if (hdr.NumArts != 1)
    fprintf(stderr, "Warning NumArts != 1 (%d)\n", hdr.NumArts);

  if (hdr.VertTblOffset)
  {
    fprintf(stderr, "Warning VertTblOffset = 0x%04x (expects 0x0000)\n",
                    hdr.VertTblOffset);
  }

  if ((hdr.XHalfSize < 0.001) || (hdr.ZHalfSize < 0.001) ||
      (hdr.XHalfSize * hdr.ZHalfSize < 0.1) ||
      (hdr.YHalfSize < 0.0))
  {
    fprintf(stderr, "Warning HalfSizes may crash game\n");
  }

  return retv;
}


/* print info --------------------------------------------------------------- */

void FCELIB_FCETYPES_PrintHeaderFce4(const int fce_size, const void *header)
{
  int i;
  FceHeader4 hdr = FCELIB_FCETYPES_GetFceHeader4((unsigned char *)header);
  int verts = 0;
  int triags = 0;

  printf("Filesize = %d (0x%x)\n", fce_size, fce_size);

  if (hdr.Version == 0x00101014)
    printf("Version = FCE4\n");
  else  /* 0x00101015 */
    printf("Version = FCE4M\n");

  printf("NumTriangles = %d (* 12 = %d) (* 56 = %d)\n", hdr.NumTriangles, 12 * hdr.NumTriangles, 56 * hdr.NumTriangles);
  printf("NumVertices = %d (* 4 = %d)  (* 12 = %d)  (* 32 = %d)\n", hdr.NumVertices, 4 * hdr.NumVertices, 12 * hdr.NumVertices, 32 * hdr.NumVertices);
  printf("NumArts = %d\n", hdr.NumArts);

  printf("XHalfSize = %f\n", hdr.XHalfSize);
  printf("YHalfSize = %f\n", hdr.YHalfSize);
  printf("ZHalfSize = %f\n", hdr.ZHalfSize);

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
  if (hdr.Version == 0x00101015)
  {
    tColor4 c;
    c.hue          = *((unsigned char *)header + 0x0924 + 0x0);
    c.saturation   = *((unsigned char *)header + 0x0924 + 0x1);
    c.brightness   = *((unsigned char *)header + 0x0924 + 0x2);
    c.transparency = *((unsigned char *)header + 0x0924 + 0x3);

    printf("Unknown3 (0x0924) as color  %3u, %3u, %3u, %3u\n",
           c.hue, c.saturation,
           c.brightness, c.transparency);
  }

  printf("Parts:\n"
                  "Idx  Verts       Triangles   (PartPos)                         Name\n");
  for (i = 0; i < FCELIB_MISC_Min(hdr.NumParts, 64); ++i)
  {
    printf(" %2d  %5d %5d %5d %5d (%9f, %9f, %9f) %s\n",
           i,
           hdr.P1stVertices[i],
           hdr.PNumVertices[i],
           hdr.P1stTriangles[i],
           hdr.PNumTriangles[i],
           hdr.PartPos[i].x, hdr.PartPos[i].y, hdr.PartPos[i].z,
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
  for (i = 0; i < FCELIB_MISC_Min(hdr.NumDummies, 16); ++i)
  {
    printf(" (%9f, %9f, %9f) %s\n",
           hdr.Dummies[i].x, hdr.Dummies[i].y, hdr.Dummies[i].z,
           hdr.DummyNames + (i * 64));
  }

  printf("Car colors (hue, saturation, brightness, transparency):\n");
  for (i = 0; i < FCELIB_MISC_Min(hdr.NumColors, 16); ++i)
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

void FCELIB_FCETYPES_PrintHeaderFce3(const int fce_size, const void *header)
{
  int i;
  FceHeader3 hdr = FCELIB_FCETYPES_GetFceHeader3((unsigned char *)header);
  int verts = 0;
  int triags = 0;

  printf("Filesize = %d (0x%x)\n", fce_size, fce_size);

  printf("Version = FCE3\n");

  printf("NumTriangles = %d (* 56 = %d)\n", hdr.NumTriangles, 56 * hdr.NumTriangles);
  printf("NumVertices = %d (* 12 = %d)  (* 32 = %d)\n", hdr.NumVertices, 12 * hdr.NumVertices, 32 * hdr.NumVertices);
  printf("NumArts = %d\n", hdr.NumArts);

  printf("XHalfSize = %f\n", hdr.XHalfSize);
  printf("YHalfSize = %f\n", hdr.YHalfSize);
  printf("ZHalfSize = %f\n", hdr.ZHalfSize);

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
  for (i = 0; i < FCELIB_MISC_Min(kFceLibImplementedFce3Parts, hdr.NumParts); ++i)
  {
    printf(" %2d  %5d %5d %5d %5d (%9f, %9f, %9f) %20s %s\n",
           i,
           hdr.P1stVertices[i],
           hdr.PNumVertices[i],
           hdr.P1stTriangles[i],
           hdr.PNumTriangles[i],
           hdr.PartPos[i].x, hdr.PartPos[i].y, hdr.PartPos[i].z,
           kFce3PartsNames[i],
           hdr.PartNames + (i * 64));

    verts += hdr.PNumVertices[i];
    triags += hdr.PNumTriangles[i];
  }
  for (i = FCELIB_MISC_Min(kFceLibImplementedFce3Parts, hdr.NumParts); i < FCELIB_MISC_Min(64, hdr.NumParts); ++i)
  {
    printf(" %2d  %5d %5d %5d %5d (%9f, %9f, %9f) %20s %s\n",
           i,
           hdr.P1stVertices[i],
           hdr.PNumVertices[i],
           hdr.P1stTriangles[i],
           hdr.PNumTriangles[i],
           hdr.PartPos[i].x, hdr.PartPos[i].y, hdr.PartPos[i].z,
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
  for (i = 0; i < FCELIB_MISC_Min(hdr.NumDummies, 16); ++i)
  {
    printf(" (%9f, %9f, %9f) %s\n",
           hdr.Dummies[i].x, hdr.Dummies[i].y, hdr.Dummies[i].z,
           hdr.DummyNames + (i * 64));
  }

  printf("Car colors (hue, saturation, brightness, transparency):\n");
  for (i = 0; i < FCELIB_MISC_Min(hdr.NumPriColors, 16); ++i)
  {
    printf("%2d  Primary     %3d, %3d, %3d, %3d\n", i,
           hdr.PriColors[i].hue, hdr.PriColors[i].saturation,
           hdr.PriColors[i].brightness, hdr.PriColors[i].transparency);
    printf("%2d  Secondary   %3d, %3d, %3d, %3d\n", i,
           hdr.SecColors[i].hue, hdr.SecColors[i].saturation,
           hdr.SecColors[i].brightness, hdr.SecColors[i].transparency);
  }
}

#ifdef __cplusplus
}  /* extern "C" */
#endif

#ifdef __cplusplus
}  /* namespace fcelib */
#endif

#endif  /* FCELIB_FCETYPES_H */