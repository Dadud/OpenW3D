#include "foundation/runtime.h"
#include "renderer/backend.h"

#include <SDL3/SDL.h>

#include <cstdio>

int main() {
    const auto runtime = openw3d::modern::runtime_info();
    const auto backend = openw3d::renderer::selected_backend();
    std::printf("%.*s v%u backend=%s modern-api=%s\n",
        static_cast<int>(runtime.name.size()), runtime.name.data(), runtime.api_version,
        backend.name, backend.modern_api_boundary ? "yes" : "no");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    SDL_Window* window = SDL_CreateWindow("OpenW3D Modern Runtime", 960, 540, 0);
    if (!window) {
        std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    bool running = true;
    while (running) {
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
