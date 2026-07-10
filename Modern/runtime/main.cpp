#include "foundation/runtime.h"
#include "renderer/backend.h"
#include "renderer/rhi.h"

#include <SDL3/SDL.h>

#include <cstdio>

int main() {
    const auto runtime = openw3d::modern::runtime_info();
    const auto backend = openw3d::renderer::selected_backend();
    auto device = openw3d::renderer::create_device({runtime.name, true});
    const bool probe_ok = device && device->create_buffer({256, false}) &&
        device->create_texture({64, 64, openw3d::renderer::Format::RGBA8Unorm}) &&
        device->begin_frame() && device->end_frame();
    std::printf("%.*s v%u backend=%s modern-api=%s rhi-probe=%s\n",
        static_cast<int>(runtime.name.size()), runtime.name.data(), runtime.api_version,
        backend.name, backend.modern_api_boundary ? "yes" : "no", probe_ok ? "ok" : "failed");

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
