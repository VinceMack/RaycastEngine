@echo off
g++ raycaster.cpp -o raycaster -ISDL2/include -LSDL2/lib -lmingw32 -lSDL2main -lSDL2