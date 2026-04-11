#pragma once

#include <vector>

#include "constants.h"
#include "scene.h"
#include "sdl_context.h"

class Renderer
{
public:
    explicit Renderer(SDLContext& sdl);

    void render(const Scene& scene);
    const std::vector<double>& getDepthBuffer() const;

private:
    SDLContext& sdl;
    std::vector<double> depthBuffer;

    int wrapPow2(int value, int size) const;
    RayHit castRay(int x, const Player& player, const Map& map_grid);
    void renderFloorAndCeiling(const Scene& scene);
    void renderWalls(const Scene& scene);
};
