# OpenW3D Android cross-compile toolchain
# Usage: cmake -DCMAKE_TOOLCHAIN_FILE=cmake/android-aarch64.cmake ..
#
# Sets up aarch64-linux-android compilation using NDK clang directly.
# Targets API 21 (Android 5.0+).

set(CMAKE_SYSTEM_NAME Android)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# NDK paths
set(ANDROID_NDK_ROOT "C:/Users/Dadud/android-dev/sdk/ndk/27.2.12479018")
set(NDK_TOOLCHAIN_PREFIX "${ANDROID_NDK_ROOT}/toolchains/llvm/prebuilt/windows-x86_64")

set(ANDROID_ABI "arm64-v8a")
set(ANDROID_PLATFORM "android-21")
set(ANDROID_STL "c++_shared")

# NDK CMake integration
set(CMAKE_ANDROID_NDK ${ANDROID_NDK_ROOT})
set(CMAKE_ANDROID_ARCH_ABI ${ANDROID_ABI})
set(CMAKE_ANDROID_API 21)
set(CMAKE_ANDROID_STL_TYPE ${ANDROID_STL})

# Use bare clang.exe with --target flag (avoids .cmd wrapper issues in MSYS)
set(AARCH64_TRIPLE "aarch64-linux-android21")
set(CMAKE_C_COMPILER   "${NDK_TOOLCHAIN_PREFIX}/bin/clang.exe")
set(CMAKE_CXX_COMPILER "${NDK_TOOLCHAIN_PREFIX}/bin/clang++.exe")
set(CMAKE_AR           "${NDK_TOOLCHAIN_PREFIX}/bin/llvm-ar.exe" CACHE FILEPATH "Archiver")
set(CMAKE_RANLIB       "${NDK_TOOLCHAIN_PREFIX}/bin/llvm-ranlib.exe" CACHE FILEPATH "Ranlib")

# Target flags
set(AARCH64_FLAGS "--target=${AARCH64_TRIPLE} -march=armv8-a")
set(CMAKE_C_FLAGS_INIT   "${AARCH64_FLAGS}")
set(CMAKE_CXX_FLAGS_INIT "${AARCH64_FLAGS}")

# Sysroot
set(CMAKE_SYSROOT "${NDK_TOOLCHAIN_PREFIX}/sysroot")
set(CMAKE_FIND_ROOT_PATH "${NDK_TOOLCHAIN_PREFIX}/sysroot")

# Search behavior
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Force the linker to use NDK's lld
set(CMAKE_EXE_LINKER_FLAGS_INIT "--target=${AARCH64_TRIPLE} -fuse-ld=lld")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "--target=${AARCH64_TRIPLE} -fuse-ld=lld")
set(CMAKE_STATIC_LINKER_FLAGS_INIT "")

# OpenW3D build options for Android (command-line -D will override these)
# All OFF for first-pass — no external deps yet
set(W3D_BUILD_OPTION_SDL3 OFF CACHE BOOL "" FORCE)
set(W3D_BUILD_OPTION_FFMPEG OFF CACHE BOOL "" FORCE)
set(W3D_BUILD_OPTION_OPENAL OFF CACHE BOOL "" FORCE)
set(W3D_BUILD_OPTION_BINK OFF CACHE BOOL "" FORCE)
set(W3D_BUILD_OPTION_WEBBROWSER OFF CACHE BOOL "" FORCE)
set(W3D_BUILD_OPTION_FREETYPE OFF CACHE BOOL "" FORCE)
set(W3D_BUILD_QT_TOOLS OFF CACHE BOOL "" FORCE)
set(OPENW3D_ANDROID ON CACHE BOOL "Building for Android" FORCE)
