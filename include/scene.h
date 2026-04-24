#pragma once

#include <vector>

#include "map.h"
#include "types.h"

class Scene
{
public:
    Scene();

    Player player;
    Map map_grid;
    std::vector<Texture> textures;
    std::vector<Entity> entities;

private:
    void generateStoneSlabTexture(Texture& texture, uint32_t slabColor, int width, int height);
    void generateBrickTexture(Texture& texture, uint32_t brickColor, int width, int height);
    void spawnTestSprites();
};
