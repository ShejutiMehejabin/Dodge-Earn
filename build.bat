@echo off

g++ main.cpp -o app.exe ^
-I ./raylib/ ^
-L ./raylib/ -lraylib -lgdi32 -lwinmm