#pragma once

#include <vector>

#include "asset_manager.h"
#include "map.h"
#include "types.h"

class Scene
{
public:
    explicit Scene(AssetManager& assetManager);

    Player player;
    Map map_grid;
    std::vector<Entity> entities;

private:
    void spawnTestSprites(AssetManager& assetManager);
};
