#include "platform/platform.h"
#include "ww3d.h"

#include <SDL3/SDL.h>

#include <cstdio>
#include <cstdlib>

namespace {

void *Get_Native_Window_Handle(SDL_Window *window)
{
    SDL_PropertiesID props = SDL_GetWindowProperties(window);
    if (props == 0) {
        return nullptr;
    }

#if defined(__ANDROID__)
    return SDL_GetPointerProperty(props, SDL_PROP_WINDOW_ANDROID_WINDOW_POINTER, nullptr);
#elif defined(_WIN32)
    return SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
#elif defined(__APPLE__)
    return SDL_GetPointerProperty(props, SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr);
#else
    void *handle = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, nullptr);
    if (handle != nullptr) {
        return handle;
    }
    return SDL_GetPointerProperty(props, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, nullptr);
#endif
}

bool Should_Init_WW3D()
{
    const char *value = std::getenv("OPENW3D_RENDERER_SMOKE_INIT");
    return value != nullptr && value[0] == '1';
}

} // namespace

int main(int, char **)
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "OpenW3D Renderer Smoke",
        960,
        540,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);

    if (window == nullptr) {
        std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    void *native_window = Get_Native_Window_Handle(window);
    std::printf("OpenW3D renderer smoke linked on %s; native_window=%p\n", OPENW3D_PLATFORM_NAME, native_window);

    if (Should_Init_WW3D()) {
        if (native_window == nullptr) {
            std::fprintf(stderr, "OPENW3D_RENDERER_SMOKE_INIT=1 but SDL did not expose a native window handle\n");
            SDL_DestroyWindow(window);
            SDL_Quit();
            return 2;
        }

        WW3DErrorType init_result = WW3D::Init(native_window, nullptr, true);
        std::printf("WW3D::Init returned %d\n", static_cast<int>(init_result));
        if (init_result != WW3D_ERROR_OK) {
            SDL_DestroyWindow(window);
            SDL_Quit();
            return 3;
        }

        WW3D::Begin_Render(true, true);
        WW3D::End_Render(true);
        WW3D::Shutdown();
    }

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
