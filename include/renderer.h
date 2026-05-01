#pragma once

#include <vector>

#include "constants.h"
#include "asset_manager.h"
#include "scene.h"
#include "sdl_context.h"

class Renderer
{
public:
    Renderer(SDLContext& sdl, AssetManager& assetManager);

    void render(const Scene& scene);
    const std::vector<double>& getDepthBuffer() const;
    RayHit castRay(int x, const Player& player, const Map& map_grid);

private:
    SDLContext& sdl;
    AssetManager& assetManager;
    std::vector<double> depthBuffer;

    int wrapPow2(int value, int size) const;
    void renderFloorAndCeiling(const Scene& scene);
    void renderWalls(const Scene& scene);
    void renderEntities(const Scene& scene);
    void renderWeapon(const Scene& scene);
};
