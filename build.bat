@echo off

IF NOT EXIST "demo\bin" mkdir "demo\bin"

gcc src\demo.c -o bin\sc2d-demo.exe -I src\external\include -I src -L src\external\lib -lraylib -lopengl32 -lgdi32 -lwinmm