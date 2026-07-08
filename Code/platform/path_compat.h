#ifndef OPENW3D_PLATFORM_PATH_COMPAT_H
#define OPENW3D_PLATFORM_PATH_COMPAT_H

/*
 * Portable path helpers for code that historically relied on Win32 path APIs
 * like _splitpath, _MAX_DRIVE, _MAX_DIR, _MAX_FNAME, _MAX_EXT.
 *
 * Modeled after xray-16's PlatformLinux.inl / PlatformApple.inl helpers and
 * the Xray-16 cross-platform portability playbook.
 */

#include "platform.h"

#include <cstddef>
#include <cstring>
#include <string>

#if defined(OPENW3D_PLATFORM_WINDOWS)
#include <stdlib.h>
#include <string.h>
#endif

namespace openw3d {
namespace path_compat {

#if !defined(OPENW3D_PLATFORM_WINDOWS)
constexpr std::size_t kMaxDrive = 3;
constexpr std::size_t kMaxDir = 256;
constexpr std::size_t kMaxFname = 256;
constexpr std::size_t kMaxExt = 256;
#else
constexpr std::size_t kMaxDrive = _MAX_DRIVE;
constexpr std::size_t kMaxDir = _MAX_DIR;
constexpr std::size_t kMaxFname = _MAX_FNAME;
constexpr std::size_t kMaxExt = _MAX_EXT;
#endif

inline char path_separator()
{
#if defined(OPENW3D_PLATFORM_WINDOWS)
    return '\\';
#else
    return '/';
#endif
}

inline void convert_to_native(char *path)
{
#if defined(OPENW3D_PLATFORM_WINDOWS)
    for (char *p = path; *p; ++p) {
        if (*p == '/') {
            *p = '\\';
        }
    }
#else
    (void)path;
#endif
}

inline void convert_to_posix(char *path)
{
#if defined(OPENW3D_PLATFORM_WINDOWS)
    for (char *p = path; *p; ++p) {
        if (*p == '\\') {
            *p = '/';
        }
    }
#else
    (void)path;
#endif
}

inline bool is_absolute(const char *path)
{
    if (path == nullptr || *path == '\0') {
        return false;
    }
#if defined(OPENW3D_PLATFORM_WINDOWS)
    if ((path[0] && path[1] == ':') && (path[2] == '\\' || path[2] == '/' || path[2] == '\0')) {
        return true;
    }
    return path[0] == '\\' || path[0] == '/';
#else
    return path[0] == '/';
#endif
}

inline const char *path_separator_str()
{
#if defined(OPENW3D_PLATFORM_WINDOWS)
    return "\\";
#else
    return "/";
#endif
}

/*
 * Drop-in replacement for _splitpath. Behavior intentionally matches the Win32
 * CRT behavior at a coarse level: it pulls the drive letter, directory, file
 * name, and extension out of a path. Any of the output pointers may be null.
 */
inline void split_path(
    const char *path,
    char *drive,
    char *dir,
    char *fname,
    char *ext)
{
    if (drive) drive[0] = '\0';
    if (dir)   dir[0]   = '\0';
    if (fname) fname[0] = '\0';
    if (ext)   ext[0]   = '\0';

    if (path == nullptr) {
        return;
    }

#if defined(OPENW3D_PLATFORM_WINDOWS)
    _splitpath(path, drive, dir, fname, ext);
#else
    const char *cursor = path;

    if (cursor[0] && cursor[1] == ':') {
        if (drive) {
            drive[0] = cursor[0];
            drive[1] = ':';
            drive[2] = '\0';
        }
        cursor += 2;
    }

    const char *end = cursor + std::strlen(cursor);
    const char *last_sep = nullptr;
    for (const char *p = cursor; p < end; ++p) {
        if (*p == '/' || *p == '\\') {
            last_sep = p;
        }
    }

    if (dir) {
        if (last_sep != nullptr) {
            std::size_t n = static_cast<std::size_t>(last_sep - cursor) + 1;
            if (n >= kMaxDir) n = kMaxDir - 1;
            std::memcpy(dir, cursor, n);
            dir[n] = '\0';
        } else {
            dir[0] = '\0';
        }
    }

    const char *name_start = (last_sep != nullptr) ? last_sep + 1 : cursor;
    const char *dot = nullptr;
    for (const char *p = name_start; p < end; ++p) {
        if (*p == '.') {
            dot = p;
        }
    }

    if (fname) {
        const char *fname_end = (dot != nullptr) ? dot : end;
        std::size_t n = static_cast<std::size_t>(fname_end - name_start);
        if (n >= kMaxFname) n = kMaxFname - 1;
        std::memcpy(fname, name_start, n);
        fname[n] = '\0';
    }

    if (ext && dot != nullptr) {
        std::size_t n = static_cast<std::size_t>(end - dot);
        if (n >= kMaxExt) n = kMaxExt - 1;
        std::memcpy(ext, dot, n);
        ext[n] = '\0';
    }
#endif
}

inline bool delete_file(const char *path)
{
    if (path == nullptr) {
        return false;
    }
#if defined(OPENW3D_PLATFORM_WINDOWS)
    return ::DeleteFileA(path) != 0;
#else
    return ::unlink(path) == 0;
#endif
}

inline bool move_file(const char *from, const char *to)
{
    if (from == nullptr || to == nullptr) {
        return false;
    }
#if defined(OPENW3D_PLATFORM_WINDOWS)
    return ::MoveFileA(from, to) != 0;
#else
    return std::rename(from, to) == 0;
#endif
}

} // namespace path_compat
} // namespace openw3d

#endif /* OPENW3D_PLATFORM_PATH_COMPAT_H */
