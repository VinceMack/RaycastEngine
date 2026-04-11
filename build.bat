@echo off
g++ src/raycaster.cpp src/engine.cpp src/input.cpp src/map.cpp src/renderer.cpp src/scene.cpp src/sdl_context.cpp ^
-o raycaster ^
-Iinclude -ISDL2/include ^
-LSDL2/lib ^
-lmingw32 -lSDL2main -lSDL2 -lSDL2_image -fopenmp