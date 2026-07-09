#include "foundation/runtime.h"

#include <SDL3/SDL.h>

#include <cstdio>

int main(int, char**) {
    const auto info = openw3d::modern::runtime_info();
    std::printf("%.*s v%u\n", static_cast<int>(info.name.size()), info.name.data(), info.api_version);

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("OpenW3D Modern", 1280, 720, SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 2;
    }

    bool running = true;
    const Uint64 deadline = SDL_GetTicks() + 500;
    while (running && SDL_GetTicks() < deadline) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) running = false;
        }
        SDL_Delay(16);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
