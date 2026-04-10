#include <SDL2/SDL.h>
#include <cstdint>
#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <omp.h>

constexpr int screen_width = 640;
constexpr int screen_height = 480;
constexpr int map_size = 24;
constexpr int textureHeight = 64;
constexpr int textureWidth = 64;

struct SDL
{
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;
    std::vector<uint32_t> pixelBuffer;
    bool running = false;

    SDL(int width, int height)
    {
        // Initalize SDL
        SDL_Init(SDL_INIT_VIDEO);

        // Initalize SDL Window
        window = SDL_CreateWindow(
            "raycaster",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            width,
            height,
            SDL_WINDOW_SHOWN
        );

        // Intalize SDL Rednerer
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

        // Initalize SDL Texture
        texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            width,
            height
        );

        pixelBuffer.resize(width * height, 0); 
        running = true;
    }

    void fillBuffer(uint32_t color)
    {
        for (int i = 0; i < screen_width * screen_height; ++i)
            pixelBuffer[i] = color;
    }

    void fillBuffer(uint32_t topColor, uint32_t bottomColor)
    {
        int half = (screen_width * screen_height) / 2;
        for (int i = 0; i < half; ++i) pixelBuffer[i] = topColor;
        for (int i = half; i < screen_width * screen_height; ++i) pixelBuffer[i] = bottomColor;
    }

    ~SDL()
    {
        if (texture) SDL_DestroyTexture(texture);
        if (renderer) SDL_DestroyRenderer(renderer);
        if (window) SDL_DestroyWindow(window);
        SDL_Quit();
    }
};

struct Vector2
{
    double x, y;

    Vector2 operator*(double scalar) const { return {x * scalar, y * scalar}; }
    Vector2 operator+(const Vector2& other) const { return {x + other.x, y + other.y}; }
    Vector2 operator-(const Vector2& other) const { return {x - other.x, y - other.y}; }
    void operator+=(const Vector2& other) { x += other.x; y += other.y; }
    void operator-=(const Vector2& other) { x -= other.x; y -= other.y; }
};

struct Player
{
    Vector2 position;
    Vector2 direction;
    Vector2 plane;
    double moveSpeed;
    double rotSpeed;

    Player(Vector2 pos, Vector2 dir, Vector2 plane, double rotationSpeed, double movementSpeed) : plane(plane)
    {
        position = pos;
        direction = dir;
        moveSpeed = movementSpeed;
        rotSpeed = rotationSpeed;
    }

    Player()
    {
        position = {22.0, 12.0};
        direction = {0.0, -1.0};
        plane = {0.66, 0.0};
        moveSpeed = 5;
        rotSpeed = 3;
    }
};

struct Map
{
    int grid[map_size][map_size] =
    {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,2,2,2,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
    {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,3,0,0,0,3,0,0,0,1},
    {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,2,2,0,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,4,0,0,0,0,5,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,4,0,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    };

    Map()
    {

    }
};

void generateBrickTexture(std::vector<uint32_t> &texture, uint32_t brickColor)
{
    texture.resize(64 * 64);
    for (int x = 0; x < 64; x++) {      // x is the column
        for (int y = 0; y < 64; y++) {  // y is the vertical pixel
            bool edge = (y % 16 == 0) || ((x + (y / 16) * 16) % 32 == 0);
            // Index by x first, then y, to make vertical stripes contiguous
            texture[x * 64 + y] = edge ? 0xFF000000 : brickColor;
        }
    }
}

void renderTexturedFloor(std::vector<uint32_t>& pixelBuffer, const Player& player, const std::vector<uint32_t>* textures)
{
    uint32_t fogColor = 0xFF777777; 

    #pragma omp parallel for
    for (int y = screen_height / 2; y < screen_height; ++y)
    {
        // 1. Calculate row distance from the horizon (p) and player height (posZ)
        float p = y - screen_height / 2;
        float posZ = 0.5 * screen_height;
        float rowDist = posZ / p;

        // 1.1 Calculate fog for the current row distance
        double visibility = 1.0 / exp(rowDist * 0.07);
        visibility = std::min(1.0, std::max(0.0, visibility));
        double invVis = 1.0 - visibility;
        
        // 1.2 Pre-calculate the fog parts (R, G, B)
        double fogR_part = 119.0 * invVis; // 0x77 is 119
        double fogG_part = 119.0 * invVis;
        double fogB_part = 119.0 * invVis;

        // 2. Calculate world coordinates of the leftmost and rightmost pixels
        Vector2 rayDirLeft = player.direction - player.plane;
        Vector2 rayDirRight = player.direction + player.plane;

        // 3. Calculate how much the world position changes per pixel across the row
        double floorStepX = rowDist * (rayDirRight.x - rayDirLeft.x) / screen_width;
        double floorStepY = rowDist * (rayDirRight.y - rayDirLeft.y) / screen_width;

        // 4. Start world position at the leftmost pixel
        double floorX = player.position.x + rowDist * rayDirLeft.x;
        double floorY = player.position.y + rowDist * rayDirLeft.y;

        for (int x = 0; x < screen_width; ++x)
        {
            int tx = (int)(textureWidth * (floorX - floor(floorX))) & (textureWidth - 1);
            int ty = (int)(textureHeight * (floorY - floor(floorY))) & (textureHeight - 1);
            floorX += floorStepX;
            floorY += floorStepY;

            uint32_t fCol = textures[3][textureWidth * tx + ty];
            uint32_t cCol = textures[6][textureWidth * tx + ty];

            // 2. Apply fog to floor
            uint8_t fr = ((fCol >> 16) & 0xFF) * visibility + fogR_part;
            uint8_t fg = ((fCol >> 8) & 0xFF) * visibility + fogG_part;
            uint8_t fb = (fCol & 0xFF) * visibility + fogB_part;

            // 3. Apply fog to ceiling
            uint8_t cr = ((cCol >> 16) & 0xFF) * visibility + fogR_part;
            uint8_t cg = ((cCol >> 8) & 0xFF) * visibility + fogG_part;
            uint8_t cb = (cCol & 0xFF) * visibility + fogB_part;

            pixelBuffer[y * screen_width + x] = (0xFF << 24) | (fr << 16) | (fg << 8) | fb;
            pixelBuffer[(screen_height - y - 1) * screen_width + x] = (0xFF << 24) | (cr << 16) | (cg << 8) | cb;
        }
    }
}

int main(int argc, char* argv[])
{
    // Set up SDL
    SDL sdl(screen_width, screen_height);
    // Set up player
    Player player;
    // Set up map
    Map map_grid;

    // Initialize all 8 textures in your main loop
    std::vector<uint32_t> texture[8];
    
    uint32_t colors[8] = { 0xFFFF0000, 0xFF00FF00, 0xFF0000FF, 0xFFFFFF00, 
                        0xFFFF00FF, 0xFF00FFFF, 0xFFFFFFFF, 0xFF777777 };

    for (int i = 0; i < 8; i++) {
        generateTexture(texture[i], colors[i]);
    }

    // Timing variables for frame rate control
    uint32_t lastTicks = SDL_GetTicks();

    // Handle frame.
    SDL_Event event;
    while (sdl.running)
    {
        uint32_t currentTicks = SDL_GetTicks();
        double deltaTime = (currentTicks - lastTicks) / 1000.0;
        lastTicks = currentTicks;

        // Update FPS in window title (requires #include <string>)
        double fps = 1.0 / (deltaTime > 0 ? deltaTime : 0.0001);
        SDL_SetWindowTitle(sdl.window, ("Raycaster - FPS: " + std::to_string((int)fps)).c_str());

        // Handle SDL quit event (if the window closes, stop running)
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                sdl.running = false;
        }

        // Handle Player Movement

        // Scale speeds by the time elapsed since last frame
        double frameMoveSpeed = player.moveSpeed * deltaTime;
        double frameRotSpeed = player.rotSpeed * deltaTime;

        const uint8_t* state = SDL_GetKeyboardState(NULL);
        if (state[SDL_SCANCODE_W]) 
        {
            // Calcualte the new potential position by adding the direction vector multiplied by the movement speed to the current position.
            // If the potential position is valid (i.e. not in a wall), update the player's position to the new position.
            // Repeat with both the x and y coordinates, checking each separately to allow for sliding along walls.
            double newX = player.position.x + player.direction.x * frameMoveSpeed;
            if(map_grid.grid[(int)newX][(int)player.position.y] == 0)
            {
                player.position.x = newX;
            }

            double newY = player.position.y + player.direction.y * frameMoveSpeed;
            if(map_grid.grid[(int)player.position.x][(int)newY] == 0)
            {
                player.position.y = newY;
            }
        }
        if (state[SDL_SCANCODE_S])
        {
            // Calculate the new potential position by subtracting the direction vector multiplied by the movement speed from the current position.
            // If the potential position is valid (i.e. not in a wall), update the player's position to the new position.
            // Repeat with both the x and y coordinates, checking each separately to allow for sliding along walls.
            double newX = player.position.x - player.direction.x * frameMoveSpeed;
            if(map_grid.grid[(int)newX][(int)player.position.y] == 0)
            {
                player.position.x = newX;
            }

            double newY = player.position.y - player.direction.y * frameMoveSpeed;
            if(map_grid.grid[(int)player.position.x][(int)newY] == 0)
            {
                player.position.y = newY;
            }
        }
        if (state[SDL_SCANCODE_D])
        {
            // Rotate the direction vector and camera plane vector to the right by multiplying them with the appropriate rotation matrix.
            // Repeat the same rotation for the camera plane vector.
            double oldDirX = player.direction.x;
            player.direction.x = player.direction.x * cos(frameRotSpeed) - player.direction.y * sin(frameRotSpeed);
            player.direction.y = oldDirX * sin(frameRotSpeed) + player.direction.y * cos(frameRotSpeed);

            oldDirX = player.plane.x;
            player.plane.x = player.plane.x * cos(frameRotSpeed) - player.plane.y * sin(frameRotSpeed);
            player.plane.y = oldDirX * sin(frameRotSpeed) + player.plane.y * cos(frameRotSpeed);
        }
        if (state[SDL_SCANCODE_A])
        {
            // Rotate the direction vector and camera plane vector to the left by multiplying them with the appropriate rotation matrix.
            // Repeat the same rotation for the camera plane vector.
            double oldDirX = player.direction.x;
            player.direction.x = player.direction.x * cos(-frameRotSpeed) - player.direction.y * sin(-frameRotSpeed);
            player.direction.y = oldDirX * sin(-frameRotSpeed) + player.direction.y * cos(-frameRotSpeed);

            oldDirX = player.plane.x;
            player.plane.x = player.plane.x * cos(-frameRotSpeed) - player.plane.y * sin(-frameRotSpeed);
            player.plane.y = oldDirX * sin(-frameRotSpeed) + player.plane.y * cos(-frameRotSpeed);
        }

        // Reset the pixel buffer:
        // Fill the top half of the pixel buffer (ceiling)
        // Fill the bottom half of the pixel buffer (floor)
        renderTexturedFloor(sdl.pixelBuffer, player, texture);
        
        // Iterate over every vertical column of the screen:
        #pragma omp parallel for
        for(int x=0; x < screen_width; x++)
        {
            double cameraX;
            Vector2 rayDir;

            // Calculate cameraX (the x-coordinate in camera space, from -1 to 1).
            cameraX = 2.0 * x / screen_width - 1;

            // Calculate the ray direction vector for the current column.
            rayDir = player.direction + (player.plane * cameraX);

            // Determine the step direction (+1 or -1) for grid traversal, for both x and y.
            int step_dir_x;
            if (rayDir.x > 0)       step_dir_x = 1;
            else if (rayDir.x < 0)  step_dir_x = -1;
            else                    step_dir_x = 0; // Ray is perfectly vertical; no X-step needed.

            int step_dir_y;
            if (rayDir.y > 0)       step_dir_y = 1;
            else if (rayDir.y < 0)  step_dir_y = -1;
            else                    step_dir_y = 0; // Ray is perfectly horizontal; no Y-step needed.

            // Identify the starting/current map square (integer coordinates), for both x and y.
            int ray_curr_grid_x = (int)player.position.x;
            int ray_curr_grid_y = (int)player.position.y;

            // Calculate the total length the ray travels to cross one full grid square, for both x and y.
            double ray_step_cost_x = fabs(1.0 / rayDir.x);
            double ray_step_cost_y = fabs(1.0 / rayDir.y);

            // Calculate the initial distance from the player to the first grid boundariesm, for both x and y.
            double dist_to_next_grid_line_x;
            if(rayDir.x > 0)        dist_to_next_grid_line_x = (ray_curr_grid_x + 1.0 - player.position.x) * ray_step_cost_x;
            else if(rayDir.x < 0)   dist_to_next_grid_line_x = (player.position.x - ray_curr_grid_x) * ray_step_cost_x;
            else                    dist_to_next_grid_line_x = INFINITY;
            
            double dist_to_next_grid_line_y;
            if(rayDir.y > 0)        dist_to_next_grid_line_y = (ray_curr_grid_y + 1.0 - player.position.y) * ray_step_cost_y;
            else if(rayDir.y < 0)   dist_to_next_grid_line_y = (player.position.y - ray_curr_grid_y) * ray_step_cost_y;
            else                    dist_to_next_grid_line_y = INFINITY;

            // Perform the DDA Loop until a wall is hit.
            bool hit = false;
            int side = 0; // 0 for vertical wall, 1 for horizontal wall
            while(!hit)
            {
                // Compare dist_to_next_grid_line_x and dist_to_next_grid_line_y
                // Increment the smaller distance by its corresponding step cost.
                // Update the corresponding current grid coordinate by its step direction.
                // Set side (0 for X, 1 for Y).
                if(dist_to_next_grid_line_x > dist_to_next_grid_line_y)
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

                // Check if map_grid.grid[ray_curr_grid_x][ray_curr_grid_y] > 0. If so, hit = true.
                if(map_grid.grid[ray_curr_grid_x][ray_curr_grid_y] > 0) hit = true;
            }

            // Calculate the final_wall_dist (perpendicular distance).
            double final_wall_dist;
            if(side == 0)   final_wall_dist = dist_to_next_grid_line_x - ray_step_cost_x;
            else            final_wall_dist = dist_to_next_grid_line_y - ray_step_cost_y;

            // Calculate line_height, draw_start, and draw_end.
            int line_height;
            line_height = screen_height / final_wall_dist;
            int draw_start;
            draw_start = -(line_height/2)+(screen_height/2);
            int draw_end;
            draw_end = (line_height/2)+(screen_height/2);
            // Clamp these values, if necessary.
            if(draw_start < 0) draw_start = 0;
            if(draw_end >= screen_height) draw_end = screen_height - 1;

            // Determine which texture to use based on the map grid value.
            int texNum = map_grid.grid[ray_curr_grid_x][ray_curr_grid_y] - 1;

            // Calculate the exact point of wall hit (wallX) to determine the corresponding x-coordinate on the texture.
            double wallX; 
            if (side == 0) wallX = player.position.y + final_wall_dist * rayDir.y;
            else           wallX = player.position.x + final_wall_dist * rayDir.x;
            wallX -= floor(wallX); // Get only the fraction

            // Calculate the x-coordinate on the texture (texX) by multiplying wallX by the texture width and converting to an integer.
            int texX = (int)(wallX * (double)textureWidth);

            // Flip texture if we are looking at the "back" of the wall
            if(side == 0 && rayDir.x > 0) texX = textureWidth - texX - 1;
            if(side == 1 && rayDir.y < 0) texX = textureWidth - texX - 1;

            // Calculate the step size for texture coordinate interpolation
            double step = 1.0 * textureHeight / line_height;
            // Calculate the initial texture position
            double texPos = (draw_start - screen_height / 2 + line_height / 2) * step;

            // Iterate through the vertical slice of the wall

            double visibility = 1.0 / exp(final_wall_dist * 0.07);
            visibility = std::min(1.0, std::max(0.0, visibility)); // Clamp 0 to 1
            uint32_t fogColor = 0xFF777777;

            // Pre-calculate the fog color part (FogColor * (1 - visibility))
            double sideShade = (side == 1) ? 0.5 : 1.0; 
            double finalVis = visibility * sideShade;
            double invVis = 1.0 - visibility;
            double fogR_part = 119.0 * invVis; // 0x77 is 119
            double fogG_part = 119.0 * invVis;
            double fogB_part = 119.0 * invVis;

            uint32_t* pixelPtr = &sdl.pixelBuffer[draw_start * screen_width + x];
            for(int y = draw_start; y < draw_end; y++)
            {
                // Cast the texture position to an integer and mask it to stay within 0-63
                int texY = (int)texPos & (textureHeight - 1);
                texPos += step;

                // Get the color from the texture using texNum, texX, and texY.
                uint32_t color = texture[texNum][textureWidth * texX + texY];

                // Calculate fog effect based on distance. The farther the wall, the more it should blend with the fog color (e.g., gray).
                uint8_t r = ((color >> 16) & 0xFF) * finalVis + fogR_part;
                uint8_t g = ((color >> 8) & 0xFF) * finalVis + fogG_part;
                uint8_t b = (color & 0xFF) * finalVis + fogB_part;

                // Set the pixel color in the pixel buffer at (x, y) to the color obtained from the texture.
                *pixelPtr = (0xFF << 24) | (r << 16) | (g << 8) | b;
                pixelPtr += screen_width;
            }
        }

        // Upload pixels
        SDL_UpdateTexture(sdl.texture, nullptr, sdl.pixelBuffer.data(), screen_width * sizeof(uint32_t));
        // Clear frame
        SDL_RenderClear(sdl.renderer);
        // Draw texture
        SDL_RenderCopy(sdl.renderer, sdl.texture, nullptr, nullptr);
        // Present frame
        SDL_RenderPresent(sdl.renderer);
    }

    return 0;
}

// cameraX & rayDir
// cameraX is a normalized coordinate that maps every vertical stripe of your screen to a value between -1 and 1.
// It tells the engine where the current ray is relative to the center of the player's view:
// We use this value to "stretch" the Camera Plane vector. By multiplying the plane vector by cameraX, we find the offset from the center.
// Adding this offset to the Direction vector gives us the unique direction for that specific ray: rayDir = dir + (plane * cameraX)
// This ensures that the rays fan out correctly to cover your entire Field of View (FOV).