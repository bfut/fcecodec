# Obj2Fce scripts for _fcecodec_

Convenience converter scripts.

```
bfut_Obj2Fce3.py
bfut_Obj2Fce3 (do not center parts).py
bfut_Obj2Fce3 (do not center parts, material to texpage).py
bfut_Obj2Fce3 (do not center parts, material to triagflag).py
bfut_Obj2Fce3 (material to texpage).py
bfut_Obj2Fce3 (material to triagflag).py
bfut_Obj2Fce4.py
bfut_Obj2Fce4 (do not center parts).py
bfut_Obj2Fce4 (do not center parts, material to texpage).py
bfut_Obj2Fce4 (do not center parts, material to triagflag).py
bfut_Obj2Fce4M.py
bfut_Obj2Fce4 (material to texpage).py
bfut_Obj2Fce4 (material to triagflag).py
bfut_Obj2Fce4M (do not center parts).py
bfut_Obj2Fce4M (do not center parts, material to texpage).py
bfut_Obj2Fce4M (do not center parts, material to triagflag).py
bfut_Obj2Fce4M (material to texpage).py
bfut_Obj2Fce4M (material to triagflag).py
```

## Installation

Install _fcecodec_ from https://github.com/bfut/fcecodec. The scripts can be found in the [_/scripts_](https://github.com/bfut/fcecodec/tree/main/scripts) folder.

## How to use

Execute in terminal with

    python <bfut_Obj2Fce*.py> /path/to/input.obj

There are variants that output FCE3, FCE4, FCE4M, respectively.

## Damage model (FCE4, FCE4M)

    <partname>
    DAMAGE_<partname>

The damage model of a part in FCE4 and FCE4M is a second vertice position and
normal per vertice. Wavefront OBJ does not understand this concept of damage
models. However, it is still easily possible to handle this scenario with
_fcecodec_ and create damage models.

In your project in your 3D application of choice, once a part/shape is finished
(e.g. `:HB`), create a copy of that shape/part and name it `DAMAGE_:HB`

In this new part, it is absolutely crucial to change nothing except the vertice
positions.

The Obj2Fce4 and Obj2Fce4M scripts will automatically copy the vertice positions
and vertice normals from the `DAMAGE_:HB` part to the damage vertice positions
and damage vertice normals of the `:HB` part.

## Dummies (Light / FX objects)

    DUMMY_##_<dummyname>

To create an FCE dummy, in your project in your 3D application of choice, create
a part called `DUMMY_##_<dummyname>`, where each # is a digit 0-9.

The only requirement is that partnames are not identical, hence the digits.

Dummynames are documented in [_/src/fcelib/fcelib_fcetypes.h_](https://github.com/bfut/fcecodec/blob/main/src/fcelib/fcelib_fcetypes.h)

_Obj2Fce_ will from that part create a dummy called `<dummyname>`. The dummy
position is computed from the part's centroid.

Only the part name and its centroid are relevant. The part can be a ball, a
diamond or anything else.

## Triangle flags

This functionality is executed in the _material to triagflag_ scripts.

FCE triangle flags are positive integer values. The values are identical across
FCE3, FCE, and FCE4M, in sofar they are implemented. FCE generally allows
to set any value, even if it serves no purpose.

The triangle flag concept does not exist in Wavefront OBJ.

The _Obj2Fce_ converter scripts set triangle flags from tags in material names.

| Triangle flag         |           Tag |   Hex value |
|:----------------------|:--------------|------------:|
| default               |          FDEF |       0x000 |
| matte (no reflection) |          FMAT |       0x001 |
| high chrome           |          FHIC |       0x002 |
| no cull               |          FNOC |       0x004 |
| semi-transparent      |          FSET |       0x008 |
| ? (unknown 1)         |          FUN1 |       0x010 |
| all windows           |          FALW |       0x020 |
| front window          |          FFRW |       0x040 |
| left window           |          FLEW |       0x080 |
| back window           |          FBAW |       0x100 |
| right window          |          FRIW |       0x200 |
| broken window         |          FBRW |       0x400 |
| ? (unknown 2)         |          FUN2 |       0x800 |

Example: In FCE4, a non-broken window on the left side may have the triangle
flag `0x0AE`. With _Obj2Fce_ this flag can be set by assigning the triangle to a
material with a name that includes `_FHIC_FNOC_FSET_FALW_FLEW_` (i.e., high
chrome, no cull, semi-transparent, all windows, left window).

It is ok when the material name includes additional elements that are not tags.
The order of the tags is irrelevant.

## Texture page

This functionality is executed in the _material to texpage_ scripts.

Similar to the triangle flag, the triangles texture pages are positive integer
values. Similarly, the _Obj2Fce_ converter scripts looks for one tag in the
material names.

The texture page and triangle flag tags can be used alongside each other.

This concept is not used in car.fce or dash.fce

| Texture page |  Tag      |   Hex value |
|:-------------|:--------- |------------:|
|            0 |  (no tag) |       0x000 |
|            1 |  T1       |       0x001 |
|            2 |  T2       |       0x002 |
|            i |  Ti       |      hex(i) |

etc.

Example: To set texture page `0`, no tag needs to be added. Texture page `2`
is set by assigning the triangle to a material name that includes the tag `_T2_`

It is ok when the material name includes additional elements that are not tags.
The order of the tags is irrelevant.