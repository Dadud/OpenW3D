#include "platform/platform.h"
#include "wwdebug.h"

#include <SDL3/SDL.h>
#include <cstdio>

int main(int, char **)
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "OpenW3D Platform Shell",
        960,
        540,
        SDL_WINDOW_RESIZABLE);

    if (window == nullptr) {
        std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    std::printf("OpenW3D platform shell initialized on %s\n", OPENW3D_PLATFORM_NAME);

    SDL_Event event;
    const Uint64 deadline = SDL_GetTicks() + 250;
    while (SDL_GetTicks() < deadline) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                SDL_DestroyWindow(window);
                SDL_Quit();
                return 0;
            }
        }
        SDL_Delay(16);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
