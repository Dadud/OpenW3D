/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 Electronic Arts Inc.
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
 *                 Project Name : OpenW3D                                                       *
 *                                                                                             *
 *                    $Author:: Orbit                                                           $*
 *                                                                                             *
 *                 $Modtime:: 4/21/26                                                          $*
 *                                                                                             *
 *                    $Revision:: 1                                                            $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * RenderInterface implementation - Factory for creating appropriate renderer backend             *
 *                                                                                             *
 *- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "render/RenderInterface.h"

// Platform detection
#if defined(_WIN32)
#include <windows.h>
// D3D9 is available on Windows
#endif

// Vulkan backend (when available)
#ifdef USE_VULKAN
#include "render/vulkan/VulkanRenderInterface.h"
#endif

/*
** Static instance pointer
*/
RenderInterface* RenderInterface::s_Instance = nullptr;

/*
** RenderInterface::Create_Renderer()
**
** Factory method that creates the appropriate renderer for the current platform.
** 
** Platform detection priority:
** 1. Vulkan (USE_VULKAN defined) - Best cross-platform support
** 2. D3D9 (Windows only) - Legacy fallback
** 3. nullptr - No renderer available
**
** Returns:
**   - Pointer to created RenderInterface instance
**   - nullptr if no suitable renderer is available
*/
RenderInterface* RenderInterface::Create_Renderer()
{
    // If we already have a renderer, return it
    if (s_Instance != nullptr) {
        return s_Instance;
    }

#ifdef USE_VULKAN
    // Vulkan is preferred when available (cross-platform)
    // On Windows, Vulkan can provide better compatibility than D3D9 in some cases
    // On Linux/macOS, Vulkan (or MoltenVK) is the only hardware-accelerated option
    
    VulkanRenderInterface* vulkanRenderer = new VulkanRenderInterface();
    if (vulkanRenderer) {
        s_Instance = vulkanRenderer;
        return s_Instance;
    }
    
    // If Vulkan failed to initialize, fall through to try other backends
#endif

#ifdef _WIN32
    // Windows: Try D3D9 backend (DX8Wrapper provides D3D9 interface)
    // Note: On Windows with USE_VULKAN, we prefer Vulkan over D3D9
    // This path would create a D3D9-specific RenderInterface implementation
    // For now, we return nullptr on Windows if Vulkan is not enabled
    // The game will use DX8Wrapper directly when RenderInterface is not available
    return nullptr;
#else
    // Non-Windows platforms without Vulkan: no hardware renderer available
    return nullptr;
#endif
}
