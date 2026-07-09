set(OPENW3D_SDL3_SOURCE_DIR "" CACHE PATH "Optional local SDL3 source checkout to build instead of find_package(SDL3)")

if(OPENW3D_SDL3_SOURCE_DIR AND EXISTS "${OPENW3D_SDL3_SOURCE_DIR}/CMakeLists.txt")
    set(SDL_SHARED ON CACHE BOOL "Build SDL3 shared library" FORCE)
    set(SDL_STATIC OFF CACHE BOOL "Build SDL3 static library" FORCE)
    set(SDL_TESTS OFF CACHE BOOL "Build SDL3 tests" FORCE)
    set(SDL_TEST_LIBRARY OFF CACHE BOOL "Build SDL3 test library" FORCE)
    add_subdirectory("${OPENW3D_SDL3_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/_deps/sdl3-build" EXCLUDE_FROM_ALL)
else()
    # DXVK/platform-shell paths need a shared SDL3 library plus headers.
    find_package(SDL3 REQUIRED COMPONENTS SDL3-shared Headers)
endif()
