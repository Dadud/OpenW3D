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
 * VulkanDX8Wrapper - DX8 interface wrapper that delegates to RenderInterface/Vulkan            *
 *                                                                                             *
 * This class provides the same static interface as DX8Wrapper but routes all rendering         *
 * calls through RenderInterface (VulkanRenderInterface on Linux/Windows) instead of D3D9.      *
 *                                                                                             *
 * Use this when you want to render via Vulkan while maintaining D3D9-compatible interface.    *
 *                                                                                             *
 *- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef VULKAN_DX8_WRAPPER_H
#define VULKAN_DX8_WRAPPER_H

#include "dx8wrapper.h"
#include "render/RenderInterface.h"

/*
** VulkanDX8Wrapper
**
** DX8 interface wrapper class that delegates to RenderInterface (Vulkan).
** This provides the same interface as DX8Wrapper but routes all rendering
** operations through the RenderInterface abstraction layer.
**
** Usage:
**   // Initialize Vulkan renderer
**   RenderInterface* renderer = RenderInterface::Create_Renderer();
**   if (renderer) {
**       renderer->Init(window_handle);
**       VulkanDX8Wrapper::Set_Renderer(renderer);
**   }
**
**   // Now all DX8Wrapper-style calls go through Vulkan
**   VulkanDX8Wrapper::Begin_Scene();
**   // ... render calls ...
**   VulkanDX8Wrapper::End_Scene();
*/
class VulkanDX8Wrapper
{
public:
	/*
	** Initialization & Shutdown
	*/
	static bool Init(void* hwnd, bool lite = false);
	static void Shutdown(void);

	/*
	** Device-dependent initialization
	*/
	static void	Do_Onetime_Device_Dependent_Inits(void);
	static void Do_Onetime_Device_Dependent_Shutdowns(void);

	static bool Is_Device_Lost() { return s_Renderer ? s_Renderer->Is_Device_Lost() : true; }
	static bool Is_Initted(void) { return s_Renderer != nullptr && s_Renderer->Is_Initted(); }

	/*
	** Rendering
	*/
	static void Begin_Scene(void);
	static void End_Scene(bool flip_frame = true);
	static void Flip_To_Primary(void);
	static void Clear(bool clear_color, bool clear_z_stencil, const Vector3& color, float z = 1.0f, unsigned int stencil = 0);

	static void Set_Viewport(const RenderInterface::Viewport& viewport);

	static void Set_Vertex_Buffer(const VertexBufferClass* vb);
	static void Set_Vertex_Buffer(const DynamicVBAccessClass& vba);
	static void Set_Index_Buffer(const IndexBufferClass* ib, unsigned short index_base_offset);
	static void Set_Index_Buffer(const DynamicIBAccessClass& iba, unsigned short index_base_offset);
	static void Set_Index_Buffer_Index_Offset(unsigned offset);

	static void Get_Render_State(RenderStateStruct& state);
	static void Set_Render_State(const RenderStateStruct& state);
	static void Release_Render_State();

	static void Set_Gamma(float gamma, float bright, float contrast, bool calibrate = true, bool uselimit = true);

	static void Set_DX8_ZBias(int zbias);
	static void	Set_Pseudo_ZBias(int zbias);
	static void Set_Projection_Transform_With_Z_Bias(const Matrix4& matrix, float znear, float zfar);

	static void Set_Transform(D3DTRANSFORMSTATETYPE transform, const Matrix4& m);
	static void Set_Transform(D3DTRANSFORMSTATETYPE transform, const Matrix3D& m);
	static void Get_Transform(D3DTRANSFORMSTATETYPE transform, Matrix4& m);
	static void	Set_World_Identity();
	static void Set_View_Identity();
	static bool	Is_World_Identity();
	static bool Is_View_Identity();

	static void Set_DX8_Light(int index, D3DLIGHT9* light);
	static void Set_DX8_Render_State(D3DRENDERSTATETYPE state, unsigned value);
	static void Set_DX8_Texture_Stage_State(unsigned stage, D3DTEXTURESTAGESTATETYPE state, unsigned value);
	static void Set_DX8_N_Patch_Mode(float segments);
	static void Set_DX8_Texture_Sampler_State(unsigned sampler, D3DSAMPLERSTATETYPE state, unsigned value);
	static void Set_DX8_Texture(unsigned int stage, IDirect3DBaseTexture9* texture);
	static void Set_Light_Environment(LightEnvironmentClass* light_env);
	static void Set_Fog(bool enable, const Vector3& color, float start, float end);

	static const D3DLIGHT9& Peek_Light(unsigned index);
	static bool Is_Light_Enabled(unsigned index);

	/*
	** Deferred state
	*/
	static void Set_Shader(const ShaderClass& shader);
	static void Get_Shader(ShaderClass& shader);
	static void Set_Texture(unsigned stage, TextureClass* texture);
	static void Set_Material(const VertexMaterialClass* material);
	static void Set_Light(unsigned index, const D3DLIGHT9* light);
	static void Set_Light(unsigned index, const LightClass& light);

	static void Apply_Render_State_Changes();

	static void Draw_Triangles(
		unsigned buffer_type,
		unsigned short start_index,
		unsigned short polygon_count,
		unsigned short min_vertex_index,
		unsigned short vertex_count);
	static void Draw_Triangles(
		unsigned short start_index,
		unsigned short polygon_count,
		unsigned short min_vertex_index,
		unsigned short vertex_count);
	static void Draw_Strip(
		unsigned short start_index,
		unsigned short index_count,
		unsigned short min_vertex_index,
		unsigned short vertex_count);

	/*
	** Resources - Textures
	*/
	static IDirect3DTexture9* _Create_DX8_Texture(
		unsigned int width,
		unsigned int height,
		WW3DFormat format,
		TextureClass::MipCountType mip_level_count,
		D3DPOOL pool = D3DPOOL_MANAGED,
		bool rendertarget = false);
	static IDirect3DTexture9* _Create_DX8_Texture(const char* filename, TextureClass::MipCountType mip_level_count);
	static IDirect3DTexture9* _Create_DX8_Texture(IDirect3DSurface9* surface, TextureClass::MipCountType mip_level_count);

	/*
	** Resources - Surfaces
	*/
	static IDirect3DSurface9* _Create_DX8_Surface(unsigned int width, unsigned int height, D3DPOOL pool, WW3DFormat format);
	static IDirect3DSurface9* _Create_DX8_Surface(const char* filename);
	static IDirect3DSurface9* _Get_DX8_Front_Buffer();
	static SurfaceClass* _Get_DX8_Back_Buffer(unsigned int num = 0);

	static void _Copy_DX8_Rects(
		IDirect3DSurface9* pSourceSurface,
		const RECT* pSourceRectsArray,
		UINT cRects,
		IDirect3DSurface9* pDestinationSurface,
		const POINT* pDestPointsArray);
	static void _Read_Texture(
		IDirect3DSurface9* pSourceSurface,
		IDirect3DSurface9* pDestinationSurface);

	static void _Update_Texture(TextureClass* system, TextureClass* video);
	static void Flush_DX8_Resource_Manager();
	static unsigned int Get_Free_Texture_RAM();

	/*
	** Statistics
	*/
	static void Begin_Statistics();
	static void End_Statistics();
	static unsigned Get_Last_Frame_Matrix_Changes();
	static unsigned Get_Last_Frame_Material_Changes();
	static unsigned Get_Last_Frame_Vertex_Buffer_Changes();
	static unsigned Get_Last_Frame_Index_Buffer_Changes();
	static unsigned Get_Last_Frame_Light_Changes();
	static unsigned Get_Last_Frame_Texture_Changes();
	static unsigned Get_Last_Frame_Render_State_Changes();
	static unsigned Get_Last_Frame_Texture_Stage_State_Changes();
	static unsigned Get_Last_Frame_DX8_Calls();

	static unsigned int Get_FrameCount(void);

	/*
	** Fog
	*/
	static bool Get_Fog_Enable() { return s_Renderer ? s_Renderer->Get_Fog_Enable() : false; }
	static D3DCOLOR Get_Fog_Color() { return s_Renderer ? s_Renderer->Get_Fog_Color() : 0; }

	/*
	** Utilities
	*/
	static Vector4 Convert_Color(unsigned color);
	static unsigned int Convert_Color(const Vector4& color);
	static unsigned int Convert_Color(const Vector3& color, const float alpha);
	static void Clamp_Color(Vector4& color);
	static unsigned int Convert_Color_Clamp(const Vector4& color);
	static void Set_Alpha(const float alpha, unsigned int& color);

	static void Set_Alpha(bool enable) { _EnableTriangleDraw = enable; }
	static bool Is_Triangle_Draw_Enabled() { return _EnableTriangleDraw; }

	/*
	** Render targets
	*/
	static IDirect3DSwapChain9* Create_Additional_Swap_Chain(HWND render_window);
	static TextureClass* Create_Render_Target(int width, int height, WW3DFormat format);
	static void Set_Render_Target(TextureClass* texture);
	static void Set_Render_Target(IDirect3DSurface9* render_target, bool use_default_depth_buffer = false);
	static void Set_Render_Target(IDirect3DSwapChain9* swap_chain);
	static bool Is_Render_To_Texture(void);

	/*
	** Static factory for creating Vulkan renderer
	** Call this first, then call Init() with the returned renderer
	*/
	static RenderInterface* Create_Renderer()
	{
		return RenderInterface::Create_Renderer();
	}

	/*
	** Set the renderer to use (must call Create_Renderer first, then Init on the renderer)
	*/
	static void Set_Renderer(RenderInterface* renderer)
	{
		s_Renderer = renderer;
	}

	/*
	** Get the current renderer
	*/
	static RenderInterface* Get_Renderer()
	{
		return s_Renderer;
	}

protected:
	static RenderInterface* s_Renderer;
	static bool _EnableTriangleDraw;
	static unsigned matrix_changes;
	static unsigned material_changes;
	static unsigned vertex_buffer_changes;
	static unsigned index_buffer_changes;
	static unsigned light_changes;
	static unsigned texture_changes;
	static unsigned render_state_changes;
	static unsigned texture_stage_state_changes;
	static unsigned sampler_state_changes;

	// Internal helper to convert D3D9 primitive type to RenderInterface type
	static RenderInterface::PrimitiveType Convert_Primitive_Type(unsigned d3d_primitive_type);

	// Internal helper to convert D3D9 light to RenderInterface light
	static void Convert_Light(const D3DLIGHT9& d3d_light, RenderInterface::Light& vulkan_light);
};

#endif // VULKAN_DX8_WRAPPER_H
