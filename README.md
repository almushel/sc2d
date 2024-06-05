# Simple Collision 2D
*A single-file 2D collision library written in C.*

The goal with this library is to create a straightforward library that checks for collisions and calculates overlaps for a wide variety of 2 dimensional shapes and lines (and doesn't try to do anything else).
It should be clear from the code and the comments what each function is doing.

## Using the Library

`SIMPLE_COLLISION_2D_IMPLEMENTATION` should be defined in **one** C/C++ source file before including this header. It can then be included in any other source file that references its functions like a standard header.

The `sc2d` functions all follow a similar structure and all return `true` if a collision has occurred.

```c
bool sc2d_check_shape1_shape2(
	[in]  float p1x, 
	[in]  float p1y, 
	[in]  float shape1_dimensions ... ,
	[in]  float p2x,
	[in]  float p2y,
	[in]  float shape2_dimensions ... ,
	[out] float *overlap_x,
	[out] float *overlap_y,
)
```

| Argument 												| Description 														|
|-------------------------------------------------------|-------------------------------------------------------------------|
| p1x, p1y 												| Position of first shape 								 			|
| <br>width, height<br>radius<br>*vertices, vert_count 	| **Dimensions of first shape:**<br>Rectangle<br>Circle<br>Polygon 	|
| p2x, p2y 												| Position of second shape 								 			|
| <br>width, height<br>radius<br>*vertices, vert_count 	| **Dimensions of second shape:**<br>Rectangle<br>Circle<br>Polygon	|
| overlap_x, overlap_y 									| Value of shortest overlap (or intersection)**  					|

## Dependencies
There are no dependencies aside from `math.h` by default.
To eliminate this, define your own replacements before including the implementation code.

```c
#define sc2d_hypotf hypotf
#define sc2d_fabsf fabsf
#define sc2d_min fminf
#define sc2d_max fmaxf
#define sc2d_atan2 atan2

#define SIMPLE_COLLISION_2D_IMPLEMENTATION
#include "sc2d.h"
```

## Static Collision Resolution

The `overlap` vector is always relative to the first shape in the argument list. Therefore, for static resolution it should be **subtracted** from the position of `shape1` and/or **added** to position `shape2` to push the shapes apart.

```c check_circles Resolution
if (sc2d_check_circles(p1.x, p1.y, r1, p2.x, p2.y, r2, &overlap.x, &overlap.y)) {
	p1.x -= overlap.x / 2.0f;
	p1.y -= overlap.y / 2.0f;

	p2.x += overlap.x / 2.0f;
	p2.y += overlap.y / 2.0f;
}
```

## Custom Vector2

By default, the polygon collision functions `sc2d_check_poly2d` and `sc2d_check_point_poly2d` assume vertices are defined as `float` x/y pairs. To use these functions with a different type, you can define a custom `sc2d_v2` struct type with members `x` and `y` before the implementation as follows:

```c Custom Vector Definition
#ifndef SIMPLE_COLLISION_2D_VECTOR2
typedef Vector3 sc2d_v2;
#define SIMPLE_COLLISION_2D_VECTOR2
#endif

#define SIMPLE_COLLISION_2D_IMPLEMENTATION
#include "sc2d.h"
```

The demo project uses `Vector3` in this way (z is ignored) to demonstrate this. This may or may not be a good idea.

## Building the Demo

The included demo program currently uses [raylib](https://github.com/raysan5/raylib) version 4.0.0 or later, but doesn't make extensive use of its features. Set `sc2d_RAYLIB_DIR` in `src/demo/CMakeLists.txt` to a raylib source directory on your machine.

Also included is a build/run configuration for Windows.

```
config.bat
build.bat
run.bat
```

This assumes that you have MinGW and [Ninja](https://github.com/ninja-build/ninja) installed.
