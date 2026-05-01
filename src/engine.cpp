#include "engine.h"

#include <string>
#include <cmath>
#include <algorithm>
#include "types.h"
#include <queue>
#include <set>
#include <deque>

Engine::Engine() : sdl(screen_width, screen_height), assetManager(), scene(assetManager), renderer(sdl, assetManager), input()
{
}

void Engine::run()
{
    uint64_t lastCounter = SDL_GetPerformanceCounter();
    double freq = (double)SDL_GetPerformanceFrequency();

    SDL_Event event;
    while (sdl.running)
    {
        uint64_t currentCounter = SDL_GetPerformanceCounter();
        double deltaTime = (double)(currentCounter - lastCounter) / freq;
        lastCounter = currentCounter;

        updateWindowTitle(deltaTime);
        processEvents(event);
        update(deltaTime);
        renderer.render(scene);
    }
}

void Engine::processEvents(SDL_Event& event)
{
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            sdl.running = false;
        }
    }
}

void Engine::fireWeapon()
{
    if (!scene.player.currentWeapon) return;

    // 1. Find the distance to the wall in the center of the screen
    RayHit wallHit = renderer.castRay(screen_width / 2, scene.player, scene.map_grid);
    double closestDist = wallHit.distance;
    Entity* targetedEntity = nullptr;

    // 2. Check entities
    for (auto& e : scene.entities) {
        if (e.type != EntityType::ENEMY || e.isDead) continue;

        // Use the same math as the renderer to find the sprite's screen position
        double spriteX = e.position.x - scene.player.position.x;
        double spriteY = e.position.y - scene.player.position.y;
        double invDet = 1.0 / (scene.player.plane.x * scene.player.direction.y - scene.player.direction.x * scene.player.plane.y);
        double transformX = invDet * (scene.player.direction.y * spriteX - scene.player.direction.x * spriteY);
        double transformY = invDet * (-scene.player.plane.y * spriteX + scene.player.plane.x * spriteY);

        if (transformY <= 0) continue; // Behind player

        int spriteScreenX = int((screen_width / 2) * (1 + transformX / transformY));
        int spriteHeight = std::abs(int(screen_height / transformY * e.scale));
        const Texture& tex = assetManager.getTexture(e.textureIndex);
        int spriteWidth = std::abs(int(spriteHeight * ((double)tex.width / tex.height)));

        // 3. Is the center of the screen (screen_width/2) inside the sprite's width?
        int leftEdge = spriteScreenX - spriteWidth / 2;
        int rightEdge = spriteScreenX + spriteWidth / 2;

        if (screen_width / 2 >= leftEdge && screen_width / 2 <= rightEdge) {
            // Check if this entity is closer than the wall and closer than any other previously checked entity
            if (transformY < closestDist) {
                closestDist = transformY;
                targetedEntity = &e;
            }
        }
    }

    // 4. Apply damage
    if (targetedEntity) {
        targetedEntity->takeDamage(scene.player.currentWeapon->damage);
    }
}

void Engine::update(double deltaTime)
{
    input.handlePlayerInput(deltaTime, scene.player, scene.map_grid);
    updateEntities(deltaTime);

    Player& p = scene.player;
    if (!p.currentWeapon) return;

    // 1. Always decrement the cooldown
    if (p.weaponCooldown > 0) p.weaponCooldown -= (float)deltaTime;

    // 2. Determine if we are pulling the trigger
    bool isFiring = input.isActionPressed("Fire");
    bool canFire = (p.weaponCooldown <= 0);

    if (isFiring && canFire)
    {
        bool triggerPulledThisFrame = !wasFirePressedLastFrame;
        
        // 3. Logic Fork:
        // If Automatic: Fire as long as trigger is held
        // If Semi-Auto: Fire only if trigger was just pulled this frame
        if (p.currentWeapon->isAutomatic || triggerPulledThisFrame)
        {
            fireWeapon();
            // Reset cooldown based on weapon definition
            p.weaponCooldown = p.currentWeapon->fireRate;
        }
    }

    // 4. Update the "Last Frame" state
    wasFirePressedLastFrame = isFiring;
}

void Engine::updateWindowTitle(double deltaTime)
{
    double msPerFrame = deltaTime * 1000.0;
    SDL_SetWindowTitle(sdl.window, ("Raycaster - Frame Time: " + std::to_string(msPerFrame) + "ms").c_str());
}

std::vector<Vector2> Engine::calculateAStarPath(Vector2 start, Vector2 target) {
    int startX = (int)start.x, startY = (int)start.y;
    int targetX = (int)target.x, targetY = (int)target.y;

    // Boundary check and trivial case
    if (startX == targetX && startY == targetY) return {};
    if (scene.map_grid.at(targetX, targetY) != 0) return {};

    // Comparison for priority queue: lowest F-cost first
    auto cmp = [](Node* a, Node* b) { return a->fCost() > b->fCost(); };
    std::priority_queue<Node*, std::vector<Node*>, decltype(cmp)> openList(cmp);
    
    // Heuristic: Octile distance (math for 8-way grid movement)
    auto getHeuristic = [&](int x, int y) {
        int dx = std::abs(x - targetX);
        int dy = std::abs(y - targetY);
        return (dx > dy) ? (dx - dy + 1.414 * dy) : (dy - dx + 1.414 * dx);
    };

    bool closedList[map_size][map_size] = {false};
    std::deque<Node> nodePool; // Automatic memory management with stable pointers

    auto addNode = [&](int x, int y, double g, Node* p) {
        nodePool.emplace_back(Node{x, y, g, getHeuristic(x, y), p});
        openList.push(&nodePool.back());
    };

    addNode(startX, startY, 0, nullptr);

    while (!openList.empty()) {
        Node* current = openList.top();
        openList.pop();

        // Path found!
        if (current->x == targetX && current->y == targetY) {
            std::vector<Vector2> path;
            while (current->parent != nullptr) {
                path.push_back({(double)current->x, (double)current->y});
                current = current->parent;
            }
            return path; // Reconstructed Target -> Start
        }

        if (closedList[current->x][current->y]) continue;
        closedList[current->x][current->y] = true;

        // Check 8 Neighbors
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx == 0 && dy == 0) continue;

                int nx = current->x + dx;
                int ny = current->y + dy;

                // Basic grid and wall check
                if (nx >= 0 && nx < map_size && ny >= 0 && ny < map_size &&
                    scene.map_grid.at(nx, ny) == 0 && !closedList[nx][ny]) {
                    
                    // --- CORNER CUTTING PREVENTION ---
                    if (std::abs(dx) + std::abs(dy) == 2) {
                        if (scene.map_grid.at(current->x + dx, current->y) != 0 || 
                            scene.map_grid.at(current->x, current->y + dy) != 0) {
                            continue; 
                        }
                    }

                    double stepCost = (std::abs(dx) + std::abs(dy) == 2) ? 1.414 : 1.0;
                    addNode(nx, ny, current->gCost + stepCost, current);
                }
            }
        }
    }

    return {};
}

void Engine::updateSmartAI(Entity& e, double deltaTime, int& pathsCalculated)
{
    e.pathTimer -= deltaTime;
    
    // 1. Path Planning (Queueing effect: limit A* calculations per frame)
    if ((e.pathTimer <= 0 || e.currentPath.empty()) && pathsCalculated < 2) {
        e.currentPath = calculateAStarPath(e.position, scene.player.position);
        e.pathTimer = 0.5f; 
        pathsCalculated++;
    }

    // 2. Path Following
    if (!e.currentPath.empty()) {
        Vector2 target = e.currentPath.back(); // Target the next cell
        // Add 0.5 to target the center of the cell
        Vector2 targetPos = {target.x + 0.5, target.y + 0.5}; 
        
        Vector2 dir = targetPos - e.position;
        double dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);

        if (dist > 0.1) {
            dir = dir * (1.0 / dist);
            e.position += dir * 2.0 * deltaTime;
        } else {
            // Reached the cell, move to the next one
            e.currentPath.pop_back();
        }
    }
}

void Engine::updateEnemyAI(Entity& e, double deltaTime, int& pathsCalculated)
{
    if (e.behavior == AIBehavior::DIRECT) {
        // --- YOUR CURRENT IMPLEMENTATION ---
        Vector2 dirToPlayer = scene.player.position - e.position;
        double distance = std::sqrt(dirToPlayer.x * dirToPlayer.x + dirToPlayer.y * dirToPlayer.y);
        if (distance < 10.0 && distance > 0.5) {
            dirToPlayer = dirToPlayer * (1.0 / distance); // Normalize
            e.position += dirToPlayer * 2.0 * deltaTime;
        }
    } 
    else if (e.behavior == AIBehavior::SMART) {
        updateSmartAI(e, deltaTime, pathsCalculated);
    }
}

void Engine::updateEntities(double deltaTime)
{
    int pathsCalculated = 0;

    // 1. Process Logic and Mark for Deletion
    for (auto& e : scene.entities)
    {
        e.totalTime += deltaTime;

        if (e.damageTimer > 0) {
            e.damageTimer -= (float)deltaTime;
        }

        // Pickup Logic
        if (e.type == EntityType::ITEM) 
        {
            Vector2 distVec = e.position - scene.player.position;
            double distSq = distVec.x * distVec.x + distVec.y * distVec.y;

            if (distSq < 0.25) // 0.5 units distance pickup radius
            { 
                if (e.weaponInside != WeaponID::NONE)
                {
                    scene.player.currentWeapon = assetManager.getWeaponDefinition(e.weaponInside);
                }
                e.dist = -1.0; // Marker: mark for deletion
                continue;      // Skip behavioral logic for this frame
            }
        }

        // Behavior Logic
        switch (e.type)
        {
            case EntityType::ITEM:
                e.vOffset = 0.8 + std::sin(e.totalTime * 3.0) * 0.1;
                break;

            case EntityType::ENEMY:
                updateEnemyAI(e, deltaTime, pathsCalculated); 
                break;

            default:
                break;
        }
    }

    // 2. Physical Deletion (Erase-Remove Idiom)
    // This moves all "marked" entities to the end and shrinks the vector.
    scene.entities.erase(
        std::remove_if(scene.entities.begin(), scene.entities.end(),
            [](const Entity& e) { return e.dist < 0.0; }),
        scene.entities.end()
    );
}