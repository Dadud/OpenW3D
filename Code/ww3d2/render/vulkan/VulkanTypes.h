/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	Vulkan backend types and definitions for OpenW3D
*/

#ifndef VULKAN_TYPES_H
#define VULKAN_TYPES_H

#include "render/RenderInterface.h"
#include <vulkan/vulkan.h>

// Maximum values
const int MAX_LIGHTS = 8;
const int MAX_VERTEX_STREAMS = 4;
const int MAX_TEXTURE_STAGES = 4;
const int MAX_RENDER_TARGETS = 1;

// Vulkan-specific structures
struct VulkanQueueFamilyIndices
{
    int GraphicsFamily = -1;
    int PresentFamily = -1;
    bool Is_Complete() const { return GraphicsFamily >= 0 && PresentFamily >= 0; }
};

struct VulkanSwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR Capabilities;
    std::vector<VkSurfaceFormatKHR> Formats;
    std::vector<VkPresentModeKHR> PresentModes;
};

struct VulkanTextureInfo
{
    VkImage Image = VK_NULL_HANDLE;
    VkDeviceMemory Memory = VK_NULL_HANDLE;
    VkImageView ImageView = VK_NULL_HANDLE;
    VkSampler Sampler = VK_NULL_HANDLE;
    int Width = 0;
    int Height = 0;
    RenderInterface::TextureFormat Format = RenderInterface::TEX_FORMAT_A8R8G8B8;
    bool Is_RenderTarget = false;
};

struct VulkanBufferInfo
{
    VkBuffer Buffer = VK_NULL_HANDLE;
    VkDeviceMemory Memory = VK_NULL_HANDLE;
    VkDescriptorBufferInfo Descriptor;
    size_t Size = 0;
    void* Mapped = nullptr;
};

struct VulkanVertexBinding
{
    VkBuffer Buffer = VK_NULL_HANDLE;
    VkDeviceMemory Memory = VK_NULL_HANDLE;
    size_t Stride = 0;
    size_t Offset = 0;
};

struct VulkanRenderPassInfo
{
    VkRenderPass RenderPass = VK_NULL_HANDLE;
    VkFramebuffer Framebuffer = VK_NULL_HANDLE;
    VkCommandBuffer CommandBuffer = VK_NULL_HANDLE;
    VkSemaphore ImageAvailableSemaphore = VK_NULL_HANDLE;
    VkSemaphore RenderFinishedSemaphore = VK_NULL_HANDLE;
    VkFence InFlightFence = VK_NULL_HANDLE;
};

struct VulkanDescriptorSetLayout
{
    VkDescriptorSetLayout Layout = VK_NULL_HANDLE;
    VkDescriptorPool Pool = VK_NULL_HANDLE;
    VkDescriptorSet Set = VK_NULL_HANDLE;
};

// Light mapping from RenderInterface::Light to Vulkan-compatible format
struct VulkanLight
{
    Vector3 Position;
    Vector3 Direction;
    float Range;
    int Type; // 1=Point, 2=Spot, 3=Directional
    Vector4 Diffuse;
    Vector4 Specular;
    Vector4 Ambient;
    bool Enabled;
};

// Render state tracking
struct VulkanRenderState
{
    // Blending
    bool BlendEnabled = false;
    BlendOperation BlendOp = RenderInterface::BLEND_OP_ADD;
    BlendFactor SrcBlend = RenderInterface::BLEND_ONE;
    BlendFactor DstBlend = RenderInterface::BLEND_ZERO;

    // Depth/Stencil
    bool DepthTestEnabled = true;
    bool DepthWriteEnabled = true;
    ComparisonFunc DepthFunc = RenderInterface::CMP_LESSEQUAL;

    bool StencilEnabled = false;
    ComparisonFunc StencilFunc = RenderInterface::CMP_ALWAYS;
    unsigned StencilRef = 0;
    StencilOperation StencilFail = STENCIL_OP_KEEP;
    StencilOperation StencilZFail = STENCIL_OP_KEEP;
    StencilOperation StencilPass = STENCIL_OP_KEEP;

    // Rasterization
    CullMode CullMode = CULL_BACK;
    FillMode FillMode = FILL_SOLID;
    float DepthBias = 0.0f;

    // Scissor
    bool ScissorTestEnabled = false;
    int ScissorLeft = 0, ScissorTop = 0;
    int ScissorRight = 0, ScissorBottom = 0;

    // Viewport
    Viewport Viewport;

    // Lights
    unsigned ActiveLights = 0;
    VulkanLight Lights[MAX_LIGHTS];

    // Texture stages
    struct TextureStage
    {
        void* Texture = nullptr;
        TextureOperation ColorOp = TOP_MODULATE;
        TextureArgument ColorArg1 = TA_CURRENT;
        TextureArgument ColorArg2 = TA_TEXTURE;
        TextureOperation AlphaOp = TOP_SELECTARG1;
        TextureArgument AlphaArg1 = TA_CURRENT;
        TextureArgument AlphaArg2 = TA_TEXTURE;
    } TextureStages[MAX_TEXTURE_STAGES];
};

#endif // VULKAN_TYPES_H
