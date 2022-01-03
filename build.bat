@echo off

IF NOT EXIST "demo\bin" mkdir "demo\bin"

gcc demo\main.c demo\polygon.c src\collib2d.c -o demo\bin\collib2d-demo.exe -I demo\external\include -I src -L demo\external\lib -lraylib -lopengl32 -lgdi32 -lwinmm