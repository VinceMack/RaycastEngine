#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "types.h"

class AssetManager {
public:
    AssetManager();

    // The primary way to get a texture
    int loadTexture(const std::string& name, const std::string& path);
    int getTextureIndex(const std::string& name) const;
    const Texture& getTexture(int index) const;

    // Procedural helpers
    int createStoneSlab(const std::string& name, uint32_t color);
    int createBricks(const std::string& name, uint32_t color);

    // Weapon Management
    void registerWeapon(WeaponID id, const WeaponDefinition& def);
    const WeaponDefinition* getWeaponDefinition(WeaponID id) const;

private:
    std::vector<Texture> textures;
    std::unordered_map<std::string, int> nameToIndex; // Maps "pistol_held" -> 11
    std::unordered_map<WeaponID, WeaponDefinition> weaponRegistry;
};