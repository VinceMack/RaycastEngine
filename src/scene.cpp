#include "scene.h"

Scene::Scene(AssetManager& assetManager)
{
    spawnTestSprites(assetManager);
}

void Scene::spawnTestSprites(AssetManager& assetManager)
{
    entities.clear();

    int signTex = assetManager.getTextureIndex("sign");
    entities.emplace_back(Vector2{18.5, 9.5}, signTex, EntityType::STATIC, AIBehavior::NONE, 1.0, 1.0);

    const WeaponDefinition* pistol = assetManager.getWeaponDefinition(WeaponID::PISTOL);
    if (pistol)
    {
        entities.emplace_back(Vector2{15.5, 15.5}, pistol->worldTextureIndex, EntityType::ITEM, AIBehavior::NONE, 0.5, 0.8);
        entities.back().weaponInside = WeaponID::PISTOL;
    }

    const WeaponDefinition* smg = assetManager.getWeaponDefinition(WeaponID::SMG);
    if (smg)
    {
        entities.emplace_back(Vector2{10.5, 10.5}, smg->worldTextureIndex, EntityType::ITEM, AIBehavior::NONE, 0.5, 0.8);
        entities.back().weaponInside = WeaponID::SMG;
    }

    int slimeTex = assetManager.getTextureIndex("slime");
    int slimeDamaged = assetManager.getTextureIndex("slime_damaged");
    entities.emplace_back(Vector2{10.5, 12.5}, slimeTex, EntityType::ENEMY, AIBehavior::SMART, 1.0, 1.0);
    entities.back().damagedTextureIndex = slimeDamaged;

    int ghostTex = assetManager.getTextureIndex("ghost");
    int ghostDamaged = assetManager.getTextureIndex("ghost_damaged");
    entities.emplace_back(Vector2{12.5, 12.5}, ghostTex, EntityType::ENEMY, AIBehavior::DIRECT, 1.0, 1.0);
    entities.back().damagedTextureIndex = ghostDamaged;
}
