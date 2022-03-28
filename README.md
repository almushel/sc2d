# Simple Collision 2D
*A single header 2D collision library written in C.*

The goal with this library is to create a straightforward library that checks for collisions and calculates overlaps for a wide variety of 2 dimensional shapes and lines (and doesn't try to do anything else). It should be clear from the code and the comments what each function is doing and why and for that reason optimization is not really a goal here. 

This library also avoids defining its own types for things like `Vector2` and `Polygon` to (hopefully) avoid conflicts and overlaps with various approaches in other libraries. I am not confident that this is a great idea.

## Using the Library

`SIMPLE_COLLISION_2D_IMPLEMENTATION` should be defined in **one** C/C++ source file before including this header. It can then be included in any other source file that references its functions like a standard header.

The `sc2d` functions all follow a similar structure and all return `true` if a collision has occurred.

```c
bool sc2d_check_shape1_shape2(
	[in] float x1, 
	[in] float y1,
	[in] float shape1_dimensions ... ,
	[in] float x2,
	[in] float y2,
	[in] float shape2_dimensions ... ,
	[out] float *overlap_x,
	[out] float *overlap_y
)
```

| Argument | Description |
|-------------------|-------------------------------------------------------|
| x1 				| X position of first shape 							|
| y1 				| Y position of first shape 							|
| <br>width, height<br>radius<br>*vertices, vert_count | **Dimensions of first shape:**<br>Rectangle<br>Circle<br>Polygon\* |
| x2 				| X position of second shape 							|
| y2 				| Y position of second shape 							|
| <br>width, height<br>radius<br>*vertices, vert_count | **Dimensions of second shape:**<br>Rectangle<br>Circle<br>Polygon\* |
| overlap_x 		| returns x value of shortest overlap (or intersection) |
| overlap_y 		| returns y value of shortest overlap (or intersection) |

\*Because `sc2d` does not defintion a 2D vector type, polygon vertices are passed as `float` array, and the `vert_count` refers to the total number of float elements **not** the number of x,y pairs (this may not be a good idea). For example, when passing an array of `struct {float x, float y}`, the vert_count should be multiplied by 2.

## Building the Demo

The included demo program currently uses [raylib](https://github.com/raysan5/raylib) version 4.0.0 or later, but doesn't make extensive use of its features. Set `sc2d_RAYLIB_DIR` in `src/demo/CMakeLists.txt` to the raylib source directory on your machine.

Also included is my current preferred build configuration for Windows.

```
config.bat
build.bat
run.bat
```

This assumes that you have MinGW and [Ninja](https://github.com/ninja-build/ninja) installed.