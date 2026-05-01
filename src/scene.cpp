#include "scene.h"

Scene::Scene(AssetManager& assetManager)
{
    spawnTestSprites(assetManager);
}

void Scene::spawnTestSprites(AssetManager& assetManager)
{
    entities.clear();

    // 1. A STATIC object (Standard Warning Sign)
    // We look up the index by the name "sign" defined in AssetManager
    int signTex = assetManager.getTextureIndex("sign");
    entities.emplace_back(Vector2{18.5, 9.5}, signTex, EntityType::STATIC, AIBehavior::NONE, 1.0, 1.0);

    // 2. An ITEM (Pistol Pickup)
    const WeaponDefinition* pistol = assetManager.getWeaponDefinition(WeaponID::PISTOL);
    if (pistol)
    {
        // The definition already knows its world texture index!
        entities.emplace_back(Vector2{15.5, 15.5}, pistol->worldTextureIndex, EntityType::ITEM, AIBehavior::NONE, 0.5, 0.8);
        entities.back().weaponInside = WeaponID::PISTOL;
    }

    // 3. An ITEM (SMG Pickup)
    const WeaponDefinition* smg = assetManager.getWeaponDefinition(WeaponID::SMG);
    if (smg)
    {
        entities.emplace_back(Vector2{10.5, 10.5}, smg->worldTextureIndex, EntityType::ITEM, AIBehavior::NONE, 0.5, 0.8);
        entities.back().weaponInside = WeaponID::SMG;
    }

    // 4. A SMART ENEMY (Respects walls)
    // We look up the index by the name "slime"
    int slimeTex = assetManager.getTextureIndex("slime");
    entities.emplace_back(Vector2{10.5, 12.5}, slimeTex, EntityType::ENEMY, AIBehavior::SMART, 1.0, 1.0);

    // 5. A GHOST ENEMY (Phases through walls)
    int ghostTex = assetManager.getTextureIndex("ghost");
    entities.emplace_back(Vector2{10.5, 12.5}, ghostTex, EntityType::ENEMY, AIBehavior::DIRECT, 1.0, 1.0);
}
