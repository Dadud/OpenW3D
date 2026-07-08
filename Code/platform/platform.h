#ifndef OPENW3D_PLATFORM_H
#define OPENW3D_PLATFORM_H

/*
 * Central OpenW3D platform detection.
 *
 * Keep OS identity here. Backend choices such as OPENW3D_SDL3, OPENW3D_WIN32,
 * W3D_HAS_FFMPEG, etc. should describe implementation choices, not the host OS.
 */

#if defined(_WIN32) || defined(_WIN64)
#  define OPENW3D_PLATFORM_WINDOWS 1
#  define OPENW3D_PLATFORM_NAME "Windows"
#elif defined(__ANDROID__)
#  define OPENW3D_PLATFORM_ANDROID 1
#  define OPENW3D_PLATFORM_POSIX 1
#  define OPENW3D_PLATFORM_NAME "Android"
#elif defined(__APPLE__)
#  include <TargetConditionals.h>
#  define OPENW3D_PLATFORM_APPLE 1
#  define OPENW3D_PLATFORM_POSIX 1
#  if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#    define OPENW3D_PLATFORM_IOS 1
#    define OPENW3D_PLATFORM_NAME "iOS"
#  else
#    define OPENW3D_PLATFORM_MACOS 1
#    define OPENW3D_PLATFORM_NAME "macOS"
#  endif
#elif defined(__EMSCRIPTEN__)
#  define OPENW3D_PLATFORM_WEB 1
#  define OPENW3D_PLATFORM_POSIX 1
#  define OPENW3D_PLATFORM_NAME "Emscripten"
#elif defined(__linux__)
#  define OPENW3D_PLATFORM_LINUX 1
#  define OPENW3D_PLATFORM_POSIX 1
#  define OPENW3D_PLATFORM_NAME "Linux"
#elif defined(__unix__) || defined(__unix)
#  define OPENW3D_PLATFORM_UNIX 1
#  define OPENW3D_PLATFORM_POSIX 1
#  define OPENW3D_PLATFORM_NAME "Unix"
#else
#  define OPENW3D_PLATFORM_UNKNOWN 1
#  define OPENW3D_PLATFORM_NAME "Unknown"
#endif

/* Transitional aliases for existing code. Prefer the OPENW3D_PLATFORM_* names
 * in new code, but these keep current branches readable while we migrate.
 */
#if defined(OPENW3D_PLATFORM_WINDOWS) && !defined(OPENW3D_WIN32)
#  define OPENW3D_WIN32 1
#endif

#if defined(OPENW3D_PLATFORM_ANDROID) && !defined(OPENW3D_ANDROID)
#  define OPENW3D_ANDROID 1
#endif

#if defined(OPENW3D_PLATFORM_POSIX) && !defined(OPENW3D_POSIX)
#  define OPENW3D_POSIX 1
#endif

#endif /* OPENW3D_PLATFORM_H */
