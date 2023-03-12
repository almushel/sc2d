# Simple Collision 2D
*A single-file 2D collision library written in C.*

The goal with this library is to create a straightforward library that checks for collisions and calculates overlaps for a wide variety of 2 dimensional shapes and lines (and doesn't try to do anything else). It should be clear from the code and the comments what each function is doing. There are no dependencies aside from `math.h`.

## Using the Library

`SIMPLE_COLLISION_2D_IMPLEMENTATION` should be defined in **one** C/C++ source file before including this header. It can then be included in any other source file that references its functions like a standard header.

The `sc2d` functions all follow a similar structure and all return `true` if a collision has occurred.

```c
bool sc2d_check_shape1_shape2(
	[in]  sc2d_v2 p1, 
	[in]  float shape1_dimensions ... ,
	[in]  sc2d_v2 p2,
	[in]  float shape2_dimensions ... ,
	[out] sc2d_v2 *overlap,
)
```

| Argument | Description |
|-------------------|-------------------------------------------------------|
| p1 				| Position of first shape 							|
| <br>width, height<br>radius<br>*vertices, vert_count | **Dimensions of first shape:**<br>Rectangle<br>Circle<br>Polygon |
| p2 				| Position of second shape 							|
| <br>width, height<br>radius<br>*vertices, vert_count | **Dimensions of second shape:**<br>Rectangle<br>Circle<br>Polygon |
| overlap 			| Value of shortest overlap (or intersection)** |


### Simple Resolution

The `overlap` vector is always relative to the first shape in the argument list. Therefore, for static resolution it should be **subtracted** from the position of `shape1` and/or **added** to position `shape2`.

```c check_circles
if (sc2d_check_circles(p1, r1, p2, r2, &overlap)) {
	p1.x -= overlap.x;
	p2.y -= overlap.y
}
```

## Building the Demo

The included demo program currently uses [raylib](https://github.com/raysan5/raylib) version 4.0.0 or later, but doesn't make extensive use of its features. Set `sc2d_RAYLIB_DIR` in `src/demo/CMakeLists.txt` to the raylib source directory on your machine.

Also included is a build/run configuration for Windows.

```
config.bat
build.bat
run.bat
```

This assumes that you have MinGW and [Ninja](https://github.com/ninja-build/ninja) installed.
