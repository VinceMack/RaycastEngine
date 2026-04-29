#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "types.h"

class AssetManager {
public:
    AssetManager();

    // Texture Management
    int loadTexture(const std::string& path);
    const Texture& getTexture(int index) const;

    // Weapon Registry
    void registerWeapon(WeaponID id, const WeaponDefinition& def);
    const WeaponDefinition* getWeaponDefinition(WeaponID id) const;

    // Procedural generation helpers (moved from Scene)
    int createStoneSlab(uint32_t color);
    int createBricks(uint32_t color);

private:
    std::vector<Texture> textures;
    std::unordered_map<WeaponID, WeaponDefinition> weaponRegistry;
};