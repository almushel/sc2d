cmake_minimum_required(VERSION 3.22.2)

project(sc2d_demo C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

set(sc2d_demo_RAYLIB_DIR "../../../raylib")

add_subdirectory(
	${sc2d_demo_RAYLIB_DIR} "../bin/raylib"
)

include_directories(
	PUBLIC "../"
	PUBLIC "${sc2d_demo_RAYLIB_DIR}/src"
)

add_executable(sc2d_demo
	sc2d_demo.c
)

target_link_libraries(sc2d_demo
	raylib opengl32 winmm
)