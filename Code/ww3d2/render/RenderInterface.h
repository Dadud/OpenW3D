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
 * RenderInterface - Abstract rendering interface for cross-platform Vulkan backend              *
 *                                                                                             *
 * This interface abstracts all rendering operations, allowing a single code path to run        *
 * on Windows (via D3D9 or Vulkan), Linux (via Vulkan), and macOS (via MoltenVK).              *
 *                                                                                             *
 *- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef RENDER_INTERFACE_H
#define RENDER_INTERFACE_H

// Basic types - these should be provided by the game
#ifndef ALWAYS_H
// Simple type definitions if always.h isn't included
#include <cstdint>
typedef uint8_t uint8;
typedef int8_t int8;
typedef uint16_t uint16;
typedef int16_t int16;
typedef uint32_t uint32;
typedef int32_t int32;
typedef uint64_t uint64;
typedef int64_t int64;
typedef float float32;
typedef double float64;
#endif

// Forward declarations for game classes
class TextureClass;
class VertexMaterialClass;
class ShaderClass;
class LightClass;
class LightEnvironmentClass;
class Matrix4;
class Matrix3D;
class Vector3;
class Vector4;
struct VertexBufferClass;
struct IndexBufferClass;
struct DynamicVBAccessClass;
struct DynamicIBAccessClass;

/*
** RenderInterface - Abstract base class for all rendering backends
**
** All rendering operations go through this interface, allowing the game
** to run on any backend (D3D9, Vulkan, Metal, etc.) without changing game code.
*/
class RenderInterface
{
public:
    virtual ~RenderInterface() = default;

    // =====================================================================
    // Initialization & Shutdown
    // =====================================================================

    // Initialize the renderer with a window handle (HWND on Windows, void* on Unix)
    virtual bool Init(void* window_handle, bool lite = false) = 0;
    virtual void Shutdown() = 0;

    // Device-dependent one-time initialization (called after device creation)
    virtual void Do_Onetime_Device_Dependent_Inits() = 0;
    virtual void Do_Onetime_Device_Dependent_Shutdowns() = 0;

    // Check device state
    virtual bool Is_Device_Lost() const = 0;
    virtual bool Is_Initted() const = 0;

    // =====================================================================
    // Scene Rendering
    // =====================================================================

    virtual void Begin_Scene() = 0;
    virtual void End_Scene(bool flip_frame = true) = 0;
    virtual void Flip_To_Primary() = 0;

    // Clear framebuffer
    // clear_color: clear color buffer
    // clear_z_stencil: clear depth and stencil buffers
    // color: clear color value
    // z: clear depth value (typically 1.0)
    // stencil: clear stencil value
    virtual void Clear(bool clear_color, bool clear_z_stencil, const Vector3& color, float z = 1.0f, unsigned int stencil = 0) = 0;

    // =====================================================================
    // Vertex & Index Buffers
    // =====================================================================

    virtual void Set_Vertex_Buffer(const VertexBufferClass* vb) = 0;
    virtual void Set_Vertex_Buffer(const DynamicVBAccessClass& vba) = 0;
    virtual void Set_Index_Buffer(const IndexBufferClass* ib, unsigned short index_base_offset) = 0;
    virtual void Set_Index_Buffer(const DynamicIBAccessClass& iba, unsigned short index_base_offset) = 0;
    virtual void Set_Index_Buffer_Index_Offset(unsigned offset) = 0;

    // =====================================================================
    // Transform & State
    // =====================================================================

    // Matrix transforms (Westwood convention)
    enum TransformType {
        TRANSFORM_WORLD = 0,
        TRANSFORM_VIEW = 1,
        TRANSFORM_PROJECTION = 2,
        TRANSFORM_TEXTURE0 = 3,
        TRANSFORM_TEXTURE1 = 4,
        TRANSFORM_TEXTURE2 = 5,
        TRANSFORM_TEXTURE3 = 6,
    };

    virtual void Set_Transform(TransformType type, const Matrix4& matrix) = 0;
    virtual void Set_Transform(TransformType type, const Matrix3D& matrix) = 0;
    virtual void Get_Transform(TransformType type, Matrix4& matrix) = 0;
    virtual void Set_World_Identity() = 0;
    virtual void Set_View_Identity() = 0;
    virtual bool Is_World_Identity() const = 0;
    virtual bool Is_View_Identity() const = 0;

    // =====================================================================
    // Lighting
    // =====================================================================

    struct Light
    {
        enum LightType {
            LIGHT_POINT = 1,
            LIGHT_SPOT = 2,
            LIGHT_DIRECTIONAL = 3,
        };

        LightType Type;
        Vector3 Position;
        Vector3 Direction;
        float Range;
        float Falloff;
        float Attenuation0;
        float Attenuation1;
        float Attenuation2;
        float Theta;      // Spotlight inner angle
        float Phi;        // Spotlight outer angle
        Vector4 Diffuse;
        Vector4 Specular;
        Vector4 Ambient;
        bool Enabled;
    };

    virtual void Set_Light(unsigned index, const Light& light) = 0;
    virtual void Set_Light(unsigned index, const LightClass& light) = 0;
    virtual void Enable_Light(unsigned index, bool enable) = 0;
    virtual void Set_Light_Count(unsigned count) = 0;
    virtual void Set_Light_Environment(LightEnvironmentClass* light_env) = 0;

    // =====================================================================
    // Materials & Shaders
    // =====================================================================

    virtual void Set_Material(const VertexMaterialClass* material) = 0;
    virtual void Set_Shader(const ShaderClass& shader) = 0;
    virtual void Get_Shader(ShaderClass& shader) = 0;

    // =====================================================================
    // Textures
    // =====================================================================

    virtual void Set_Texture(unsigned stage, TextureClass* texture) = 0;

    // =====================================================================
    // Fog
    // =====================================================================

    virtual void Set_Fog(bool enable, const Vector3& color, float start, float end) = 0;

    // =====================================================================
    // Drawing
    // =====================================================================

    enum PrimitiveType {
        PRIMITIVE_POINT_LIST = 1,
        PRIMITIVE_LINE_LIST = 2,
        PRIMITIVE_LINE_STRIP = 3,
        PRIMITIVE_TRIANGLE_LIST = 4,
        PRIMITIVE_TRIANGLE_STRIP = 5,
        PRIMITIVE_TRIANGLE_FAN = 6,
    };

    virtual void Draw(PrimitiveType type, unsigned short start_index, unsigned short polygon_count,
                     unsigned short min_vertex_index = 0, unsigned short vertex_count = 0) = 0;

    virtual void Draw_Indexed(PrimitiveType type, unsigned short start_index,
                             unsigned short polygon_count, unsigned short min_vertex_index,
                             unsigned short vertex_count) = 0;

    virtual void Draw_Triangles(unsigned buffer_type, unsigned short start_index,
                               unsigned short polygon_count, unsigned short min_vertex_index,
                               unsigned short vertex_count) = 0;

    // =====================================================================
    // Render States
    // =====================================================================

    enum RenderState {
        RS_ZENABLE,           // Z-buffer enable
        RS_ZWRITEENABLE,      // Z-write enable
        RS_ALPHABLENDENABLE,  // Alpha blending
        RS_ALPHATESTENABLE,   // Alpha testing
        RS_CULLMODE,          // Culling mode
        RS_FILLMODE,          // Fill mode
        RS_SHADEMODE,         // Shade mode
        RS_BLENDOP,           // Blend operation
        RS_SRCBLEND,          // Source blend
        RS_DESTBLEND,         // Destination blend
        RS_DEPTHBIAS,         // Depth bias
        RS_STENCILENABLE,     // Stencil enable
        RS_STENCILFUNC,       // Stencil function
        RS_STENCILREF,        // Stencil reference
        RS_STENCILMASK,       // Stencil mask
        RS_STENCILWRITEMASK,  // Stencil write mask
        RS_STENCILFAIL,       // Stencil fail operation
        RS_STENCILZFAIL,      // Stencil Z-fail operation
        RS_STENCILPASS,       // Stencil pass operation
        RS_SCISSORTESTENABLE, // Scissor test
        RS_CLIPPLANEENABLE,   // Clip planes
        RS_COLORWRITEENABLE,  // Color write enable
        RS_SRGBWRITEENABLE,   // sRGB write enable
    };

    enum BlendOperation {
        BLEND_OP_ADD = 1,
        BLEND_OP_SUBTRACT = 2,
        BLEND_OP_REV_SUBTRACT = 3,
        BLEND_OP_MIN = 4,
        BLEND_OP_MAX = 5,
    };

    enum BlendFactor {
        BLEND_ZERO = 1,
        BLEND_ONE = 2,
        BLEND_SRCCOLOR = 3,
        BLEND_INVSRCCOLOR = 4,
        BLEND_SRCALPHA = 5,
        BLEND_INVSRCALPHA = 6,
        BLEND_DESTALPHA = 7,
        BLEND_INVDESTALPHA = 8,
        BLEND_DESTCOLOR = 9,
        BLEND_INVDESTCOLOR = 10,
        BLEND_SRCALPHASAT = 11,
        BLEND_BLENDFACTOR = 12,
        BLEND_INVBLENDFACTOR = 13,
        BLEND_SRC1COLOR = 14,
        BLEND_INVSRC1COLOR = 15,
        BLEND_SRC1ALPHA = 16,
        BLEND_INVSRC1ALPHA = 17,
    };

    enum ComparisonFunc {
        CMP_NEVER = 1,
        CMP_LESS = 2,
        CMP_EQUAL = 3,
        CMP_LESSEQUAL = 4,
        CMP_GREATER = 5,
        CMP_NOTEQUAL = 6,
        CMP_GREATEREQUAL = 7,
        CMP_ALWAYS = 8,
    };

    enum StencilOperation {
        STENCIL_OP_KEEP = 1,
        STENCIL_OP_ZERO = 2,
        STENCIL_OP_REPLACE = 3,
        STENCIL_OP_INCR = 4,
        STENCIL_OP_DECR = 5,
        STENCIL_OP_INVERT = 6,
        STENCIL_OP_INCRSAT = 7,
        STENCIL_OP_DECRSAT = 8,
    };

    enum CullMode {
        CULL_NONE = 1,
        CULL_FRONT = 2,
        CULL_BACK = 3,
    };

    enum FillMode {
        FILL_POINT = 1,
        FILL_WIREFRAME = 2,
        FILL_SOLID = 3,
    };

    virtual void Set_Render_State(RenderState state, unsigned value) = 0;
    virtual void Set_Render_State(RenderState state, int value) = 0;
    virtual void Set_Blend_Operation(BlendOperation op) = 0;
    virtual void Set_Source_Blend(BlendFactor factor) = 0;
    virtual void Set_Dest_Blend(BlendFactor factor) = 0;
    virtual void Set_Cull_Mode(CullMode mode) = 0;
    virtual void Set_Depth_Bias(float bias) = 0;
    virtual void Set_Scissor_Test(bool enable, int left, int top, int right, int bottom) = 0;

    // =====================================================================
    // Viewport
    // =====================================================================

    struct Viewport
    {
        unsigned int X;
        unsigned int Y;
        unsigned int Width;
        unsigned int Height;
        float MinZ;
        float MaxZ;
    };

    virtual void Set_Viewport(const Viewport& viewport) = 0;

    // =====================================================================
    // Texture Stage States
    // =====================================================================

    enum TextureStageState {
        TSS_COLOROP = 1,
        TSS_COLORARG1 = 2,
        TSS_COLORARG2 = 3,
        TSS_ALPHAOP = 4,
        TSS_ALPHAARG1 = 5,
        TSS_ALPHAARG2 = 6,
        TSS_BUMPENVLSCALE = 7,
        TSS_BUMPENVLOFFSET = 8,
        TSS_TEXTURETRANSFORMFLAGS = 9,
    };

    enum TextureArgument {
        TA_CONSTANT = 2,
        TA_CURRENT = 3,
        TA_DIFFUSE = 4,
        TA_SELECTMASK = 5,
        TA_SPECULAR = 6,
        TA_TEMP = 7,
        TA_TFACTOR = 8,
    };

    enum TextureOperation {
        TOP_DISABLE = 1,
        TOP_SELECTARG1 = 2,
        TOP_SELECTARG2 = 3,
        TOP_MODULATE = 4,
        TOP_MODULATE2X = 5,
        TOP_MODULATE4X = 6,
        TOP_ADD = 7,
        TOP_ADDSIGNED = 8,
        TOP_ADDSIGNED2X = 9,
        TOP_SUBTRACT = 10,
        TOP_ADDSMOOTH = 11,
        TOP_BLENDDIFFUSEALPHA = 12,
        TOP_BLENDTEXTUREALPHA = 13,
        TOP_BLENDFACTORALPHA = 14,
        TOP_BLENDTEXTUREALPHAPM = 15,
        TOP_BLENDCURRENTALPHA = 16,
        TOP_PREMODULATE = 17,
        TOP_MODULATEALPHA_ADDCOLOR = 18,
        TOP_MODULATECOLOR_ADDALPHA = 19,
        TOP_MODULATEINVALPHA_ADDCOLOR = 20,
        TOP_MODULATEINVCOLOR_ADDALPHA = 21,
        TOP_BUMPENVMAP = 22,
        TOP_BUMPENVLSCALE = 23,
        TOP_BUMPENVLOFFSET = 24,
        TOP_DOTPRODUCT = 25,
    };

    virtual void Set_Texture_Stage_State(unsigned stage, TextureStageState state, unsigned value) = 0;

    // =====================================================================
    // Sampler States
    // =====================================================================

    enum SamplerState {
        SS_ADDRESSU = 1,
        SS_ADDRESSV = 2,
        SS_ADDRESSW = 3,
        SS_BORDERCOLOR = 4,
        SS_MAGFILTER = 5,
        SS_MINFILTER = 6,
        SS_MIPFILTER = 7,
        SS_MIPMAPLODBIAS = 8,
        SS_MAXMIPLEVEL = 9,
        SS_MAXANISOTROPY = 10,
        SS_SRGBTEXTURE = 11,
    };

    enum TextureAddress {
        TAM_WRAP = 1,
        TAM_MIRROR = 2,
        TAM_CLAMP = 3,
        TAM_BORDER = 4,
        TAM_MIRRORONCE = 5,
    };

    enum TextureFilter {
        TF_NONE = 1,
        TF_POINT = 2,
        TF_LINEAR = 3,
        TF_ANISOTROPIC = 4,
        TF_PYRAMIDALQUAD = 5,
        TF_GAUSSIANQUAD = 6,
    };

    virtual void Set_Sampler_State(unsigned sampler, SamplerState state, unsigned value) = 0;

    // =====================================================================
    // Rectangle structure for Copy_Rects
    // =====================================================================

    struct Rect {
        unsigned X;
        unsigned Y;
        unsigned Width;
        unsigned Height;
    };

    // =====================================================================
    // Resources - Textures
    // =====================================================================

    enum TextureFormat {
        TEX_FORMAT_A8R8G8B8 = 0,
        TEX_FORMAT_X8R8G8B8 = 1,
        TEX_FORMAT_R5G6B5 = 2,
        TEX_FORMAT_A1R5G5B5 = 3,
        TEX_FORMAT_A4R4G4B4 = 4,
        TEX_FORMAT_L8 = 5,
        TEX_FORMAT_A8L8 = 6,
        TEX_FORMAT_DXT1 = 7,
        TEX_FORMAT_DXT3 = 8,
        TEX_FORMAT_DXT5 = 9,
        TEX_FORMAT_D16 = 10,
        TEX_FORMAT_D24S8 = 11,
        TEX_FORMAT_D32 = 12,
    };

    enum TexturePool {
        POOL_DEFAULT = 0,
        POOL_MANAGED = 1,
        POOL_SYSTEMMEM = 2,
    };

    // Create a new texture
    virtual void* Create_Texture(unsigned width, unsigned height, TextureFormat format,
                                 bool generate_mips = false, bool rendertarget = false) = 0;

    // Create texture from file
    virtual void* Create_Texture(const char* filename, bool generate_mips = false) = 0;

    // Update texture from system memory
    virtual void Update_Texture(TextureClass* system_texture, TextureClass* video_texture) = 0;

    // =====================================================================
    // Resources - Surfaces
    // =====================================================================

    virtual void* Create_Surface(unsigned width, unsigned height, TextureFormat format,
                                 TexturePool pool = POOL_DEFAULT) = 0;
    virtual void* Create_Surface(const char* filename) = 0;
    virtual void* Get_Front_Buffer() = 0;
    virtual void* Get_Back_Buffer(unsigned index = 0) = 0;

    // Copy between surfaces
    virtual void Copy_Rects(void* source_surface, const void* source_rects, unsigned rect_count,
                           void* dest_surface, int dest_x, int dest_y) = 0;

    // Read texture data back from GPU
    virtual void Read_Texture(void* surface, void* dest_surface) = 0;

    // =====================================================================
    // Query Methods
    // =====================================================================

    virtual unsigned int Get_Frame_Count() const = 0;
    virtual unsigned int Get_Frame_Rate() const = 0;
    virtual unsigned int Get_Triangle_Count() const = 0;
    virtual unsigned int Get_Draw_Call_Count() const = 0;

    // =====================================================================
    // Static Factory Method
    // =====================================================================

    // Create the appropriate renderer for this platform
    // Returns nullptr if no suitable renderer is available
    static RenderInterface* Create_Renderer();

    // Get singleton instance
    static RenderInterface* Get() { return s_Instance; }

protected:
    static RenderInterface* s_Instance;
};

// Convenience macros for renderer access
#define RENDERER (RenderInterface::Get())

#endif // RENDER_INTERFACE_H
