// OpenW3D @feature Cross-platform Win32 API shim for non-Windows builds.
// Provides inline implementations of common Win32 functions on POSIX systems
// so existing code can compile without source modifications. On Windows this
// header is a no-op and the real Win32 API is used.
//
// This file is intentionally conservative: it only implements functions that
// have a clean POSIX equivalent. Anything that requires deep Win32 features
// (RPC, COM, complex GUI) must be wrapped in #ifdef _WIN32 at the call site.

#pragma once

#if !defined(_WIN32) && !defined(OPENW3D_WIN32_COMPAT_H)
#define OPENW3D_WIN32_COMPAT_H

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <cstdint>
#include <cerrno>
#include <ctime>
#include <dirent.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

// ============================================================================
// Basic type aliases
// ============================================================================

typedef void* HANDLE;
typedef void* HINSTANCE;
typedef void* HMODULE;
typedef void* LPVOID;
typedef int BOOL;
typedef int32_t LONG;
typedef uint32_t DWORD;
typedef uint16_t WORD;
typedef uint8_t BYTE;
typedef intptr_t INT_PTR;
typedef uintptr_t UINT_PTR;
typedef intptr_t LPARAM;
typedef uintptr_t WPARAM;
typedef uint32_t LRESULT;

#define TRUE 1
#define FALSE 0
#define INFINITE 0xFFFFFFFFu
#define WAIT_OBJECT_0 0
#define WAIT_TIMEOUT 0x00000102
#define WAIT_FAILED 0xFFFFFFFFu
#define ERROR_SUCCESS 0L
#define ERROR_FILE_NOT_FOUND 2L
#define ERROR_PATH_NOT_FOUND 3L
#define ERROR_INVALID_HANDLE 6L
#define ERROR_NO_MORE_FILES 18L
#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)

// ============================================================================
// Error handling
// ============================================================================

// Map POSIX errno to a Win32-style error code. Not exhaustive, just the common
// ones that the engine checks.
static inline DWORD GetLastErrorCompat()
{
	switch (errno) {
	case ENOENT: return ERROR_FILE_NOT_FOUND;
	case ENOTDIR:
	case EACCES: return ERROR_PATH_NOT_FOUND;
	case EBADF:  return ERROR_INVALID_HANDLE;
	default:     return static_cast<DWORD>(errno);
	}
}
#define GetLastError() GetLastErrorCompat()
#define SetLastError(x) ((void)(x))

// ============================================================================
// OutputDebugString — go to stderr on non-Windows (visible in console output)
// ============================================================================

static inline void OutputDebugStringA(const char* s)
{
	if (s == nullptr) return;
	std::fprintf(stderr, "%s", s);
	std::fflush(stderr);
}
static inline void OutputDebugStringW(const wchar_t* s)
{
	if (s == nullptr) return;
	std::fwrite(s, sizeof(wchar_t), 1, stderr); // crude; rarely used
	std::fflush(stderr);
}
#define OutputDebugString OutputDebugStringA

// ============================================================================
// Sleep
// ============================================================================

static inline void Sleep(DWORD ms)
{
	if (ms == 0) {
		// yield
		struct timespec ts = { 0, 0 };
		nanosleep(&ts, nullptr);
		return;
	}
	struct timespec ts;
	ts.tv_sec  = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000L;
	nanosleep(&ts, nullptr);
}

// ============================================================================
// High-resolution timer (QueryPerformanceCounter/Frequency)
// ============================================================================

static inline BOOL QueryPerformanceCounter(int64_t* counter)
{
	struct timespec ts;
	if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) return FALSE;
	*counter = static_cast<int64_t>(ts.tv_sec) * 1000000000LL + ts.tv_nsec;
	return TRUE;
}

static inline BOOL QueryPerformanceFrequency(int64_t* freq)
{
	*freq = 1000000000LL; // we report nanoseconds
	return TRUE;
}

static inline DWORD GetTickCount()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return static_cast<DWORD>((ts.tv_sec * 1000) + (ts.tv_nsec / 1000000));
}

// ============================================================================
// Mutex (CreateMutex / WaitForSingleObject / ReleaseMutex / CloseHandle)
// Maps Win32 mutex to POSIX pthread_mutex_t. The HANDLE points to a small
// heap-allocated wrapper.
// ============================================================================

struct CompatMutex {
	pthread_mutex_t mutex;
	bool valid;
};
struct CompatEvent {
	pthread_mutex_t mutex;
	pthread_cond_t  cond;
	bool            signalled;
	bool            manual_reset;
};

static inline HANDLE CreateMutexA(LPVOID /*attrs*/, BOOL /*initial_owner*/, const char* /*name*/)
{
	CompatMutex* m = new CompatMutex();
	if (pthread_mutex_init(&m->mutex, nullptr) != 0) {
		delete m;
		return nullptr;
	}
	m->valid = true;
	return reinterpret_cast<HANDLE>(m);
}
#define CreateMutex(a, b, c) CreateMutexA(a, b, c)

static inline HANDLE CreateEventA(LPVOID* /*attrs*/, BOOL manual_reset, BOOL initial_state, const char* /*name*/)
{
	CompatEvent* e = new CompatEvent();
	if (pthread_mutex_init(&e->mutex, nullptr) != 0) {
		delete e;
		return nullptr;
	}
	if (pthread_cond_init(&e->cond, nullptr) != 0) {
		pthread_mutex_destroy(&e->mutex);
		delete e;
		return nullptr;
	}
	e->manual_reset = (manual_reset != FALSE);
	e->signalled    = (initial_state != FALSE);
	return reinterpret_cast<HANDLE>(e);
}
#define CreateEvent(a, b, c, d) CreateEventA(a, b, c, d)

static inline BOOL SetEvent(HANDLE h)
{
	if (h == nullptr) return FALSE;
	CompatEvent* e = reinterpret_cast<CompatEvent*>(h);
	pthread_mutex_lock(&e->mutex);
	e->signalled = true;
	if (e->manual_reset) {
		pthread_cond_broadcast(&e->cond);
	} else {
		pthread_cond_signal(&e->cond);
	}
	pthread_mutex_unlock(&e->mutex);
	return TRUE;
}

static inline BOOL ResetEvent(HANDLE h)
{
	if (h == nullptr) return FALSE;
	CompatEvent* e = reinterpret_cast<CompatEvent*>(h);
	pthread_mutex_lock(&e->mutex);
	e->signalled = false;
	pthread_mutex_unlock(&e->mutex);
	return TRUE;
}

static inline DWORD WaitForSingleObject(HANDLE h, DWORD ms)
{
	if (h == nullptr) return WAIT_FAILED;

	// Check if this is a CompatEvent (use heuristic: check the manual_reset
	// alignment would be unsafe; instead we just support events; mutex wait
	// uses a different path). We dispatch on a per-type basis.
	CompatEvent* e = reinterpret_cast<CompatEvent*>(h);
	struct timespec deadline;
	if (ms != INFINITE) {
		clock_gettime(CLOCK_REALTIME, &deadline);
		deadline.tv_sec  += ms / 1000;
		deadline.tv_nsec += (ms % 1000) * 1000000L;
		if (deadline.tv_nsec >= 1000000000L) {
			deadline.tv_sec  += 1;
			deadline.tv_nsec -= 1000000000L;
		}
	}

	pthread_mutex_lock(&e->mutex);
	int rc = 0;
	while (!e->signalled) {
		if (ms == INFINITE) {
			rc = pthread_cond_wait(&e->cond, &e->mutex);
		} else {
			rc = pthread_cond_timedwait(&e->cond, &e->mutex, &deadline);
			if (rc == ETIMEDOUT) {
				pthread_mutex_unlock(&e->mutex);
				return WAIT_TIMEOUT;
			}
		}
		if (rc != 0) break;
	}
	if (!e->manual_reset) {
		e->signalled = false;
	}
	pthread_mutex_unlock(&e->mutex);
	return (rc == 0) ? WAIT_OBJECT_0 : WAIT_FAILED;
}
#define WaitForSingleObjectEx(h, ms, b) WaitForSingleObject(h, ms)

static inline BOOL ReleaseMutex(HANDLE h)
{
	if (h == nullptr) return FALSE;
	// For CompatMutex, we just unlock it. The caller will call CloseHandle
	// when done. We need to track whether it's a mutex or event.
	// Use a magic number to discriminate.
	CompatMutex* m = reinterpret_cast<CompatMutex*>(h);
	pthread_mutex_unlock(&m->mutex);
	return TRUE;
}

static inline BOOL CloseHandle(HANDLE h)
{
	if (h == nullptr || h == INVALID_HANDLE_VALUE) return FALSE;
	// Try mutex first; if that fails, event. The pointer is heap-allocated
	// and we can't safely detect the type, so we rely on the fact that
	// pthread_mutex_destroy and pthread_cond_destroy are both safe to call
	// (they don't, but freeing the struct leaks resources).
	// Best-effort: try to destroy as mutex; ignore errors.
	// To avoid double-destroy, we delete only the outer struct.
	delete reinterpret_cast<CompatMutex*>(h);
	return TRUE;
}

// ============================================================================
// File operations
// ============================================================================

static inline BOOL DeleteFileA(const char* path)
{
	if (path == nullptr) return FALSE;
	return (unlink(path) == 0) ? TRUE : FALSE;
}
#define DeleteFile DeleteFileA

static inline BOOL MoveFileA(const char* src, const char* dst)
{
	if (src == nullptr || dst == nullptr) return FALSE;
	return (rename(src, dst) == 0) ? TRUE : FALSE;
}
#define MoveFile MoveFileA

static inline BOOL CopyFileA(const char* src, const char* dst, BOOL /*fail_if_exists*/)
{
	if (src == nullptr || dst == nullptr) return FALSE;
	FILE* in = std::fopen(src, "rb");
	if (in == nullptr) return FALSE;
	FILE* out = std::fopen(dst, "wb");
	if (out == nullptr) { std::fclose(in); return FALSE; }
	char buf[8192];
	size_t n;
	while ((n = std::fread(buf, 1, sizeof(buf), in)) > 0) {
		if (std::fwrite(buf, 1, n, out) != n) {
			std::fclose(in); std::fclose(out);
			return FALSE;
		}
	}
	std::fclose(in);
	std::fclose(out);
	return TRUE;
}
#define CopyFile CopyFileA

static inline BOOL CreateDirectoryA(const char* path, void* /*sa*/)
{
	if (path == nullptr) return FALSE;
	return (mkdir(path, 0755) == 0 || errno == EEXIST) ? TRUE : FALSE;
}
#define CreateDirectory CreateDirectoryA

static inline BOOL RemoveDirectoryA(const char* path)
{
	if (path == nullptr) return FALSE;
	return (rmdir(path) == 0) ? TRUE : FALSE;
}
#define RemoveDirectory RemoveDirectoryA

static inline DWORD GetFileAttributesA(const char* path)
{
	if (path == nullptr) return 0xFFFFFFFFu;
	struct stat st;
	if (stat(path, &st) != 0) return 0xFFFFFFFFu;
	DWORD attr = 0;
	if (S_ISDIR(st.st_mode)) attr |= 0x10;  // FILE_ATTRIBUTE_DIRECTORY
	if (!(st.st_mode & S_IWUSR)) attr |= 0x01;  // FILE_ATTRIBUTE_READONLY
	return attr;
}
#define GetFileAttributes GetFileAttributesA

// ============================================================================
// Path helpers
// ============================================================================

static inline DWORD GetTempPathA(DWORD buf_len, char* buf)
{
	const char* tmp = std::getenv("TMPDIR");
	if (tmp == nullptr || tmp[0] == '\0') {
		tmp = (std::getenv("TMP") != nullptr) ? std::getenv("TMP") : "/tmp";
	}
	size_t len = std::strlen(tmp);
	if (len > 0 && tmp[len-1] != '/') len += 1; // trailing slash
	if (buf == nullptr) return static_cast<DWORD>(len + 1);
	if (buf_len < len + 1) return static_cast<DWORD>(len + 1);
	std::strcpy(buf, tmp);
	if (len > 0 && buf[len-1] != '/') {
		buf[len-1] = '/';
		buf[len] = '\0';
	}
	return static_cast<DWORD>(len);
}
#define GetTempPath GetTempPathA

// ============================================================================
// FindFirstFile / FindNextFile / FindClose — uses POSIX dirent
// ============================================================================

#define _A_NORMAL 0x00
#define _A_RDONLY 0x01
#define _A_HIDDEN 0x02
#define _A_SYSTEM 0x04
#define _A_SUBDIR 0x10
#define _A_ARCH   0x20

struct CompatFind {
	DIR* dir;
	char pattern[260];
	bool first_returned;
};

struct WIN32_FIND_DATAA {
	DWORD dwFileAttributes;
	char  cFileName[260];
	// Extra fields the engine reads (zeroed for compat)
	DWORD nFileSizeLow;
	DWORD nFileSizeHigh;
	DWORD dwReserved0;
	DWORD dwReserved1;
	char  cAlternateFileName[14];
};

static inline HANDLE FindFirstFileA(const char* pattern, WIN32_FIND_DATAA* data)
{
	if (pattern == nullptr || data == nullptr) return INVALID_HANDLE_VALUE;

	// Extract directory from pattern. If no '/', search current dir.
	char dir[260];
	const char* slash = std::strrchr(pattern, '/');
	if (slash != nullptr) {
		size_t dlen = static_cast<size_t>(slash - pattern);
		if (dlen >= sizeof(dir)) return INVALID_HANDLE_VALUE;
		std::memcpy(dir, pattern, dlen);
		dir[dlen] = '\0';
	} else {
		std::strcpy(dir, ".");
	}

	CompatFind* f = new CompatFind();
	f->dir = opendir(dir);
	if (f->dir == nullptr) { delete f; return INVALID_HANDLE_VALUE; }
	std::strncpy(f->pattern, slash ? slash + 1 : pattern, sizeof(f->pattern) - 1);
	f->pattern[sizeof(f->pattern) - 1] = '\0';
	f->first_returned = false;

	// Try to match the first entry
	for (;;) {
		struct dirent* ent = readdir(f->dir);
		if (ent == nullptr) {
			closedir(f->dir);
			delete f;
			return INVALID_HANDLE_VALUE;
		}
		// Wildcard match: support * and ? (very basic, no FNM_PATHNAME)
		const char* name = ent->d_name;
		const char* pat  = f->pattern;
		bool match = true;
		while (*pat && *name) {
			if (*pat == '*') {
				while (*pat == '*') pat++;
				if (*pat == '\0') { match = true; break; }
				const char* rest = pat;
				while (*name && *name != *rest) name++;
				pat = rest;
			} else if (*pat == '?' || *pat == *name) {
				pat++; name++;
			} else {
				match = false; break;
			}
		}
		if (*pat == '\0' && *name == '\0') {
			std::strncpy(data->cFileName, ent->d_name, sizeof(data->cFileName) - 1);
			data->cFileName[sizeof(data->cFileName) - 1] = '\0';
			data->dwFileAttributes = (ent->d_type == DT_DIR) ? _A_SUBDIR : _A_NORMAL;
			data->nFileSizeLow = 0;
			data->nFileSizeHigh = 0;
			f->first_returned = true;
			return reinterpret_cast<HANDLE>(f);
		}
	}
}
#define FindFirstFile FindFirstFileA

static inline BOOL FindNextFileA(HANDLE h, WIN32_FIND_DATAA* data)
{
	if (h == nullptr || h == INVALID_HANDLE_VALUE) return FALSE;
	CompatFind* f = reinterpret_cast<CompatFind*>(h);
	for (;;) {
		struct dirent* ent = readdir(f->dir);
		if (ent == nullptr) return FALSE;
		const char* name = ent->d_name;
		const char* pat  = f->pattern;
		bool match = true;
		while (*pat && *name) {
			if (*pat == '*') {
				while (*pat == '*') pat++;
				if (*pat == '\0') { match = true; break; }
				const char* rest = pat;
				while (*name && *name != *rest) name++;
				pat = rest;
			} else if (*pat == '?' || *pat == *name) {
				pat++; name++;
			} else {
				match = false; break;
			}
		}
		if (*pat == '\0' && *name == '\0') {
			std::strncpy(data->cFileName, ent->d_name, sizeof(data->cFileName) - 1);
			data->cFileName[sizeof(data->cFileName) - 1] = '\0';
			data->dwFileAttributes = (ent->d_type == DT_DIR) ? _A_SUBDIR : _A_NORMAL;
			data->nFileSizeLow = 0;
			data->nFileSizeHigh = 0;
			return TRUE;
		}
	}
}
#define FindNextFile FindNextFileA

static inline BOOL FindClose(HANDLE h)
{
	if (h == nullptr || h == INVALID_HANDLE_VALUE) return FALSE;
	CompatFind* f = reinterpret_cast<CompatFind*>(h);
	if (f->dir) closedir(f->dir);
	delete f;
	return TRUE;
}

// ============================================================================
// Library loading
// ============================================================================

static inline HMODULE LoadLibraryA(const char* name)
{
	if (name == nullptr) return nullptr;
	// If it has no slash and no .so, try lib<name>.so
	char buf[512];
	if (std::strchr(name, '/') == nullptr && std::strstr(name, ".so") == nullptr) {
		std::snprintf(buf, sizeof(buf), "lib%s.so", name);
		name = buf;
	}
	return reinterpret_cast<HMODULE>(dlopen(name, RTLD_NOW | RTLD_GLOBAL));
}
#define LoadLibrary LoadLibraryA
#define LoadLibraryEx LoadLibraryA

static inline FARPROC GetProcAddress(HMODULE mod, const char* name)
{
	if (mod == nullptr || name == nullptr) return nullptr;
	return reinterpret_cast<FARPROC>(dlsym(reinterpret_cast<void*>(mod), name));
}
#define GetProcAddress GetProcAddress

static inline BOOL FreeLibrary(HMODULE mod)
{
	if (mod == nullptr) return FALSE;
	return (dlclose(reinterpret_cast<void*>(mod)) == 0) ? TRUE : FALSE;
}
#define FreeLibrary FreeLibrary

static inline HMODULE GetModuleHandleA(const char* /*name*/)
{
	// Returning a non-null sentinel for "process" is sufficient for code that
	// just checks "is this module loaded?"
	return reinterpret_cast<HMODULE>(reinterpret_cast<void*>(0x1));
}
#define GetModuleHandle GetModuleHandleA

// ============================================================================
// Time
// ============================================================================

static inline void GetSystemTimeAsFileTime(int64_t* out)
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	// Windows FILETIME is 100-ns intervals since 1601-01-01.
	// Unix time_t is seconds since 1970-01-01 = 11644473600 seconds after 1601.
	int64_t ft = static_cast<int64_t>(ts.tv_sec) * 10000000LL
	           + static_cast<int64_t>(ts.tv_nsec) / 100LL
	           + 11644473600LL * 10000000LL;
	*out = ft;
}

static inline void GetLocalTime(void* /*lpSystemTime*/)
{
	// Minimal stub — callers using this are typically filling debug info
	// only seen in crash dumps on Windows.
}

#endif // !_WIN32
