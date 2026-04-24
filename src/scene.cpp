#include "scene.h"

Scene::Scene(AssetManager& assetManager)
{
    spawnTestSprites(assetManager);
}

void Scene::spawnTestSprites(AssetManager& assetManager)
{
    entities.clear();

    // 1. A STATIC object (Standard Warning Sign)
    // Position, TextureIndex, Type, Scale, vOffset
    entities.emplace_back(Vector2{18.5, 9.5}, 8, EntityType::STATIC, 1.0, 1.0);

    // 2. An ITEM (Bouncing/Bobbing Sign)
    // We set scale to 0.5 and vOffset to 0.8 so it hovers slightly
    const WeaponDefinition* pistol = assetManager.getWeaponDefinition(WeaponID::PISTOL);
    if (pistol)
    {
        entities.emplace_back(Vector2{15.5, 15.5}, pistol->worldTextureIndex, EntityType::ITEM, 0.5, 0.8);
        entities.back().weaponInside = WeaponID::PISTOL;
    }

    const WeaponDefinition* smg = assetManager.getWeaponDefinition(WeaponID::SMG);
    if (smg)
    {
        entities.emplace_back(Vector2{10.5, 10.5}, smg->worldTextureIndex, EntityType::ITEM, 0.5, 0.8);
        entities.back().weaponInside = WeaponID::SMG;
    }

    // 3. An ENEMY (The Chaser)
    // This one will move toward you once the updateEnemyAI logic is running
    entities.emplace_back(Vector2{10.5, 10.5}, 7, EntityType::ENEMY, 1.0, 1.0);
}

