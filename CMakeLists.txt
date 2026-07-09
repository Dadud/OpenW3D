cmake_minimum_required(VERSION 3.25...4.0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

include(FeatureSummary)
include(CMakeDependentOption)

# We don't support in tree builds, so help people make the right choice.
if (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
    message(FATAL_ERROR "Building in-source is not supported! Create a build dir and remove ${CMAKE_SOURCE_DIR}/CMakeCache.txt")
endif()

project(commando LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# This file handles extra settings wanted/needed for different compilers.
include(compilers)

# This file allows embedding resources
include(embed)

# Set where the build results will end up
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}" CACHE PATH "Where to output archives")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}" CACHE PATH "Where to output libraries")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}" CACHE PATH "Where to output binaries")

if(ANDROID OR OPENW3D_ANDROID)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

option(W3D_CLIENT "Build full game client." ON)
add_feature_info(OpenW3DClient W3D_CLIENT "Build OpenW3D game client")

option(W3D_FDS "Build as headless server." ON)
add_feature_info(OpenW3DHeadless W3D_FDS "Build OpenW3D headless server")

# Do we want to build extra SDK stuff or just the game binary?
option(W3D_TOOLS "Build additional tools." ON)
add_feature_info(OpenW3DTools W3D_TOOLS "Build OpenW3D Mod Tools")

option(W3D_PLATFORM_SHELL "Build a minimal SDL3 platform/window shell." OFF)
add_feature_info(OpenW3DPlatformShell W3D_PLATFORM_SHELL "Build minimal SDL3 platform shell")

option(W3D_RENDERER_SMOKE "Build a minimal SDL3 + renderer link/runtime smoke executable." OFF)
add_feature_info(OpenW3DRendererSmoke W3D_RENDERER_SMOKE "Build minimal renderer smoke executable")

option(W3D_MODERN_RUNTIME "Build the modern cross-platform runtime foundation." OFF)
add_feature_info(OpenW3DModernRuntime W3D_MODERN_RUNTIME "Build modern cross-platform runtime")

option(SYNTAX_CHECK_ONLY "Syntax check the files only?" OFF)
add_feature_info(SyntaxCheck SYNTAX_CHECK_ONLY "Syntax check files only")
if(SYNTAX_CHECK_ONLY)
    add_compile_options("-fsyntax-only")
endif(SYNTAX_CHECK_ONLY)

set(W3D_RENDERER_DEFAULT "NATIVE")
if(OPENW3D_ANDROID)
    set(W3D_RENDERER_DEFAULT "NULL")
endif()
set(W3D_RENDERER "${W3D_RENDERER_DEFAULT}" CACHE STRING "Renderer backend: NATIVE, DXVK, or NULL")
set_property(CACHE W3D_RENDERER PROPERTY STRINGS NATIVE DXVK NULL)
if(NOT W3D_RENDERER MATCHES "^(NATIVE|DXVK|NULL)$")
    message(FATAL_ERROR "W3D_RENDERER must be NATIVE, DXVK, or NULL (got '${W3D_RENDERER}')")
endif()
add_feature_info(RendererBackend W3D_RENDERER "Renderer backend selected for this build")

# Android builds get their own defaults via the toolchain file
if(OPENW3D_ANDROID)
    option(W3D_BUILD_OPTION_SDL3 "Build OpenW3D with SDL3." OFF)
    option(W3D_BUILD_OPTION_WEBBROWSER "Build OpenW3D with webbrowser." OFF)
    option(W3D_BUILD_OPTION_FFMPEG "Build with ffmpeg." OFF)
    option(W3D_BUILD_OPTION_OPENAL "Build with openal." OFF)
    option(W3D_BUILD_OPTION_FREETYPE "Build with freetype." OFF)
else()
    # Do we want to build with SDL3 for windows and inputs?
    cmake_dependent_option(W3D_BUILD_OPTION_SDL3 "Build OpenW3D with SDL3." OFF WIN32 ON)
    # Do we want to build with webbrowser?
    cmake_dependent_option(W3D_BUILD_OPTION_WEBBROWSER "Build OpenW3D with webbrowser." ON WIN32 OFF)
    # Do we want to build with ffmpeg for audio/video decoding?
    cmake_dependent_option(W3D_BUILD_OPTION_FFMPEG "Build with ffmpeg." OFF WIN32 ON)
    # Do we want to build with OpenAL/ffmpeg for audio playback?
    cmake_dependent_option(W3D_BUILD_OPTION_OPENAL "Build with openal." ON W3D_BUILD_OPTION_FFMPEG OFF)
    # Do we want to build with freetype and fontconfig for font rendering?
    cmake_dependent_option(W3D_BUILD_OPTION_FREETYPE "Build with freetype." OFF WIN32 ON)
endif()

if(W3D_RENDERER STREQUAL "NULL")
    if(NOT W3D_PLATFORM_SHELL AND NOT W3D_MODERN_RUNTIME)
        set(W3D_BUILD_OPTION_SDL3 OFF CACHE BOOL "Build OpenW3D with SDL3." FORCE)
    endif()
    set(W3D_BUILD_OPTION_WEBBROWSER OFF CACHE BOOL "Build OpenW3D with webbrowser." FORCE)
    set(W3D_BUILD_OPTION_FFMPEG OFF CACHE BOOL "Build with ffmpeg." FORCE)
    set(W3D_BUILD_OPTION_OPENAL OFF CACHE BOOL "Build with openal." FORCE)
    set(W3D_BUILD_OPTION_FREETYPE OFF CACHE BOOL "Build with freetype." FORCE)
endif()

add_feature_info(SDL3Build W3D_BUILD_OPTION_SDL3 "Build OpenW3D with SDL3")
add_feature_info(BrowserBuild W3D_BUILD_OPTION_WEBBROWSER "Build OpenW3D with webbrowser")
add_feature_info(FFMpegBuild W3D_BUILD_OPTION_FFMPEG "Build OpenW3D with FFMpeg")
add_feature_info(OpenALBuild W3D_BUILD_OPTION_OPENAL "Build OpenW3D with OpenAL")
add_feature_info(FreetypeBuild W3D_BUILD_OPTION_FREETYPE "Build OpenW3D with freetype")

option(W3D_BUILD_QT_TOOLS "Build Qt-based GUI tools." OFF)
add_feature_info(QtTools W3D_BUILD_QT_TOOLS "Build Qt front-end tools")

if(NOT WIN32)
    add_compile_definitions(__cdecl=)
    add_compile_definitions(__stdcall=)
    add_compile_definitions(_stdcall=)
    add_compile_definitions(stricmp=strcasecmp)
    add_compile_definitions(strnicmp=strncasecmp)
    add_compile_definitions(wcsnicmp=wcsncasecmp)
    add_compile_definitions(wcsicmp=wcscasecmp)
endif()

include(FetchContent)

if(W3D_BUILD_OPTION_FFMPEG)
    include(ffmpeg)
endif()

if(W3D_BUILD_OPTION_SDL3)
    include(sdl3)
    add_compile_definitions(-DOPENW3D_SDL3=1)
elseif(WIN32)
    add_compile_definitions(-DOPENW3D_WIN32=1)
elseif(OPENW3D_ANDROID OR W3D_RENDERER STREQUAL "NULL")
    # Core/null builds do not need a windowing backend yet; use POSIX compat mode.
    add_compile_definitions(-DOPENW3D_POSIX=1)
else()
    message(FATAL_ERROR "Invalid platform backend: enable SDL3, build on WIN32, or use W3D_RENDERER=NULL for core-only POSIX bring-up")
endif()

add_compile_definitions(-DWEBBROWSER_ENABLED=$<BOOL:${W3D_BUILD_OPTION_WEBBROWSER}>)

if(W3D_BUILD_OPTION_OPENAL)
    include(openal)
endif()

if(W3D_BUILD_OPTION_FREETYPE)
    include(freetype)
endif()

if(W3D_BUILD_QT_TOOLS)
    if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
        message(FATAL_ERROR "Qt tooling is only supported for 64-bit builds. Configure with a 64-bit generator or set W3D_BUILD_QT_TOOLS=OFF.")
    endif()
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)
    include(qt_tools)
endif()

if(WIN32 AND NOT W3D_RENDERER STREQUAL "NULL")
    # Do we want to build with bink for video decoding?
    cmake_dependent_option(W3D_BUILD_OPTION_BINK "Build with bink." ON NOT W3D_BUILD_OPTION_FFMPEG OFF)
    add_feature_info(BinkBuild W3D_BUILD_OPTION_BINK "Build OpenW3D with Bink")

    if(W3D_BUILD_OPTION_BINK)
        include(bink)
        target_compile_definitions(binkstub INTERFACE W3D_HAS_BINK)
    endif()

    # Do we want to build with miles for audio playback?
    cmake_dependent_option(W3D_BUILD_OPTION_MILES "Build with miles." ON NOT W3D_BUILD_OPTION_OPENAL OFF)
    add_feature_info(MilesBuild W3D_BUILD_OPTION_MILES "Build OpenW3D with Miles Audio")

    if(W3D_BUILD_OPTION_MILES)
        include(miles)
    endif()

    option(W3D_BUILD_OPTION_ICU "Build with ICU." OFF)
    add_feature_info(ICUBuild W3D_BUILD_OPTION_ICU "Build OpenW3D with ICU")

    if(W3D_BUILD_OPTION_ICU)
        include(icu)
    endif()
elseif(NOT W3D_RENDERER STREQUAL "NULL")
    if(NOT OPENW3D_ANDROID)
        set(W3D_BUILD_OPTION_ICU ON)
        include(icu)
    endif()
endif()

if(NOT W3D_RENDERER STREQUAL "NULL")
    include(dx9)
endif()

option(W3D_BUILD_OPTION_GAMESPY "Build GameSpy online services." ON)
add_feature_info(GameSpyBuild W3D_BUILD_OPTION_GAMESPY "Build GameSpy online services")

if(W3D_BUILD_OPTION_GAMESPY AND NOT OPENW3D_ANDROID AND NOT W3D_RENDERER STREQUAL "NULL")
    include(gamespy)
endif()

if(W3D_MODERN_RUNTIME)
    if(NOT W3D_BUILD_OPTION_SDL3)
        message(FATAL_ERROR "W3D_MODERN_RUNTIME requires W3D_BUILD_OPTION_SDL3=ON")
    endif()
    add_subdirectory(Modern)
endif()

# Add main build targets
add_subdirectory(Code)

feature_summary(WHAT ENABLED_FEATURES DESCRIPTION "Enabled features:")
feature_summary(WHAT DISABLED_FEATURES DESCRIPTION "Disabled features:")
