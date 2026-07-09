/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 OpenW3D Contributors.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
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

/*
** Stub for the Windows-only message loop helpers. Mobile/desktop POSIX
** builds use SDL3 (or similar) to drive the event loop, so these entry
** points are no-ops on those platforms.
*/

#include "msgloop.h"

#if !defined(OPENW3D_PLATFORM_WINDOWS)

void Windows_Message_Handler(void)
{
	// On non-Windows platforms the host event loop is provided by the
	// active windowing toolkit (SDL3 today). This stub is intentionally
	// empty so the engine can compile without dragging in HWND/HACCEL/MSG.
}

void Add_Modeless_Dialog(HWND) {}
void Remove_Modeless_Dialog(HWND) {}
void Add_Accelerator(HWND, HACCEL) {}
void Remove_Accelerator(HACCEL) {}

bool (*Message_Intercept_Handler)(MSG &) = nullptr;

#endif
