# DXVK needs a shared SDL3 library
if(NOT W3D_BUILD_OPTION_SDL3)
    return()
endif()
find_package(SDL3 COMPONENTS SDL3-shared Headers)

if(NOT SDL3_FOUND)
    message(WARNING "SDL3 not found — disabling SDL3 backend. Install libsdl3-dev for full client.")
    unset(W3D_BUILD_OPTION_SDL3)
    set(W3D_BUILD_OPTION_SDL3 OFF CACHE BOOL "Build OpenW3D with SDL3." FORCE)
    set(W3D_BUILD_OPTION_SDL3 OFF)
endif()
