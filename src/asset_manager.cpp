#include "asset_manager.h"
#include <SDL2/SDL_image.h>
#include <stdexcept>
#include <cmath>

AssetManager::AssetManager()
{
    // Wall Textures
    createBricks("wall_red",     0xFFFF0000); 
    createBricks("wall_green",   0xFF00FF00); 
    createBricks("wall_blue",    0xFF0000FF); 
    createStoneSlab("floor",     0xFFFFFF00); 
    createBricks("wall_purple",  0xFFFF00FF); 
    createBricks("wall_cyan",    0xFF00FFFF); 
    createStoneSlab("ceiling",   0xFFFFFFFF); 
    
    // Entity Textures
    loadTexture("slime",        "assets/slime.png");
    loadTexture("sign",         "assets/sign.png");
    loadTexture("save_disk",    "assets/save_disk.png");
    loadTexture("ghost",        "assets/ghost.png");
    
    // Weapon Textures
    int pWorld = loadTexture("pistol_world", "assets/pistol_world.png");
    int pHeld  = loadTexture("pistol_held",  "assets/pistol_held.png");
    int sWorld = loadTexture("smg_world",    "assets/smg_world.png");
    int sHeld  = loadTexture("smg_held",     "assets/smg_held.png");

    // Register Weapons
    registerWeapon(WeaponID::PISTOL, { WeaponID::PISTOL, "Pistol", pWorld, pHeld });
    registerWeapon(WeaponID::SMG,    { WeaponID::SMG,    "SMG",    sWorld, sHeld });
}

int AssetManager::loadTexture(const std::string& name, const std::string& path) {
    Texture tex;
    tex.loadFromFile(path);
    textures.push_back(std::move(tex));
    int index = (int)textures.size() - 1;
    nameToIndex[name] = index;
    return index;
}

int AssetManager::getTextureIndex(const std::string& name) const {
    auto it = nameToIndex.find(name);
    if (it != nameToIndex.end()) return it->second;
    return -1; // Not found
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

int AssetManager::createStoneSlab(const std::string& name, uint32_t slabColor) {
    Texture texture(64, 64);
    const uint32_t groutColor = 0xFF1A1A1A;
    for (int x = 0; x < 64; x++) {
        for (int y = 0; y < 64; y++) {
            bool isGrout = (x < 2 || x >= 62 || y < 2 || y >= 62);
            texture.at(x, y) = isGrout ? groutColor : slabColor;
        }
    }
    textures.push_back(std::move(texture));
    int index = (int)textures.size() - 1;
    nameToIndex[name] = index;
    return index;
}

int AssetManager::createBricks(const std::string& name, uint32_t brickColor) {
    Texture texture(64, 64);
    for (int x = 0; x < 64; x++) {
        for (int y = 0; y < 64; y++) {
            bool edge = (y % 16 == 0) || ((x + (y / 16) * 16) % 32 == 0);
            texture.at(x, y) = edge ? 0xFF000000 : brickColor;
        }
    }
    textures.push_back(std::move(texture));
    int index = (int)textures.size() - 1;
    nameToIndex[name] = index;
    return index;
}