# SDL3 support - optional for OpenW3D
# This module is only included when W3D_BUILD_OPTION_SDL3 is ON and on Windows

if(W3D_BUILD_OPTION_SDL3)
    find_package(SDL3 QUIET)
    if(SDL3_FOUND)
        message(STATUS "SDL3 found - enabling SDL input driver")
    else()
        message(STATUS "SDL3 not found - using stub implementation")
        # Create stub targets so dependent libraries don't fail to configure
        add_library(SDL3::SDL3-shared INTERFACE)
        add_library(SDL3::Headers INTERFACE)
    endif()
endif()

# Always define sdl3stub for compatibility
add_library(sdl3stub INTERFACE)
