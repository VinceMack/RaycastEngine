#include "engine.h"

#include <string>
#include <cmath>
#include <algorithm>
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
    std::vector<Node*> allNodes; // Keep track for memory cleanup

    auto addNode = [&](int x, int y, double g, Node* p) {
        Node* n = new Node{x, y, g, getHeuristic(x, y), p};
        allNodes.push_back(n);
        openList.push(n);
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
            for (auto n : allNodes) delete n;
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
                    // If moving diagonally, ensure both adjacent orthogonal cells are empty
                    // Example: Moving NE? Must ensure N and E are not walls.
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

    // Cleanup memory if no path found
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