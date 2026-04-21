/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	Vulkan implementation of RenderInterface for cross-platform rendering
*/

#ifndef VULKAN_RENDER_INTERFACE_H
#define VULKAN_RENDER_INTERFACE_H

#include "render/RenderInterface.h"
#include "VulkanTypes.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <map>

class VulkanRenderInterface : public RenderInterface
{
public:
    VulkanRenderInterface();
    virtual ~VulkanRenderInterface();

    // =====================================================================
    // Initialization & Shutdown
    // =====================================================================

    bool Init(void* window_handle, bool lite = false) override;
    void Shutdown() override;
    void Do_Ontime_Device_Dependent_Inits() override;
    void Do_Ontime_Device_Dependent_Shutdowns() override;
    bool Is_Device_Lost() const override { return m_DeviceLost; }
    bool Is_Initted() const override { return m_Initted; }

    // =====================================================================
    // Scene Rendering
    // =====================================================================

    void Begin_Scene() override;
    void End_Scene(bool flip_frame = true) override;
    void Flip_To_Primary() override;
    void Clear(bool clear_color, bool clear_z_stencil, const Vector3& color,
               float z = 1.0f, unsigned int stencil = 0) override;

    // =====================================================================
    // Vertex & Index Buffers
    // =====================================================================

    void Set_Vertex_Buffer(const VertexBufferClass* vb) override;
    void Set_Vertex_Buffer(const DynamicVBAccessClass& vba) override;
    void Set_Index_Buffer(const IndexBufferClass* ib, unsigned short index_base_offset) override;
    void Set_Index_Buffer(const DynamicIBAccessClass& iba, unsigned short index_base_offset) override;
    void Set_Index_Buffer_Index_Offset(unsigned offset) override;

    // =====================================================================
    // Transform & State
    // =====================================================================

    void Set_Transform(TransformType type, const Matrix4& matrix) override;
    void Set_Transform(TransformType type, const Matrix3D& matrix) override;
    void Get_Transform(TransformType type, Matrix4& matrix) override;
    void Set_World_Identity() override;
    void Set_View_Identity() override;
    bool Is_World_Identity() const override;
    bool Is_View_Identity() const override;

    // =====================================================================
    // Lighting
    // =====================================================================

    void Set_Light(unsigned index, const Light& light) override;
    void Set_Light(unsigned index, const LightClass& light) override;
    void Enable_Light(unsigned index, bool enable) override;
    void Set_Light_Count(unsigned count) override;
    void Set_Light_Environment(LightEnvironmentClass* light_env) override;

    // =====================================================================
    // Materials & Shaders
    // =====================================================================

    void Set_Material(const VertexMaterialClass* material) override;
    void Set_Shader(const ShaderClass& shader) override;
    void Get_Shader(ShaderClass& shader) override;

    // =====================================================================
    // Textures
    // =====================================================================

    void Set_Texture(unsigned stage, TextureClass* texture) override;

    // =====================================================================
    // Fog
    // =====================================================================

    void Set_Fog(bool enable, const Vector3& color, float start, float end) override;

    // =====================================================================
    // Drawing
    // =====================================================================

    void Draw(PrimitiveType type, unsigned short start_index, unsigned short polygon_count,
              unsigned short min_vertex_index = 0, unsigned short vertex_count = 0) override;
    void Draw_Indexed(PrimitiveType type, unsigned short start_index,
                      unsigned short polygon_count, unsigned short min_vertex_index,
                      unsigned short vertex_count) override;
    void Draw_Triangles(unsigned buffer_type, unsigned short start_index,
                        unsigned short polygon_count, unsigned short min_vertex_index,
                        unsigned short vertex_count) override;

    // =====================================================================
    // Render States
    // =====================================================================

    void Set_Render_State(RenderState state, unsigned value) override;
    void Set_Render_State(RenderState state, int value) override;
    void Set_Blend_Operation(BlendOperation op) override;
    void Set_Source_Blend(BlendFactor factor) override;
    void Set_Dest_Blend(BlendFactor factor) override;
    void Set_Cull_Mode(CullMode mode) override;
    void Set_Depth_Bias(float bias) override;
    void Set_Scissor_Test(bool enable, int left, int top, int right, int bottom) override;

    // =====================================================================
    // Viewport
    // =====================================================================

    void Set_Viewport(const Viewport& viewport) override;

    // =====================================================================
    // Texture Stage States
    // =====================================================================

    void Set_Texture_Stage_State(unsigned stage, TextureStageState state, unsigned value) override;

    // =====================================================================
    // Sampler States
    // =====================================================================

    void Set_Sampler_State(unsigned sampler, SamplerState state, unsigned value) override;

    // =====================================================================
    // Resources - Textures
    // =====================================================================

    void* Create_Texture(unsigned width, unsigned height, TextureFormat format,
                         bool generate_mips = false, bool rendertarget = false) override;
    void* Create_Texture(const char* filename, bool generate_mips = false) override;
    void Update_Texture(TextureClass* system_texture, TextureClass* video_texture) override;

    // =====================================================================
    // Resources - Surfaces
    // =====================================================================

    void* Create_Surface(unsigned width, unsigned height, TextureFormat format,
                          TexturePool pool = POOL_DEFAULT) override;
    void* Create_Surface(const char* filename) override;
    void* Get_Front_Buffer() override;
    void* Get_Back_Buffer(unsigned index = 0) override;
    void Copy_Rects(void* source_surface, const void* source_rects, unsigned rect_count,
                    void* dest_surface, int dest_x, int dest_y) override;
    void Read_Texture(void* surface, void* dest_surface) override;

    // =====================================================================
    // Query Methods
    // =====================================================================

    unsigned int Get_Frame_Count() const override { return m_FrameCount; }
    unsigned int Get_Frame_Rate() const override { return m_FrameRate; }
    unsigned int Get_Triangle_Count() const override { return m_TriangleCount; }
    unsigned int Get_Draw_Call_Count() const override { return m_DrawCallCount; }

    // =====================================================================
    // Vulkan-specific methods
    // =====================================================================

    VkInstance Get_Vulkan_Instance() const { return m_Instance; }
    VkPhysicalDevice Get_Physical_Device() const { return m_PhysicalDevice; }
    VkDevice Get_Device() const { return m_Device; }
    VkQueue Get_Graphics_Queue() const { return m_GraphicsQueue; }
    VkCommandPool Get_Command_Pool() const { return m_CommandPool; }

    // Begin single-use command buffer for resource creation
    VkCommandBuffer Begin_Single_Time_Commands();
    void End_Single_Time_Commands(VkCommandBuffer command_buffer);

private:
    // Internal helper methods
    bool Create_Instance();
    bool Create_Device();
    bool Create_Command_Pool();
    bool Create_Swap_Chain();
    bool Create_Render_Pass();
    bool Create_Descriptor_Set_Layout();
    bool Create_Graphics_Pipeline();
    bool Create_Framebuffers();
    bool Create_Sync_Objects();
    void Cleanup_Swap_Chain();

    // Vertex and Index buffer helpers
    bool Create_Vertex_Buffer(size_t size, const void* data, VkBufferUsageFlags usage);
    bool Create_Index_Buffer(size_t size, const void* data, VkBufferUsageFlags usage);
    bool Copy_To_GPU(VkBuffer dst_buffer, VkDeviceMemory dst_memory, const void* data, size_t size);

    // Vulkan conversion helpers
    VkFormat Convert_Format(TextureFormat format);
    VkPrimitiveTopology Convert_Primitive_Type(PrimitiveType type);
    VkCullModeFlags Convert_Cull_Mode(CullMode mode);
    VkBlendFactor Convert_Blend_Factor(BlendFactor factor);
    VkBlendOp Convert_Blend_Op(BlendOperation op);
    VkCompareOp Convert_Compare_Op(ComparisonFunc func);
    VkStencilOp Convert_Stencil_Op(StencilOperation op);
    VkFilter Convert_Filter(TextureFilter filter);
    VkSamplerAddressMode Convert_Texture_Address(TextureAddress addr);

    // Shader module creation helper
    VkShaderModule Create_Shader_Module(const uint32_t* code, size_t size);

    // Matrix conversion helpers (D3D to Vulkan coordinate system)
    Matrix4 Flip_Y_Axis(const Matrix4& matrix);
    Matrix4 Convert_Projection(const Matrix4& matrix);

    // Update descriptor sets for current frame
    void Update_Descriptor_Sets();

    // Rebuild pipeline state when needed
    void Rebuild_Pipeline_State();

    // State tracking
    bool m_Initted = false;
    bool m_DeviceLost = false;
    bool m_LiteMode = false;

    // Frame counters
    unsigned int m_FrameCount = 0;
    unsigned int m_FrameRate = 0;
    unsigned int m_TriangleCount = 0;
    unsigned int m_DrawCallCount = 0;

    // Window handle
    void* m_WindowHandle = nullptr;

    // Vulkan core objects
    VkInstance m_Instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    VkDevice m_Device = VK_NULL_HANDLE;
    VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
    VkQueue m_PresentQueue = VK_NULL_HANDLE;
    VkCommandPool m_CommandPool = VK_NULL_HANDLE;

    // Surface and queue families
    VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
    uint32_t m_GraphicsQueueFamily = 0;
    uint32_t m_PresentQueueFamily = 0;

    // Swap chain
    VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
    std::vector<VkImage> m_SwapChainImages;
    std::vector<VkImageView> m_SwapChainImageViews;
    VkFormat m_SwapChainImageFormat;
    VkExtent2D m_SwapChainExtent;

    // Render pass and framebuffers
    VkRenderPass m_RenderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> m_SwapChainFramebuffers;

    // Pipeline
    VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_GraphicsPipeline = VK_NULL_HANDLE;

    // Shader modules
    VkShaderModule m_VertexShaderModule = VK_NULL_HANDLE;
    VkShaderModule m_FragmentShaderModule = VK_NULL_HANDLE;

    // Descriptor set
    VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;

    // Current frame data
    uint32_t m_CurrentFrame = 0;
    uint32_t m_ImageIndex = 0;

    // Sync objects
    std::vector<VkSemaphore> m_ImageAvailableSemaphores;
    std::vector<VkSemaphore> m_RenderFinishedSemaphores;
    std::vector<VkFence> m_InFlightFences;

    // Vertex and index buffers
    VulkanVertexBinding m_VertexBuffers[MAX_VERTEX_STREAMS];
    VulkanBufferInfo m_IndexBuffer;
    unsigned m_IndexBufferOffset = 0;

    // Current transforms
    Matrix4 m_WorldMatrix;
    Matrix4 m_ViewMatrix;
    Matrix4 m_ProjectionMatrix;
    Matrix4 m_TextureMatrices[4];
    bool m_WorldIdentity = false;
    bool m_ViewIdentity = false;

    // Render state tracking
    VulkanRenderState m_RenderState;

    // Active textures
    void* m_Textures[MAX_TEXTURE_STAGES] = { nullptr };

    // Light environment
    LightEnvironmentClass* m_LightEnvironment = nullptr;

    // Shader/material/fog dirty flags for pipeline rebuilding
    bool m_DescriptorSetDirty = false;
    bool m_MaterialDirty = false;
    bool m_FogDirty = false;
    bool m_ShaderDirty = false;

    // Current material
    const VertexMaterialClass* m_CurrentMaterial = nullptr;
    ShaderClass m_CurrentShader;

    // Material data for shader uniforms
    struct {
        Vector4 Ambient;
        Vector4 Diffuse;
        Vector4 Specular;
        Vector4 Emissive;
        float EnableLighting;
        float DepthCue;
    } m_MaterialData;

    // Fog data for shader uniforms
    struct {
        float Enabled;
        Vector4 Color;
        float Start;
        float End;
        float Range;
    } m_FogData;

    // Fog settings
    bool m_FogEnabled = false;
    Vector3 m_FogColor = Vector3(0.5f, 0.5f, 0.5f);
    float m_FogStart = 0.0f;
    float m_FogEnd = 1.0f;

    // Current command buffer
    VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;

    // Texture cache for cleanup
    std::map<std::string, VulkanTextureInfo*> m_TextureCache;

    // =====================================================================
    // Internal texture helper methods
    // =====================================================================

    // Find suitable memory type for allocation
    uint32_t Find_Memory_Type(uint32_t type_filter, VkMemoryPropertyFlags properties);

    // Create a Vulkan image with proper memory allocation
    bool Create_Image(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
                      VkImageUsageFlags usage, uint32_t mip_levels, VkMemoryPropertyFlags properties,
                      VkImage& image, VkDeviceMemory& memory);

    // Create an image view for the given image
    bool Create_Image_View(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags,
                           VkImageViewType view_type, uint32_t mip_levels, VkImageView& view);

    // Create a sampler with specified filtering and addressing
    bool Create_Sampler(VkFilter mag_filter, VkFilter min_filter, VkSamplerAddressMode address_mode_u,
                       VkSamplerAddressMode address_mode_v, VkSamplerMipmapMode mip_mode, float max_anisotropy,
                       VkSampler& sampler);

    // Transition image layout between layouts
    void Transition_Image_Layout(VkImage image, VkFormat format, VkImageLayout old_layout,
                                 VkImageLayout new_layout, uint32_t mip_levels);

    // Copy data from buffer to image
    void Copy_Buffer_To_Image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    // Copy image to image
    void Copy_Image_To_Image(VkImage src_image, VkImage dst_image, uint32_t width, uint32_t height,
                             VkImageAspectFlags aspect_mask);

    // Generate mipmaps for an image
    void Generate_Mipmaps(VkImage image, VkFormat format, uint32_t width, uint32_t height, uint32_t mip_levels);

    // Load texture from file (TGA format)
    VulkanTextureInfo* Load_Texture_From_File(const char* filename, bool generate_mips);

    // Get bytes per pixel for texture format
    uint32_t Get_Bytes_Per_Pixel(TextureFormat format);

    // Get Vulkan format from TextureFormat
    VkFormat Get_Vulkan_Format(TextureFormat format);

    // Copy rectangle helper
    void Copy_Rect_Helper(VkImage src_image, VkImage dst_image, uint32_t src_x, uint32_t src_y,
                          uint32_t dst_x, uint32_t dst_y, uint32_t width, uint32_t height);
};

#endif // VULKAN_RENDER_INTERFACE_H
