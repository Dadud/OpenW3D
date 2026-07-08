// OpenW3D @feature Shared SDL3 window reference for non-Windows builds.
// Set by WINMAIN.CPP after SDL_CreateWindow succeeds. Read by
// directinput_sdl3.cpp for mouse grab and event delivery, and by
// anything else that needs the SDL3 window (e.g. dialog parenting).
// On Windows this stays null and the Win32 path is unaffected.

#pragma once

struct SDL_Window;

#ifdef __cplusplus
extern "C" {
#endif

SDL_Window* Get_SDL3_Main_Window(void);
void Set_SDL3_Main_Window(SDL_Window* win);

#ifdef __cplusplus
}
#endif
