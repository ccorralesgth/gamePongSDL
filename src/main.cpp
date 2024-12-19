#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <cmath>

// Screen dimensions
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

// Paddle dimensions
const int PADDLE_WIDTH = 20;
const int PADDLE_HEIGHT = 100;

// Ball dimensions
const int BALL_SIZE = 20;

// Ball speed
int ballSpeedX = 5;
int ballSpeedY = 5;

// Scores
int leftScore = 0;
int rightScore = 0;
const int MAX_SCORE = 1;

void renderCenterText(SDL_Renderer *renderer, TTF_Font *font, const std::string &text, int width = SCREEN_WIDTH, int heigh = SCREEN_HEIGHT)
{
    SDL_Color color = {255, 255, 255, 255}; // White color
    // SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    SDL_Surface *surface = TTF_RenderText_Blended(font, text.c_str(), color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dstRect = {(width / 2) - surface->w / 2, (heigh / 2) - surface->h / 2, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &dstRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void renderDashedLine(SDL_Renderer *renderer, int x, int y1, int y2, int dashLength, int gapLength)
{
    bool draw = true;
    for (int y = y1; y < y2; y += dashLength + gapLength)
    {
        if (draw)
        {
            SDL_RenderDrawLine(renderer, x, y, x, y + dashLength);
        }
        draw = !draw;
    }
}

void renderBorder(SDL_Renderer *renderer, int thickness)
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White color
    for (int i = 0; i < thickness; ++i)
    {
        SDL_RenderDrawLine(renderer, i, i, SCREEN_WIDTH - i, i);                                         // Top border
        SDL_RenderDrawLine(renderer, i, i, i, SCREEN_HEIGHT - i);                                        // Left border
        SDL_RenderDrawLine(renderer, SCREEN_WIDTH - 1 - i, i, SCREEN_WIDTH - 1 - i, SCREEN_HEIGHT - i);  // Right border
        SDL_RenderDrawLine(renderer, i, SCREEN_HEIGHT - 1 - i, SCREEN_WIDTH - i, SCREEN_HEIGHT - 1 - i); // Bottom border
    }
}

void renderText(SDL_Renderer *renderer, TTF_Font *font, const std::string &text, int x, int y)
{
    SDL_Color color = {255, 255, 255, 255}; // White color
    // SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    SDL_Surface *surface = TTF_RenderText_Blended(font, text.c_str(), color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dstRect = {x, y, surface->w, surface->h};
    // SDL_Rect dstRect = {x - surface->w / 2, y - surface->h / 2, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &dstRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

// SDL_Texture* renderText(const std::string &message,const std::string &fontFile, SDL_Color color,int fontSize, SDL_Renderer* renderer) {
//     TTF_Font* font = TTF_OpenFont(fontFile.c_str(), fontSize);
//     SDL_Surface* surf = TTF_RenderText_Blended(font, message.c_str(), color);
//     SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surf);
//     SDL_FreeSurface(surf);
//     TTF_CloseFont(font);
//     return texture;
// }

void renderGradientCircle(SDL_Renderer *renderer, int centerX, int centerY, int radius)
{
    for (int y = -radius; y <= radius; y++)
    {
        for (int x = -radius; x <= radius; x++)
        {
            if (x * x + y * y <= radius * radius)
            {
                int distance = std::sqrt(x * x + y * y);
                int colorValue = 255 - (distance * 255 / radius);
                SDL_SetRenderDrawColor(renderer, colorValue, colorValue, colorValue, 255);
                SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
            }
        }
    }
}

int main(int argc, char *argv[])
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return -1;
    }

    // Initialize SDL_ttf
    if (TTF_Init() == -1)
    {
        std::cerr << "Failed to initialize SDL_ttf: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    // Load Fonts
    SDL_Color font_color = {255, 255, 255, 255};
    std::string font_name = "res/fonts/NES-Chimera/NES-Chimera.ttf";
    TTF_Font *font = TTF_OpenFont(font_name.c_str(), 25);
    if (!font)
    {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        std::cerr << "Failed to initialize SDL_mixer: " << Mix_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    // load sound effects
    Mix_Chunk *paddleSound = Mix_LoadWAV("res/sounds/paddle_hit.wav");
    Mix_Chunk *wallSound = Mix_LoadWAV("res/sounds/wall_hit.wav");
    Mix_Chunk *scoreSound = Mix_LoadWAV("res/sounds/score_update.wav");

    if (!paddleSound || !wallSound || !scoreSound)
    {
        std::cerr << "Failed to load sound effects: " << Mix_GetError() << std::endl;
        Mix_CloseAudio();
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    // Create window
    SDL_Window *window = SDL_CreateWindow("Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window)
    {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        Mix_FreeChunk(paddleSound);
        Mix_FreeChunk(wallSound);
        Mix_FreeChunk(scoreSound);
        Mix_CloseAudio();
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    // Create renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        Mix_FreeChunk(paddleSound);
        Mix_FreeChunk(wallSound);
        Mix_FreeChunk(scoreSound);
        Mix_CloseAudio();
        SDL_Quit();
        return -1;
    }

    // Paddles and ball
    SDL_Rect paddle1 = {50, (SCREEN_HEIGHT / 2) - (PADDLE_HEIGHT / 2), PADDLE_WIDTH, PADDLE_HEIGHT};
    SDL_Rect paddle2 = {SCREEN_WIDTH - 50 - PADDLE_WIDTH, (SCREEN_HEIGHT / 2) - (PADDLE_HEIGHT / 2), PADDLE_WIDTH, PADDLE_HEIGHT};
    SDL_Rect ball = {(SCREEN_WIDTH / 2) - (BALL_SIZE / 2), (SCREEN_HEIGHT / 2) - (BALL_SIZE / 2), BALL_SIZE, BALL_SIZE};

    // Show start screen
    bool start = false;
    while (!start)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT)
            {
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                Mix_FreeChunk(paddleSound);
                Mix_FreeChunk(wallSound);
                Mix_FreeChunk(scoreSound);
                Mix_CloseAudio();
                TTF_Quit();
                SDL_Quit();
                return 0;
            }
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN)
            {
                start = true;
            }
            else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
            {
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                Mix_FreeChunk(paddleSound);
                Mix_FreeChunk(wallSound);
                Mix_FreeChunk(scoreSound);
                Mix_CloseAudio();
                TTF_Quit();
                SDL_Quit();
                return 0;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
        SDL_RenderClear(renderer);
        renderCenterText(renderer, font, "Press Enter to Start", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_RenderPresent(renderer);
    }

    bool running = true;
    bool gameOver = false;
    SDL_Event event;

    // Game loop
    while (running)
    {
        // Handle events
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
            if (gameOver && event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
            {
                running = false;
            }
            else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN)
            {
                // gameOver = false;
                if (gameOver){
                    leftScore = 0;
                rightScore = 0;
                ball.x = (SCREEN_WIDTH / 2) - (BALL_SIZE / 2);
                ball.y = (SCREEN_HEIGHT / 2) - (BALL_SIZE / 2);
                ballSpeedX = 5;
                ballSpeedY = 5;
                }else{
                    gameOver = false;
                    leftScore = 0;
                    rightScore = 0;
                }
                

                // Clear screen
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderClear(renderer); // TODO: why do we need to render clear here?
            }
        }

        if (!gameOver)
        {
            // Keyboard state
            const Uint8 *keystate = SDL_GetKeyboardState(NULL);
            if (keystate[SDL_SCANCODE_W] && paddle1.y > 0)
                paddle1.y -= 5;
            if (keystate[SDL_SCANCODE_S] && paddle1.y < SCREEN_HEIGHT - PADDLE_HEIGHT)
                paddle1.y += 5;
            if (keystate[SDL_SCANCODE_UP] && paddle2.y > 0)
                paddle2.y -= 5;
            if (keystate[SDL_SCANCODE_DOWN] && paddle2.y < SCREEN_HEIGHT - PADDLE_HEIGHT)
                paddle2.y += 5;

            // Move ball
            ball.x += ballSpeedX;
            ball.y += ballSpeedY;

            // Ball collision with top and bottom
            if (ball.y <= 0 || ball.y + BALL_SIZE >= SCREEN_HEIGHT)
            {
                ballSpeedY = -ballSpeedY;
            }

            // Ball collision with paddles
            if (SDL_HasIntersection(&ball, &paddle1) || SDL_HasIntersection(&ball, &paddle2))
            {
                ballSpeedX = -ballSpeedX;
                Mix_PlayChannel(-1, paddleSound, 0);
            }

            if (ball.x <= 0)
            {
                rightScore++;
                ball.x = (SCREEN_WIDTH / 2) - (BALL_SIZE / 2);
                ball.y = (SCREEN_HEIGHT / 2) - (BALL_SIZE / 2);
                ballSpeedX = -ballSpeedX;
                Mix_PlayChannel(-1, scoreSound, 0);
            }
            else if (ball.x + BALL_SIZE >= SCREEN_WIDTH)
            {
                leftScore++;
                ball.x = (SCREEN_WIDTH / 2) - (BALL_SIZE / 2);
                ball.y = (SCREEN_HEIGHT / 2) - (BALL_SIZE / 2);
                ballSpeedX = -ballSpeedX;
                Mix_PlayChannel(-1, scoreSound, 0);
            }

            if (rightScore >= MAX_SCORE || leftScore >= MAX_SCORE)
            {
                // running = false;
                gameOver = true;
            }
        }

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer); // TODO: why do we need to render clear here?

        if (gameOver)
        {
            if (leftScore > rightScore)
            {
                renderCenterText(renderer, font, "Left Player Wins", SCREEN_WIDTH, SCREEN_HEIGHT);
            }
            else
            {
                renderCenterText(renderer, font, "Right Player Wins", SCREEN_WIDTH, SCREEN_HEIGHT);
            }
            renderCenterText(renderer, font, "Press Enter to Restart", SCREEN_WIDTH, SCREEN_HEIGHT + 50);
            renderCenterText(renderer, font, "Press ESC to Quit", SCREEN_WIDTH, SCREEN_HEIGHT + 100);
        }
        else
        {
            // Clear screen
            // SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            // SDL_RenderClear(renderer); // TODO: why do we need to render clear here?

            // Render border
            renderBorder(renderer, 5);

            // Render center circle
            renderGradientCircle(renderer, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 100);

            // Draw paddles and ball
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderFillRect(renderer, &paddle1);
            SDL_RenderFillRect(renderer, &paddle2);
            SDL_RenderFillRect(renderer, &ball);

            // Render scores
            renderText(renderer, font, std::to_string(leftScore), (SCREEN_WIDTH / 2) - 50, 50);
            renderText(renderer, font, std::to_string(rightScore), (SCREEN_WIDTH / 2) + 50, 50);

            // Render dashed center line
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White color
            renderDashedLine(renderer, SCREEN_WIDTH / 2, 0, SCREEN_HEIGHT, 10, 10);
        }

        // Update screen
        SDL_RenderPresent(renderer);

        // Delay to control frame rate
        SDL_Delay(16);
    }

    // Clean up
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_FreeChunk(paddleSound);
    Mix_FreeChunk(wallSound);
    Mix_FreeChunk(scoreSound);
    Mix_CloseAudio();
    TTF_Quit();
    SDL_Quit();

    return 0;
}