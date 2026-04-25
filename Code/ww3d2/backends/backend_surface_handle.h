/*
**	Command & Conquer Renegade(tm)
**	Community contribution - Licensed under GPLv3
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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : ww3d                                                         *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * BackendSurfaceHandle - backend-private surface handle definition.                            *
 * Each backend implements this differently. Callers use the WW3DBackend methods.             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef BACKEND_SURFACE_HANDLE_H
#define BACKEND_SURFACE_HANDLE_H

struct IDirect3DSurface9;

struct BackendSurfaceHandle
{
    IDirect3DSurface9* D3DSurface;  // DX8 surface, nullptr for other backends
    void* BackendData;               // backend-specific data (e.g. Vulkan image)

    BackendSurfaceHandle() : D3DSurface(nullptr), BackendData(nullptr) {}
};

#endif // BACKEND_SURFACE_HANDLE_H
