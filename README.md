# RaycastEngine

A simple C++/SDL2 software raycaster with OpenMP parallelized rendering.

## Project Layout

- `src/`:
  - `raycaster.cpp`: program entry point (`main`)
  - `engine.cpp`: main loop, update, event handling
  - `input.cpp`: action-based input bindings (WASD + Arrow keys)
  - `renderer.cpp`: ray casting and wall/floor/ceiling rendering
  - `scene.cpp`: player, map, and texture setup
  - `map.cpp`: flattened 1D map storage and lookup
  - `sdl_context.cpp`: SDL window/renderer/texture lifecycle
- `include/`: headers for all modules
- `SDL2/`: bundled SDL2 headers/libs used by `build.bat`

## Build

From the project root:

```bat
build.bat
```

This compiles all source files and outputs `raycaster.exe`.

## Run

```bat
raycaster.exe
```

## Controls

- Move forward: `W` or `Up Arrow`
- Move backward: `S` or `Down Arrow`
- Turn left: `A` or `Left Arrow`
- Turn right: `D` or `Right Arrow`