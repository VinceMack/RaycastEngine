#include "scene.h"

Scene::Scene() : textures(8)
{
    uint32_t colors[8] = { 0xFFFF0000, 0xFF00FF00, 0xFF0000FF, 0xFFFFFF00,
                           0xFFFF00FF, 0xFF00FFFF, 0xFFFFFFFF, 0xFF777777 };

    for (int i = 0; i < 8; i++)
    {
        if (i == 3 || i == 6)
        {
            generateStoneSlabTexture(textures[i], colors[i], 64, 64);
        }
        else
        {
            generateBrickTexture(textures[i], colors[i], 64, 64);
        }
    }

    generateBrickTexture(textures[7], colors[7], 128, 128);
}

void Scene::generateStoneSlabTexture(Texture& texture, uint32_t slabColor, int width, int height)
{
    texture.resize(width, height);
    uint32_t groutColor = 0xFF1A1A1A;

    for (int x = 0; x < texture.width; x++)
    {
        for (int y = 0; y < texture.height; y++)
        {
            bool isGrout = (x < 2 || x >= texture.width - 2 || y < 2 || y >= texture.height - 2);
            texture.at(x, y) = isGrout ? groutColor : slabColor;
        }
    }
}

void Scene::generateBrickTexture(Texture& texture, uint32_t brickColor, int width, int height)
{
    texture.resize(width, height);

    for (int x = 0; x < texture.width; x++)
    {
        for (int y = 0; y < texture.height; y++)
        {
            bool edge = (y % 16 == 0) || ((x + (y / 16) * 16) % 32 == 0);
            texture.at(x, y) = edge ? 0xFF000000 : brickColor;
        }
    }
}
