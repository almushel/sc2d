cmake_minimum_required(VERSION 3.22.1)

project(sc2d_demo C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

add_executable(sc2d_demo
	sc2d_demo.c
)

if (WIN32)
	target_link_libraries(sc2d_demo
		raylib opengl32 winmm
	)
else()
	target_link_libraries(sc2d_demo
		raylib GL m pthread dl rt X11
	)
endif()
