@echo off
gcc demo\main.c demo\polygon.c src\collib2d.c -o demo\bin\collib2d-demo.exe -I external\include -I src -L external\lib -lraylib -lopengl32 -lgdi32 -lwinmm