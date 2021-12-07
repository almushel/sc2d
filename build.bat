@echo off
gcc src\main.c src\polygon.c src\collib2d.c -o ..\build\sat-2d.exe -I external\include -L external\lib -lraylib -lopengl32 -lgdi32 -lwinmm