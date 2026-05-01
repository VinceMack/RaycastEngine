#include "engine.h"

#include <string>
#include <cmath>
#include <algorithm> // Required for std::remove_if
#include "types.h"
#include <queue>
#include <set>

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

void Engine::update(double deltaTime)
{
    input.handlePlayerInput(deltaTime, scene.player, scene.map_grid);
    updateEntities(deltaTime);
}

void Engine::updateWindowTitle(double deltaTime)
{
    double msPerFrame = deltaTime * 1000.0;
    SDL_SetWindowTitle(sdl.window, ("Raycaster - Frame Time: " + std::to_string(msPerFrame) + "ms").c_str());
}

std::vector<Vector2> Engine::calculateAStarPath(Vector2 start, Vector2 target)
{
    int startX = (int)start.x, startY = (int)start.y;
    int targetX = (int)target.x, targetY = (int)target.y;

    // If start and target are the same, return empty
    if (startX == targetX && startY == targetY) return {};

    struct ANode {
        int x, y;
        double g, h;
        ANode* parent;
        double f() const { return g + h; }
    };

    // Priority queue to get node with lowest F cost
    auto cmp = [](ANode* a, ANode* b) { return a->f() > b->f(); };
    std::priority_queue<ANode*, std::vector<ANode*>, decltype(cmp)> openList(cmp);
    
    // Track visited nodes to avoid infinite loops
    bool closedList[map_size][map_size] = {false};
    std::vector<ANode*> allNodes; // For memory cleanup

    auto addNode = [&](int x, int y, double g, ANode* p) {
        ANode* n = new ANode{x, y, g, std::abs(x - targetX) + (double)std::abs(y - targetY), p};
        allNodes.push_back(n);
        openList.push(n);
    };

    addNode(startX, startY, 0, nullptr);

    while (!openList.empty()) {
        ANode* current = openList.top();
        openList.pop();

        if (current->x == targetX && current->y == targetY) {
            // Path found! Reconstruct it
            std::vector<Vector2> path;
            while (current->parent != nullptr) {
                path.push_back({(double)current->x, (double)current->y});
                current = current->parent;
            }
            // Cleanup memory
            for (auto n : allNodes) delete n;
            return path; // Note: Path is Target -> Start for easy pop_back()
        }

        if (closedList[current->x][current->y]) continue;
        closedList[current->x][current->y] = true;

        // Check 4 Neighbors (Up, Down, Left, Right)
        int dx[] = {0, 0, 1, -1};
        int dy[] = {1, -1, 0, 0};

        for (int i = 0; i < 4; i++) {
            int nx = current->x + dx[i];
            int ny = current->y + dy[i];

            if (nx >= 0 && nx < map_size && ny >= 0 && ny < map_size &&
                scene.map_grid.at(nx, ny) == 0 && !closedList[nx][ny]) {
                addNode(nx, ny, current->g + 1, current);
            }
        }
    }

    // No path found
    for (auto n : allNodes) delete n;
    return {};
}

void Engine::updateSmartAI(Entity& e, double deltaTime)
{
    e.pathTimer -= deltaTime;
    
    // 1. Path Planning (once every 0.5s to save CPU)
    if (e.pathTimer <= 0 || e.currentPath.empty()) {
        e.currentPath = calculateAStarPath(e.position, scene.player.position);
        e.pathTimer = 0.5f; 
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

void Engine::updateEnemyAI(Entity& e, double deltaTime)
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
        updateSmartAI(e, deltaTime);
    }
}

void Engine::updateEntities(double deltaTime)
{
    // 1. Process Logic and Mark for Deletion
    for (auto& e : scene.entities)
    {
        e.totalTime += deltaTime;

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
                updateEnemyAI(e, deltaTime); 
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