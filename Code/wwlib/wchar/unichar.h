/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 OpenW3D Contributors.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, ors
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#ifndef UNICHAR_H
#define UNICHAR_H

#include <wchar.h>
#include <wctype.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#endif

typedef wchar_t unichar_t;
#define u_strlen(x) wcslen(x)
#define u_strcpy(x, y) wcscpy(x, y)
#define u_strncpy(x, y, z) wcsncpy(x, y, z)
#define u_strcat(x, y) wcscat(x, y)
#define u_vsnprintf_u(w, x, y, z) vswprintf(w, x, y, z)
#define u_strcmp(x, y) wcscmp(x, y)
#define u_strncmp(x, y, z) wcsncmp(x, y, z)
#if defined(_WIN32)
#define u_strcasecmp(x, y, z) _wcsicmp(x, y)
#define u_strncasecmp(x, y, z, w) wcsnicmp(x, y, z)
#else
#define u_strcasecmp(x, y, z) wcscasecmp(x, y)
#define u_strncasecmp(x, y, z, w) wcsncasecmp(x, y, z)
#endif
#define u_strpbrk(x, y) wcspbrk(x, y)
#define u_isspace(x) iswspace(x)
#define u_tolower(x) towlower(x)
#define u_strchr(x, y) wcschr(x, y)
#define u_strrchr(x, y) wcsrchr(x, y)
#define u_strstr(x, y) wcsstr(x, y)
#define u_snprintf_u(x, y, z, ...) swprintf(x, y, z, ##__VA_ARGS__)
#define u_sscanf_u(x, y, ...) swscanf(x, y, ##__VA_ARGS__)
#define u_vsnprintf_n(w, x, y, z) vsnprintf(w, x, y, z)

#define U_COMPARE_CODE_POINT_ORDER 0x8000
#define U_CHAR(str) (L##str)

inline size_t u_mbtows(unichar_t* dst, const char* src, size_t len)
{
#if defined(_WIN32)
	int retval = MultiByteToWideChar(CP_UTF8, 0, src, -1, dst, len);
	if (retval <= 0) {
		return size_t(-1);
	}
	return size_t(retval);
#else
	// Use mbsrtowcs for UTF-8 conversion on Linux/macOS
	mbstate_t state;
	memset(&state, 0, sizeof(state));
	size_t retval = mbsrtowcs(dst, &src, len, &state);
	if (retval == (size_t)-1) {
		return size_t(-1);
	}
	return retval + 1; // include null terminator like Windows version
#endif
}

inline size_t u_wstomb(char* dst, const unichar_t* src, size_t len)
{
#if defined(_WIN32)
	int retval = WideCharToMultiByte(CP_UTF8, 0, src, -1, dst, len, nullptr, nullptr);
	if (retval <= 0) {
		return size_t(-1);
	}
	return size_t(retval);
#else
	// Use wcsrtombs for UTF-8 conversion on Linux/macOS
	mbstate_t state;
	memset(&state, 0, sizeof(state));
	size_t retval = wcsrtombs(dst, &src, len, &state);
	if (retval == (size_t)-1) {
		return size_t(-1);
	}
	return retval + 1; // include null terminator like Windows version
#endif
}

#endif // UNICHAR_H
