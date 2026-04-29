#include "asset_manager.h"
#include <SDL2/SDL_image.h>
#include <stdexcept>
#include <utility>

namespace
{
    void generateStoneSlabTexture(Texture& texture, uint32_t slabColor, int width, int height)
    {
        texture.resize(width, height);
        const uint32_t groutColor = 0xFF1A1A1A;

        for (int x = 0; x < texture.width; x++)
        {
            for (int y = 0; y < texture.height; y++)
            {
                bool isGrout = (x < 2 || x >= texture.width - 2 || y < 2 || y >= texture.height - 2);
                texture.at(x, y) = isGrout ? groutColor : slabColor;
            }
        }
    }

    void generateBrickTexture(Texture& texture, uint32_t brickColor, int width, int height)
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
}

int AssetManager::loadTexture(const std::string& path) {
    Texture tex;
    tex.loadFromFile(path); // Uses the logic you already implemented
    textures.push_back(std::move(tex));
    return (int)textures.size() - 1;
}

const Texture& AssetManager::getTexture(int index) const {
    return textures.at(index);
}

void AssetManager::registerWeapon(WeaponID id, const WeaponDefinition& def) {
    weaponRegistry[id] = def;
}

const WeaponDefinition* AssetManager::getWeaponDefinition(WeaponID id) const {
    auto it = weaponRegistry.find(id);
    if (it != weaponRegistry.end()) return &it->second;
    return nullptr;
}

AssetManager::AssetManager()
{
    const uint32_t colors[8] = {
        0xFFFF0000, 0xFF00FF00, 0xFF0000FF, 0xFFFFFF00,
        0xFFFF00FF, 0xFF00FFFF, 0xFFFFFFFF, 0xFF777777
    };

    textures.resize(8);

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

    textures[7].loadFromFile("assets/slime_person.png");

    textures.push_back(Texture());
    textures.back().loadFromFile("assets/sign.png");

    textures.push_back(Texture());
    textures.back().loadFromFile("assets/pistol_world.png");

    textures.push_back(Texture());
    textures.back().loadFromFile("assets/save_disk.png");

    textures.push_back(Texture());
    textures.back().loadFromFile("assets/pistol_held.png");

    textures.push_back(Texture());
    textures.back().loadFromFile("assets/smg_world.png");

    textures.push_back(Texture());
    textures.back().loadFromFile("assets/smg_held.png");

    registerWeapon(WeaponID::PISTOL, WeaponDefinition{
        WeaponID::PISTOL,
        "Pistol",
        9,
        11
    });

    registerWeapon(WeaponID::SMG, WeaponDefinition{
        WeaponID::SMG,
        "SMG",
        12,
        13
    });
}

int AssetManager::createStoneSlab(uint32_t color)
{
    Texture texture;
    generateStoneSlabTexture(texture, color, 64, 64);
    textures.push_back(std::move(texture));
    return static_cast<int>(textures.size()) - 1;
}

int AssetManager::createBricks(uint32_t color)
{
    Texture texture;
    generateBrickTexture(texture, color, 64, 64);
    textures.push_back(std::move(texture));
    return static_cast<int>(textures.size()) - 1;
}