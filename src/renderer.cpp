#include "renderer.h"

#include <algorithm>
#include <cmath>

#include <omp.h>

Renderer::Renderer(SDLContext& sdl) : sdl(sdl), depthBuffer(screen_width, 0.0)
{
}

void Renderer::render(const Scene& scene)
{
    renderFloorAndCeiling(scene);
    renderWalls(scene);
    renderSprites(scene);

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
    const Texture& floorTexture = scene.textures[3];
    const Texture& ceilingTexture = scene.textures[6];

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
        const Texture& tex = scene.textures[texNum];

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

void Renderer::renderSprites(const Scene& scene)
{
    const Player& player = scene.player;
    std::vector<Sprite> sprites = scene.entities;

    // 1. Calculate distance for each sprite
    for (auto& s : sprites) {
        s.dist = ((player.position.x - s.position.x) * (player.position.x - s.position.x) + 
                (player.position.y - s.position.y) * (player.position.y - s.position.y));
    }

    // 2. Sort sprites: Farthest to Nearest (Painter's Algorithm)
    std::sort(sprites.begin(), sprites.end(), [](const Sprite& a, const Sprite& b) {
        return a.dist > b.dist;
    });

    // 3. Project and Draw
    for (const auto& s : sprites) {
        // Translate sprite position relative to player
        double spriteX = s.position.x - player.position.x;
        double spriteY = s.position.y - player.position.y;

        // Transform with the inverse camera matrix
        // invDet is required for matrix multiplication
        double invDet = 1.0 / (player.plane.x * player.direction.y - player.direction.x * player.plane.y);

        double transformX = invDet * (player.direction.y * spriteX - player.direction.x * spriteY);
        double transformY = invDet * (-player.plane.y * spriteX + player.plane.x * spriteY); // Depth

        // Determine screen x-coordinate
        int spriteScreenX = int((screen_width / 2) * (1 + transformX / transformY));

        // Calculate height and width of the sprite on screen
        int spriteHeight = std::abs(int(screen_height / transformY));
        int spriteWidth = std::abs(int(screen_height / transformY));

        // Calculate drawing boundaries
        int drawStartY = std::max(0, -spriteHeight / 2 + screen_height / 2);
        int drawEndY = std::min(screen_height - 1, spriteHeight / 2 + screen_height / 2);
        int drawStartX = std::max(0, -spriteWidth / 2 + spriteScreenX);
        int drawEndX = std::min(screen_width - 1, spriteWidth / 2 + spriteScreenX);

        const Texture& tex = scene.textures[s.textureIndex];

        // Draw the sprite vertical stripe by vertical stripe
        for (int stripe = drawStartX; stripe < drawEndX; stripe++) {
            int texX = int(256 * (stripe - (-spriteWidth / 2 + spriteScreenX)) * tex.width / spriteWidth) / 256;

            // Check if:
            // 1. It's in front of the camera (transformY > 0)
            // 2. It's not obscured by a wall (transformY < depthBuffer[stripe])
            if (transformY > 0 && transformY < depthBuffer[stripe]) {
                for (int y = drawStartY; y < drawEndY; y++) {
                    int d = y * 256 - screen_height * 128 + spriteHeight * 128;
                    int texY = ((d * tex.height) / spriteHeight) / 256;

                    uint32_t color = tex.at(texX, texY);
                    uint8_t alpha = (color >> 24) & 0xFF;
                    if(alpha > 0) sdl.pixelBuffer[y * screen_width + stripe] = color;
                }
            }
        }
    }
}
