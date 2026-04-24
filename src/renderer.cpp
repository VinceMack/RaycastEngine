#include "renderer.h"

#include <algorithm>
#include <cmath>

#include <omp.h>

Renderer::Renderer(SDLContext& sdl, AssetManager& assetManager) : sdl(sdl), assetManager(assetManager), depthBuffer(screen_width, 0.0)
{
}

void Renderer::render(const Scene& scene)
{
    // 1. Prepare for the new frame
    std::fill(depthBuffer.begin(), depthBuffer.end(), INFINITY);

    // 2. Draw the 3D world (Back to Front)
    renderFloorAndCeiling(scene);
    renderWalls(scene);
    renderEntities(scene); // These use the depthBuffer to hide behind walls

    // 3. Draw the 2D UI / Overlay (Always on top)
    renderWeapon(scene); // This IGNORES the depthBuffer

    // 4. Final output to the window
    SDL_UpdateTexture(sdl.texture, nullptr, sdl.pixelBuffer.data(), screen_width * sizeof(uint32_t));
    SDL_RenderClear(sdl.renderer);
    SDL_RenderCopy(sdl.renderer, sdl.texture, nullptr, nullptr);
    SDL_RenderPresent(sdl.renderer);
}

const std::vector<double>& Renderer::getDepthBuffer() const
{
    return depthBuffer;
}

int Renderer::wrapPow2(int value, int size) const
{
    return value & (size - 1);
}

RayHit Renderer::castRay(int x, const Player& player, const Map& map_grid)
{
    double cameraX = 2.0 * x / screen_width - 1;
    Vector2 rayDir = player.direction + (player.plane * cameraX);

    int step_dir_x;
    if (rayDir.x > 0)       step_dir_x = 1;
    else if (rayDir.x < 0)  step_dir_x = -1;
    else                    step_dir_x = 0;

    int step_dir_y;
    if (rayDir.y > 0)       step_dir_y = 1;
    else if (rayDir.y < 0)  step_dir_y = -1;
    else                    step_dir_y = 0;

    int ray_curr_grid_x = (int)player.position.x;
    int ray_curr_grid_y = (int)player.position.y;

    double ray_step_cost_x = fabs(1.0 / rayDir.x);
    double ray_step_cost_y = fabs(1.0 / rayDir.y);

    double dist_to_next_grid_line_x;
    if (rayDir.x > 0)       dist_to_next_grid_line_x = (ray_curr_grid_x + 1.0 - player.position.x) * ray_step_cost_x;
    else if (rayDir.x < 0)  dist_to_next_grid_line_x = (player.position.x - ray_curr_grid_x) * ray_step_cost_x;
    else                    dist_to_next_grid_line_x = INFINITY;

    double dist_to_next_grid_line_y;
    if (rayDir.y > 0)       dist_to_next_grid_line_y = (ray_curr_grid_y + 1.0 - player.position.y) * ray_step_cost_y;
    else if (rayDir.y < 0)  dist_to_next_grid_line_y = (player.position.y - ray_curr_grid_y) * ray_step_cost_y;
    else                    dist_to_next_grid_line_y = INFINITY;

    bool hit = false;
    int side = 0;
    while (!hit)
    {
        if (dist_to_next_grid_line_x > dist_to_next_grid_line_y)
        {
            dist_to_next_grid_line_y += ray_step_cost_y;
            ray_curr_grid_y += step_dir_y;
            side = 1;
        }
        else
        {
            dist_to_next_grid_line_x += ray_step_cost_x;
            ray_curr_grid_x += step_dir_x;
            side = 0;
        }

        if (map_grid.at(ray_curr_grid_x, ray_curr_grid_y) > 0) hit = true;
    }

    double final_wall_dist;
    if (side == 0) final_wall_dist = dist_to_next_grid_line_x - ray_step_cost_x;
    else           final_wall_dist = dist_to_next_grid_line_y - ray_step_cost_y;

    return {final_wall_dist, map_grid.at(ray_curr_grid_x, ray_curr_grid_y), side, ray_curr_grid_x, ray_curr_grid_y, rayDir};
}

void Renderer::renderFloorAndCeiling(const Scene& scene)
{
    const Player& player = scene.player;
    const Texture& floorTexture = assetManager.getTexture(3);
    const Texture& ceilingTexture = assetManager.getTexture(6);

    #pragma omp parallel for
    for (int y = screen_height / 2; y < screen_height; ++y)
    {
        float p = y - screen_height / 2;
        float posZ = 0.5f * screen_height;
        float rowDist = posZ / p;

        double visibility = 1.0 / exp(rowDist * 0.07);
        visibility = std::min(1.0, std::max(0.0, visibility));
        double invVis = 1.0 - visibility;

        double fogR_part = 119.0 * invVis;
        double fogG_part = 119.0 * invVis;
        double fogB_part = 119.0 * invVis;

        Vector2 rayDirLeft = player.direction - player.plane;
        Vector2 rayDirRight = player.direction + player.plane;

        double floorStepX = rowDist * (rayDirRight.x - rayDirLeft.x) / screen_width;
        double floorStepY = rowDist * (rayDirRight.y - rayDirLeft.y) / screen_width;

        double floorX = player.position.x + rowDist * rayDirLeft.x;
        double floorY = player.position.y + rowDist * rayDirLeft.y;

        for (int x = 0; x < screen_width; ++x)
        {
            double fracX = floorX - floor(floorX);
            double fracY = floorY - floor(floorY);

            int floorTx = wrapPow2((int)(fracX * floorTexture.width), floorTexture.width);
            int floorTy = wrapPow2((int)(fracY * floorTexture.height), floorTexture.height);
            int ceilTx = wrapPow2((int)(fracX * ceilingTexture.width), ceilingTexture.width);
            int ceilTy = wrapPow2((int)(fracY * ceilingTexture.height), ceilingTexture.height);

            floorX += floorStepX;
            floorY += floorStepY;

            uint32_t fCol = floorTexture.at(floorTx, floorTy);
            uint32_t cCol = ceilingTexture.at(ceilTx, ceilTy);

            uint8_t fr = ((fCol >> 16) & 0xFF) * visibility + fogR_part;
            uint8_t fg = ((fCol >> 8) & 0xFF) * visibility + fogG_part;
            uint8_t fb = (fCol & 0xFF) * visibility + fogB_part;

            uint8_t cr = ((cCol >> 16) & 0xFF) * visibility + fogR_part;
            uint8_t cg = ((cCol >> 8) & 0xFF) * visibility + fogG_part;
            uint8_t cb = (cCol & 0xFF) * visibility + fogB_part;

            sdl.pixelBuffer[y * screen_width + x] = (0xFF << 24) | (fr << 16) | (fg << 8) | fb;
            sdl.pixelBuffer[(screen_height - y - 1) * screen_width + x] = (0xFF << 24) | (cr << 16) | (cg << 8) | cb;
        }
    }
}

void Renderer::renderWalls(const Scene& scene)
{
    const Player& player = scene.player;
    const Map& map_grid = scene.map_grid;

    #pragma omp parallel for
    for (int x = 0; x < screen_width; x++)
    {
        RayHit hit = castRay(x, player, map_grid);
        depthBuffer[x] = hit.distance;

        int line_height = screen_height / hit.distance;
        int draw_start = -(line_height / 2) + (screen_height / 2);
        int draw_end = (line_height / 2) + (screen_height / 2);
        if (draw_start < 0) draw_start = 0;
        if (draw_end >= screen_height) draw_end = screen_height - 1;

        int texNum = hit.wallType - 1;
        const Texture& tex = assetManager.getTexture(texNum);

        double wallX;
        if (hit.side == 0) wallX = player.position.y + hit.distance * hit.rayDir.y;
        else               wallX = player.position.x + hit.distance * hit.rayDir.x;
        wallX -= floor(wallX);

        int texX = (int)(wallX * (double)tex.width);

        if (hit.side == 0 && hit.rayDir.x > 0) texX = tex.width - texX - 1;
        if (hit.side == 1 && hit.rayDir.y < 0) texX = tex.width - texX - 1;

        texX = wrapPow2(texX, tex.width);

        double step = 1.0 * tex.height / line_height;
        double texPos = (draw_start - screen_height / 2 + line_height / 2) * step;

        double visibility = 1.0 / exp(hit.distance * 0.07);
        visibility = std::min(1.0, std::max(0.0, visibility));

        double sideShade = (hit.side == 1) ? 0.5 : 1.0;
        double finalVis = visibility * sideShade;
        double invVis = 1.0 - visibility;
        double fogR_part = 119.0 * invVis;
        double fogG_part = 119.0 * invVis;
        double fogB_part = 119.0 * invVis;

        uint32_t* pixelPtr = &sdl.pixelBuffer[draw_start * screen_width + x];
        for (int y = draw_start; y < draw_end; y++)
        {
            int texY = wrapPow2((int)texPos, tex.height);
            texPos += step;

            uint32_t color = tex.at(texX, texY);

            uint8_t r = ((color >> 16) & 0xFF) * finalVis + fogR_part;
            uint8_t g = ((color >> 8) & 0xFF) * finalVis + fogG_part;
            uint8_t b = (color & 0xFF) * finalVis + fogB_part;

            *pixelPtr = (0xFF << 24) | (r << 16) | (g << 8) | b;
            pixelPtr += screen_width;
        }
    }
}

void Renderer::renderEntities(const Scene& scene)
{
    const Player& player = scene.player;
    // Work on a copy for sorting, or sort the scene entities directly if preferred
    std::vector<Entity> entities = scene.entities;

    // 1. Calculate squared distance for each entity (faster than sqrt)
    for (auto& e : entities) {
        e.dist = ((player.position.x - e.position.x) * (player.position.x - e.position.x) + 
                  (player.position.y - e.position.y) * (player.position.y - e.position.y));
    }

    // 2. Sort entities: Farthest to Nearest (Painter's Algorithm)
    std::sort(entities.begin(), entities.end(), [](const Entity& a, const Entity& b) {
        return a.dist > b.dist;
    });

    // 3. Project and Draw
    for (const auto& e : entities) {
        double spriteX = e.position.x - player.position.x;
        double spriteY = e.position.y - player.position.y;

        double invDet = 1.0 / (player.plane.x * player.direction.y - player.direction.x * player.plane.y);
        double transformX = invDet * (player.direction.y * spriteX - player.direction.x * spriteY);
        double transformY = invDet * (-player.plane.y * spriteX + player.plane.x * spriteY);

        if (transformY <= 0) continue; 

        // Use the entity's specific frame for animation
        const Texture& tex = assetManager.getTexture(e.textureIndex + e.currentFrame);
        if (tex.width <= 0 || tex.height <= 0)
        {
            continue;
        }

        double aspectRatio = (double)tex.width / (double)tex.height;

        int spriteScreenX = int((screen_width / 2) * (1 + transformX / transformY));
        
        // Use the entity's specific vOffset (1.0 = floor, 0.0 = horizon)
        int spriteScreenY = int((screen_height / 2) * (1 + e.vOffset / transformY));

        // Use the entity's specific scale
        int spriteHeight = std::abs(int(screen_height / transformY * e.scale));
        int spriteWidth = std::abs(int(spriteHeight * aspectRatio));
        if (spriteHeight <= 0 || spriteWidth <= 0)
        {
            continue;
        }

        int drawEndY = std::min(screen_height, spriteScreenY);
        int drawStartY = std::max(0, spriteScreenY - spriteHeight);
        int drawStartX = std::max(0, -spriteWidth / 2 + spriteScreenX);
        int drawEndX = std::min(screen_width, spriteWidth / 2 + spriteScreenX);
        if (drawStartX >= drawEndX || drawStartY >= drawEndY)
        {
            continue;
        }

        int spriteLeftX = -spriteWidth / 2 + spriteScreenX;
        int spriteTopY = spriteScreenY - spriteHeight;

        // Fog calculation (same as before)
        double visibility = 1.0 / exp(transformY * 0.07);
        visibility = std::min(1.0, std::max(0.0, visibility));
        double invVis = 1.0 - visibility;
        double fog_component = 119.0 * invVis; 

        for (int stripe = drawStartX; stripe < drawEndX; stripe++) {
            int texX = int(256 * (stripe - spriteLeftX) * tex.width / spriteWidth) / 256;
            texX = std::clamp(texX, 0, tex.width - 1);

            if (transformY < depthBuffer[stripe]) {
                uint32_t* pixelPtr = &sdl.pixelBuffer[drawStartY * screen_width + stripe];
                
                for (int y = drawStartY; y < drawEndY; y++) {
                    int texY = ((y - spriteTopY) * tex.height) / spriteHeight;
                    texY = std::clamp(texY, 0, tex.height - 1);
                    uint32_t color = tex.at(texX, texY);

                    if ((color >> 24) & 0xFF) { 
                        uint8_t r = ((color >> 16) & 0xFF) * visibility + fog_component;
                        uint8_t g = ((color >> 8) & 0xFF) * visibility + fog_component;
                        uint8_t b = (color & 0xFF) * visibility + fog_component;
                        *pixelPtr = (0xFF << 24) | (r << 16) | (g << 8) | b;
                    }
                    pixelPtr += screen_width;
                }
            }
        }
    }
}

void Renderer::renderWeapon(const Scene& scene)
{
    if (!scene.player.currentWeapon) return;

    const Texture& tex = assetManager.getTexture(scene.player.currentWeapon->handTextureIndex);
    if (tex.width <= 0 || tex.height <= 0)
    {
        return;
    }

    // 2. Scale the weapon to fit the screen
    double screenScale = 2.0; 
    int drawW = tex.width * screenScale;
    int drawH = tex.height * screenScale;
    if (drawW <= 0 || drawH <= 0)
    {
        return;
    }

    // 3. Position: bottom right of the screen
    int startX = screen_width - drawW;
    int startY = screen_height - drawH;

    int clipStartX = std::max(0, startX);
    int clipStartY = std::max(0, startY);
    int clipEndX = std::min(screen_width, startX + drawW);
    int clipEndY = std::min(screen_height, startY + drawH);

    if (clipStartX >= clipEndX || clipStartY >= clipEndY)
    {
        return;
    }

    // 4. Draw directly to pixelBuffer (ignoring depthBuffer!)
    for (int x = clipStartX; x < clipEndX; x++) {
        for (int y = clipStartY; y < clipEndY; y++) {
            int localX = x - startX;
            int localY = y - startY;

            int texX = (localX * tex.width) / drawW;
            int texY = (localY * tex.height) / drawH;
            texX = std::clamp(texX, 0, tex.width - 1);
            texY = std::clamp(texY, 0, tex.height - 1);
            uint32_t color = tex.at(texX, texY);

            if ((color >> 24) & 0xFF) { // Alpha check
                sdl.pixelBuffer[y * screen_width + x] = color;
            }
        }
    }
}