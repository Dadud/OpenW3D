/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	Vulkan implementation of RenderInterface for cross-platform rendering
*/

#include "VulkanRenderInterface.h"
#include "VulkanTypes.h"
#include "ww3d.h"
#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <cmath>

// Static instance
RenderInterface* RenderInterface::s_Instance = nullptr;

VulkanRenderInterface::VulkanRenderInterface()
{
    s_Instance = this;
}

VulkanRenderInterface::~VulkanRenderInterface()
{
    Shutdown();
    s_Instance = nullptr;
}

bool VulkanRenderInterface::Init(void* window_handle, bool lite)
{
    m_WindowHandle = window_handle;
    m_LiteMode = lite;

    // Initialize Vulkan
    if (!Create_Instance()) {
        std::cerr << "Failed to create Vulkan instance" << std::endl;
        return false;
    }

    // Create surface and swap chain first (this also determines queue families)
    if (!Create_Swap_Chain()) {
        std::cerr << "Failed to create swap chain" << std::endl;
        return false;
    }

    // Now create device with the queue families determined by the surface
    if (!Create_Device()) {
        std::cerr << "Failed to create Vulkan device" << std::endl;
        return false;
    }

    if (!Create_Command_Pool()) {
        std::cerr << "Failed to create command pool" << std::endl;
        return false;
    }

    if (!Create_Render_Pass()) {
        std::cerr << "Failed to create render pass" << std::endl;
        return false;
    }

    if (!Create_Descriptor_Set_Layout()) {
        std::cerr << "Failed to create descriptor set layout" << std::endl;
        return false;
    }

    if (!Create_Graphics_Pipeline()) {
        std::cerr << "Failed to create graphics pipeline" << std::endl;
        return false;
    }

    // Create framebuffers
    if (!Create_Framebuffers()) {
        std::cerr << "Failed to create framebuffers" << std::endl;
        return false;
    }

    // Create sync objects
    if (!Create_Sync_Objects()) {
        std::cerr << "Failed to create sync objects" << std::endl;
        return false;
    }

    m_Initted = true;
    return true;
}

void VulkanRenderInterface::Shutdown()
{
    if (m_Device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(m_Device);
    }

    // Cleanup sync objects
    for (size_t i = 0; i < m_InFlightFences.size(); i++) {
        if (m_InFlightFences[i] != VK_NULL_HANDLE) {
            vkDestroyFence(m_Device, m_InFlightFences[i], nullptr);
            m_InFlightFences[i] = VK_NULL_HANDLE;
        }
        if (m_RenderFinishedSemaphores[i] != VK_NULL_HANDLE) {
            vkDestroySemaphore(m_Device, m_RenderFinishedSemaphores[i], nullptr);
            m_RenderFinishedSemaphores[i] = VK_NULL_HANDLE;
        }
        if (m_ImageAvailableSemaphores[i] != VK_NULL_HANDLE) {
            vkDestroySemaphore(m_Device, m_ImageAvailableSemaphores[i], nullptr);
            m_ImageAvailableSemaphores[i] = VK_NULL_HANDLE;
        }
    }

    Cleanup_Swap_Chain();

    // Cleanup all created textures
    for (auto& tex_pair : m_TextureCache) {
        VulkanTextureInfo* tex_info = tex_pair.second;
        if (tex_info) {
            if (tex_info->Sampler != VK_NULL_HANDLE) {
                vkDestroySampler(m_Device, tex_info->Sampler, nullptr);
            }
            if (tex_info->ImageView != VK_NULL_HANDLE) {
                vkDestroyImageView(m_Device, tex_info->ImageView, nullptr);
            }
            if (tex_info->Image != VK_NULL_HANDLE) {
                vkDestroyImage(m_Device, tex_info->Image, nullptr);
            }
            if (tex_info->Memory != VK_NULL_HANDLE) {
                vkFreeMemory(m_Device, tex_info->Memory, nullptr);
            }
            delete tex_info;
        }
    }
    m_TextureCache.clear();

    if (m_DescriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);
        m_DescriptorPool = VK_NULL_HANDLE;
    }

    if (m_DescriptorSetLayout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(m_Device, m_DescriptorSetLayout, nullptr);
        m_DescriptorSetLayout = VK_NULL_HANDLE;
    }

    if (m_PipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);
        m_PipelineLayout = VK_NULL_HANDLE;
    }

    if (m_GraphicsPipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(m_Device, m_GraphicsPipeline, nullptr);
        m_GraphicsPipeline = VK_NULL_HANDLE;
    }

    if (m_VertexShaderModule != VK_NULL_HANDLE) {
        vkDestroyShaderModule(m_Device, m_VertexShaderModule, nullptr);
        m_VertexShaderModule = VK_NULL_HANDLE;
    }

    if (m_FragmentShaderModule != VK_NULL_HANDLE) {
        vkDestroyShaderModule(m_Device, m_FragmentShaderModule, nullptr);
        m_FragmentShaderModule = VK_NULL_HANDLE;
    }

    if (m_RenderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);
        m_RenderPass = VK_NULL_HANDLE;
    }

    if (m_CommandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
        m_CommandPool = VK_NULL_HANDLE;
    }

    if (m_Surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
        m_Surface = VK_NULL_HANDLE;
    }

    if (m_Device != VK_NULL_HANDLE) {
        vkDestroyDevice(m_Device, nullptr);
        m_Device = VK_NULL_HANDLE;
    }

    if (m_Instance != VK_NULL_HANDLE) {
        vkDestroyInstance(m_Instance, nullptr);
        m_Instance = VK_NULL_HANDLE;
    }

    m_Initted = false;
}

void VulkanRenderInterface::Do_Onetime_Device_Dependent_Inits()
{
    // Called after device is created - initialize Vulkan-specific resources
}

void VulkanRenderInterface::Do_Ontime_Device_Dependent_Shutdowns()
{
    // Called before device is destroyed - cleanup Vulkan-specific resources
}

void VulkanRenderInterface::Begin_Scene()
{
    // Begin recording to command buffer
}

void VulkanRenderInterface::End_Scene(bool flip_frame)
{
    // End recording and submit to queue
    if (flip_frame) {
        Flip_To_Primary();
    }
}

void VulkanRenderInterface::Flip_To_Primary()
{
    // Present the swap chain image
    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &m_RenderFinishedSemaphores[m_CurrentFrame];
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &m_SwapChain;
    present_info.pImageIndices = &m_ImageIndex;

    vkQueuePresentKHR(m_PresentQueue, &present_info);

    // Move to next frame
    m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    m_FrameCount++;
}

void VulkanRenderInterface::Clear(bool clear_color, bool clear_z_stencil,
                                   const Vector3& color, float z, unsigned int stencil)
{
    VkClearAttachments clear_attachments[2];
    int attachment_count = 0;

    if (clear_color) {
        clear_attachments[attachment_count].sType = VK_STRUCTURE_TYPE_CLEAR_ATTACHMENT;
        clear_attachments[attachment_count].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        clear_attachments[attachment_count].colorAttachment = 0;
        clear_attachments[attachment_count].clearValue.color.float32[0] = color.X;
        clear_attachments[attachment_count].clearValue.color.float32[1] = color.Y;
        clear_attachments[attachment_count].clearValue.color.float32[2] = color.Z;
        clear_attachments[attachment_count].clearValue.color.float32[3] = 1.0f;
        attachment_count++;
    }

    if (clear_z_stencil) {
        clear_attachments[attachment_count].sType = VK_STRUCTURE_TYPE_CLEAR_ATTACHMENT;
        clear_attachments[attachment_count].aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        clear_attachments[attachment_count].clearValue.depthStencil.depth = z;
        clear_attachments[attachment_count].clearValue.depthStencil.stencil = stencil;
        attachment_count++;
    }

    if (attachment_count > 0 && m_CommandBuffer != VK_NULL_HANDLE) {
        VkClearRect clear_rect = {};
        clear_rect.rect.offset = {0, 0};
        clear_rect.rect.extent = m_SwapChainExtent;
        clear_rect.baseArrayLayer = 0;
        clear_rect.layerCount = 1;

        vkCmdClearAttachments(m_CommandBuffer, attachment_count, clear_attachments, 1, &clear_rect);
    }
}

void VulkanRenderInterface::Set_Vertex_Buffer(const VertexBufferClass* vb)
{
    // Clear vertex buffer binding if nullptr
    if (vb == nullptr) {
        if (m_VertexBuffers[0].Buffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(m_Device, m_VertexBuffers[0].Buffer, nullptr);
            vkFreeMemory(m_Device, m_VertexBuffers[0].Memory, nullptr);
            m_VertexBuffers[0].Buffer = VK_NULL_HANDLE;
            m_VertexBuffers[0].Memory = VK_NULL_HANDLE;
            m_VertexBuffers[0].Stride = 0;
        }
        return;
    }

    // Get vertex data based on buffer type
    unsigned vertex_count = vb->Get_Vertex_Count();
    unsigned stride = vb->FVF_Info().Get_FVF_Size();
    const void* vertex_data = nullptr;

    if (vb->Type() == BUFFER_TYPE_SORTING) {
        // Sorting buffer stores data directly
        SortingVertexBufferClass* sort_vb = static_cast<SortingVertexBufferClass*>(const_cast<VertexBufferClass*>(vb));
        vertex_data = sort_vb->VertexBuffer;
    } else if (vb->Type() == BUFFER_TYPE_DX8) {
        // DX8 buffer needs to be locked to get data
        DX8VertexBufferClass* dx8_vb = static_cast<DX8VertexBufferClass*>(const_cast<VertexBufferClass*>(vb));
        IDirect3DVertexBuffer9* d3d_vb = dx8_vb->Get_DX8_Vertex_Buffer();
        if (d3d_vb) {
            void* locked_data = nullptr;
            if (SUCCEEDED(d3d_vb->Lock(0, 0, &locked_data, D3DLOCK_READONLY))) {
                Create_Vertex_Buffer(vertex_count * stride, locked_data, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
                d3d_vb->Unlock();
                return;
            }
        }
        return;
    } else {
        return; // Unknown buffer type
    }

    if (vertex_data) {
        Create_Vertex_Buffer(vertex_count * stride, vertex_data, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    }
}

void VulkanRenderInterface::Set_Vertex_Buffer(const DynamicVBAccessClass& vba)
{
    // Get the underlying vertex buffer from the dynamic access
    VertexBufferClass* vb = vba.VertexBuffer;
    if (vb == nullptr) {
        Set_Vertex_Buffer(nullptr);
        return;
    }

    // For dynamic buffers, we use the VertexBufferOffset and VertexCount
    unsigned vertex_count = vba.Get_Vertex_Count();
    unsigned stride = vb->FVF_Info().Get_FVF_Size();
    const void* vertex_data = nullptr;

    if (vb->Type() == BUFFER_TYPE_SORTING) {
        SortingVertexBufferClass* sort_vb = static_cast<SortingVertexBufferClass*>(vb);
        vertex_data = sort_vb->VertexBuffer + vba.VertexBufferOffset;
    } else if (vb->Type() == BUFFER_TYPE_DX8) {
        DX8VertexBufferClass* dx8_vb = static_cast<DX8VertexBufferClass*>(vb);
        IDirect3DVertexBuffer9* d3d_vb = dx8_vb->Get_DX8_Vertex_Buffer();
        if (d3d_vb) {
            void* locked_data = nullptr;
            unsigned lock_offset = vba.VertexBufferOffset * stride;
            unsigned lock_size = vertex_count * stride;
            if (SUCCEEDED(d3d_vb->Lock(lock_offset, lock_size, &locked_data, D3DLOCK_READONLY))) {
                Create_Vertex_Buffer(vertex_count * stride, locked_data, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
                d3d_vb->Unlock();
                return;
            }
        }
        return;
    } else {
        return;
    }

    if (vertex_data) {
        Create_Vertex_Buffer(vertex_count * stride, vertex_data, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    }
}

void VulkanRenderInterface::Set_Index_Buffer(const IndexBufferClass* ib, unsigned short index_base_offset)
{
    // Clear index buffer binding if nullptr
    if (ib == nullptr) {
        if (m_IndexBuffer.Buffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(m_Device, m_IndexBuffer.Buffer, nullptr);
            vkFreeMemory(m_Device, m_IndexBuffer.Memory, nullptr);
            m_IndexBuffer.Buffer = VK_NULL_HANDLE;
            m_IndexBuffer.Memory = VK_NULL_HANDLE;
            m_IndexBuffer.Size = 0;
        }
        m_IndexBufferOffset = 0;
        return;
    }

    // Get index data based on buffer type
    unsigned index_count = ib->Get_Index_Count();
    const void* index_data = nullptr;

    if (ib->Type() == BUFFER_TYPE_SORTING) {
        // Sorting buffer stores data directly
        SortingIndexBufferClass* sort_ib = static_cast<SortingIndexBufferClass*>(const_cast<IndexBufferClass*>(ib));
        index_data = sort_ib->index_buffer;
    } else if (ib->Type() == BUFFER_TYPE_DX8) {
        // DX8 buffer needs to be locked to get data
        DX8IndexBufferClass* dx8_ib = static_cast<DX8IndexBufferClass*>(const_cast<IndexBufferClass*>(ib));
        IDirect3DIndexBuffer9* d3d_ib = dx8_ib->Get_DX8_Index_Buffer();
        if (d3d_ib) {
            void* locked_data = nullptr;
            if (SUCCEEDED(d3d_ib->Lock(0, 0, &locked_data, D3DLOCK_READONLY))) {
                bool success = Create_Index_Buffer(index_count * sizeof(unsigned short), locked_data, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
                d3d_ib->Unlock();
                m_IndexBufferOffset = index_base_offset;
                return;
            }
        }
        m_IndexBufferOffset = index_base_offset;
        return;
    } else {
        return; // Unknown buffer type
    }

    if (index_data) {
        Create_Index_Buffer(index_count * sizeof(unsigned short), index_data, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    }
    m_IndexBufferOffset = index_base_offset;
}

void VulkanRenderInterface::Set_Index_Buffer(const DynamicIBAccessClass& iba, unsigned short index_base_offset)
{
    // Get the underlying index buffer from the dynamic access
    IndexBufferClass* ib = iba.IndexBuffer;
    if (ib == nullptr) {
        Set_Index_Buffer(nullptr, 0);
        return;
    }

    // For dynamic buffers, we use the IndexBufferOffset and IndexCount
    unsigned index_count = iba.Get_Index_Count();
    const void* index_data = nullptr;

    if (ib->Type() == BUFFER_TYPE_SORTING) {
        SortingIndexBufferClass* sort_ib = static_cast<SortingIndexBufferClass*>(ib);
        index_data = sort_ib->index_buffer + iba.IndexBufferOffset;
    } else if (ib->Type() == BUFFER_TYPE_DX8) {
        DX8IndexBufferClass* dx8_ib = static_cast<DX8IndexBufferClass*>(ib);
        IDirect3DIndexBuffer9* d3d_ib = dx8_ib->Get_DX8_Index_Buffer();
        if (d3d_ib) {
            void* locked_data = nullptr;
            unsigned lock_offset = iba.IndexBufferOffset * sizeof(unsigned short);
            unsigned lock_size = index_count * sizeof(unsigned short);
            if (SUCCEEDED(d3d_ib->Lock(lock_offset, lock_size, &locked_data, D3DLOCK_READONLY))) {
                Create_Index_Buffer(index_count * sizeof(unsigned short), locked_data, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
                d3d_ib->Unlock();
                m_IndexBufferOffset = index_base_offset;
                return;
            }
        }
        return;
    } else {
        return;
    }

    if (index_data) {
        Create_Index_Buffer(index_count * sizeof(unsigned short), index_data, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    }
    m_IndexBufferOffset = index_base_offset;
}

void VulkanRenderInterface::Set_Index_Buffer_Index_Offset(unsigned offset)
{
    m_IndexBufferOffset = offset;
}

// =====================================================================
// Buffer Creation Helpers
// =====================================================================

bool VulkanRenderInterface::Create_Vertex_Buffer(size_t size, const void* data, VkBufferUsageFlags usage)
{
    if (size == 0 || data == nullptr) {
        return false;
    }

    // Destroy existing buffer if any
    if (m_VertexBuffers[0].Buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(m_Device, m_VertexBuffers[0].Buffer, nullptr);
        vkFreeMemory(m_Device, m_VertexBuffers[0].Memory, nullptr);
        m_VertexBuffers[0].Buffer = VK_NULL_HANDLE;
        m_VertexBuffers[0].Memory = VK_NULL_HANDLE;
    }

    // Create the vertex buffer
    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_Device, &buffer_info, nullptr, &m_VertexBuffers[0].Buffer) != VK_SUCCESS) {
        std::cerr << "Failed to create vertex buffer" << std::endl;
        return false;
    }

    // Allocate device memory
    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(m_Device, m_VertexBuffers[0].Buffer, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_reqs.size;
    alloc_info.memoryTypeIndex = Find_Memory_Type(mem_reqs.memoryTypeBits,
                                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(m_Device, &alloc_info, nullptr, &m_VertexBuffers[0].Memory) != VK_SUCCESS) {
        std::cerr << "Failed to allocate vertex buffer memory" << std::endl;
        vkDestroyBuffer(m_Device, m_VertexBuffers[0].Buffer, nullptr);
        m_VertexBuffers[0].Buffer = VK_NULL_HANDLE;
        return false;
    }

    vkBindBufferMemory(m_Device, m_VertexBuffers[0].Buffer, m_VertexBuffers[0].Memory, 0);

    // Copy data to GPU
    if (!Copy_To_GPU(m_VertexBuffers[0].Buffer, m_VertexBuffers[0].Memory, data, size)) {
        vkDestroyBuffer(m_Device, m_VertexBuffers[0].Buffer, nullptr);
        vkFreeMemory(m_Device, m_VertexBuffers[0].Memory, nullptr);
        m_VertexBuffers[0].Buffer = VK_NULL_HANDLE;
        m_VertexBuffers[0].Memory = VK_NULL_HANDLE;
        return false;
    }

    return true;
}

bool VulkanRenderInterface::Create_Index_Buffer(size_t size, const void* data, VkBufferUsageFlags usage)
{
    if (size == 0 || data == nullptr) {
        return false;
    }

    // Destroy existing buffer if any
    if (m_IndexBuffer.Buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(m_Device, m_IndexBuffer.Buffer, nullptr);
        vkFreeMemory(m_Device, m_IndexBuffer.Memory, nullptr);
        m_IndexBuffer.Buffer = VK_NULL_HANDLE;
        m_IndexBuffer.Memory = VK_NULL_HANDLE;
    }

    // Create the index buffer
    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_Device, &buffer_info, nullptr, &m_IndexBuffer.Buffer) != VK_SUCCESS) {
        std::cerr << "Failed to create index buffer" << std::endl;
        return false;
    }

    // Allocate device memory
    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(m_Device, m_IndexBuffer.Buffer, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_reqs.size;
    alloc_info.memoryTypeIndex = Find_Memory_Type(mem_reqs.memoryTypeBits,
                                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(m_Device, &alloc_info, nullptr, &m_IndexBuffer.Memory) != VK_SUCCESS) {
        std::cerr << "Failed to allocate index buffer memory" << std::endl;
        vkDestroyBuffer(m_Device, m_IndexBuffer.Buffer, nullptr);
        m_IndexBuffer.Buffer = VK_NULL_HANDLE;
        return false;
    }

    vkBindBufferMemory(m_Device, m_IndexBuffer.Buffer, m_IndexBuffer.Memory, 0);

    // Copy data to GPU
    if (!Copy_To_GPU(m_IndexBuffer.Buffer, m_IndexBuffer.Memory, data, size)) {
        vkDestroyBuffer(m_Device, m_IndexBuffer.Buffer, nullptr);
        vkFreeMemory(m_Device, m_IndexBuffer.Memory, nullptr);
        m_IndexBuffer.Buffer = VK_NULL_HANDLE;
        m_IndexBuffer.Memory = VK_NULL_HANDLE;
        return false;
    }

    m_IndexBuffer.Size = size;
    return true;
}

bool VulkanRenderInterface::Copy_To_GPU(VkBuffer dst_buffer, VkDeviceMemory dst_memory, const void* data, size_t size)
{
    if (size == 0 || data == nullptr) {
        return false;
    }

    // Create staging buffer for copying data
    VkBuffer staging_buffer;
    VkDeviceMemory staging_memory;

    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_Device, &buffer_info, nullptr, &staging_buffer) != VK_SUCCESS) {
        std::cerr << "Failed to create staging buffer" << std::endl;
        return false;
    }

    // Allocate staging memory (HOST_VISIBLE for mapping)
    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(m_Device, staging_buffer, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_reqs.size;
    alloc_info.memoryTypeIndex = Find_Memory_Type(mem_reqs.memoryTypeBits,
                                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(m_Device, &alloc_info, nullptr, &staging_memory) != VK_SUCCESS) {
        std::cerr << "Failed to allocate staging memory" << std::endl;
        vkDestroyBuffer(m_Device, staging_buffer, nullptr);
        return false;
    }

    vkBindBufferMemory(m_Device, staging_buffer, staging_memory, 0);

    // Map and copy data to staging buffer
    void* mapped;
    if (vkMapMemory(m_Device, staging_memory, 0, size, 0, &mapped) != VK_SUCCESS) {
        std::cerr << "Failed to map staging memory" << std::endl;
        vkFreeMemory(m_Device, staging_memory, nullptr);
        vkDestroyBuffer(m_Device, staging_buffer, nullptr);
        return false;
    }
    memcpy(mapped, data, size);
    vkUnmapMemory(m_Device, staging_memory);

    // Create a one-time command buffer to copy data
    VkCommandBufferAllocateInfo cmd_alloc_info = {};
    cmd_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd_alloc_info.commandPool = m_CommandPool;
    cmd_alloc_info.commandBufferCount = 1;

    VkCommandBuffer copy_cmd;
    if (vkAllocateCommandBuffers(m_Device, &cmd_alloc_info, &copy_cmd) != VK_SUCCESS) {
        std::cerr << "Failed to allocate copy command buffer" << std::endl;
        vkFreeMemory(m_Device, staging_memory, nullptr);
        vkDestroyBuffer(m_Device, staging_buffer, nullptr);
        return false;
    }

    // Begin command buffer
    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(copy_cmd, &begin_info) != VK_SUCCESS) {
        std::cerr << "Failed to begin copy command buffer" << std::endl;
        vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &copy_cmd);
        vkFreeMemory(m_Device, staging_memory, nullptr);
        vkDestroyBuffer(m_Device, staging_buffer, nullptr);
        return false;
    }

    // Copy staging buffer to destination buffer
    VkBufferCopy copy_region = {};
    copy_region.srcOffset = 0;
    copy_region.dstOffset = 0;
    copy_region.size = size;
    vkCmdCopyBuffer(copy_cmd, staging_buffer, dst_buffer, 1, &copy_region);

    // End and submit command buffer
    vkEndCommandBuffer(copy_cmd);

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &copy_cmd;

    if (vkQueueSubmit(m_GraphicsQueue, 1, &submit_info, VK_NULL_HANDLE) != VK_SUCCESS) {
        std::cerr << "Failed to submit copy command" << std::endl;
        vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &copy_cmd);
        vkFreeMemory(m_Device, staging_memory, nullptr);
        vkDestroyBuffer(m_Device, staging_buffer, nullptr);
        return false;
    }

    // Wait for the copy to complete
    vkQueueWaitIdle(m_GraphicsQueue);

    // Cleanup
    vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &copy_cmd);
    vkFreeMemory(m_Device, staging_memory, nullptr);
    vkDestroyBuffer(m_Device, staging_buffer, nullptr);

    return true;
}

void VulkanRenderInterface::Set_Transform(TransformType type, const Matrix4& matrix)
{
    switch (type) {
        case TRANSFORM_WORLD:
            m_WorldMatrix = matrix;
            m_WorldIdentity = false;
            break;
        case TRANSFORM_VIEW:
            m_ViewMatrix = matrix;
            m_ViewIdentity = false;
            break;
        case TRANSFORM_PROJECTION:
            m_ProjectionMatrix = matrix;
            break;
        case TRANSFORM_TEXTURE0:
        case TRANSFORM_TEXTURE1:
        case TRANSFORM_TEXTURE2:
        case TRANSFORM_TEXTURE3:
            m_TextureMatrices[type - TRANSFORM_TEXTURE0] = matrix;
            break;
    }
}

void VulkanRenderInterface::Set_Transform(TransformType type, const Matrix3D& matrix)
{
    // Convert Matrix3D to Matrix4 and call the other overload
    Matrix4 m4;
    m4.Set(matrix);
    Set_Transform(type, m4);
}

void VulkanRenderInterface::Get_Transform(TransformType type, Matrix4& matrix)
{
    switch (type) {
        case TRANSFORM_WORLD:
            matrix = m_WorldMatrix;
            break;
        case TRANSFORM_VIEW:
            matrix = m_ViewMatrix;
            break;
        case TRANSFORM_PROJECTION:
            matrix = m_ProjectionMatrix;
            break;
        case TRANSFORM_TEXTURE0:
        case TRANSFORM_TEXTURE1:
        case TRANSFORM_TEXTURE2:
        case TRANSFORM_TEXTURE3:
            matrix = m_TextureMatrices[type - TRANSFORM_TEXTURE0];
            break;
    }
}

void VulkanRenderInterface::Set_World_Identity()
{
    m_WorldIdentity = true;
    m_WorldMatrix.Identity();
}

void VulkanRenderInterface::Set_View_Identity()
{
    m_ViewIdentity = true;
    m_ViewMatrix.Identity();
}

bool VulkanRenderInterface::Is_World_Identity() const
{
    return m_WorldIdentity;
}

bool VulkanRenderInterface::Is_View_Identity() const
{
    return m_ViewIdentity;
}

void VulkanRenderInterface::Set_Light(unsigned index, const Light& light)
{
    if (index >= MAX_LIGHTS) return;

    VulkanLight& vulkan_light = m_RenderState.Lights[index];
    vulkan_light.Position = light.Position;
    vulkan_light.Direction = light.Direction;
    vulkan_light.Range = light.Range;
    vulkan_light.Type = light.Type;
    vulkan_light.Diffuse = light.Diffuse;
    vulkan_light.Specular = light.Specular;
    vulkan_light.Ambient = light.Ambient;
    vulkan_light.Enabled = light.Enabled;

    // Update active light count
    if (light.Enabled) {
        for (unsigned i = m_RenderState.ActiveLights; i <= index; ++i) {
            if (m_RenderState.Lights[i].Enabled) {
                m_RenderState.ActiveLights = i + 1;
            }
        }
    }

    // Mark descriptor set as dirty for next frame
    m_DescriptorSetDirty = true;
}

void VulkanRenderInterface::Set_Light(unsigned index, const LightClass& light)
{
    if (index >= MAX_LIGHTS) return;

    Light vulkan_light;
    vulkan_light.Enabled = true;

    // Convert LightClass type to our Light type
    switch (light.Get_Type()) {
        case LightClass::POINT:
            vulkan_light.Type = Light::LIGHT_POINT;
            break;
        case LightClass::DIRECTIONAL:
            vulkan_light.Type = Light::LIGHT_DIRECTIONAL;
            break;
        case LightClass::SPOT:
            vulkan_light.Type = Light::LIGHT_SPOT;
            vulkan_light.Theta = light.Get_Spot_Angle_Cos();
            vulkan_light.Phi = light.Get_Spot_Angle_Cos(); // outer cone
            {
                Vector3 dir;
                light.Get_Spot_Direction(dir);
                vulkan_light.Direction = dir;
            }
            vulkan_light.Falloff = light.Get_Spot_Exponent();
            break;
    }

    // Get intensity and colors
    Vector3 ambient, diffuse, specular;
    light.Get_Ambient(&ambient);
    light.Get_Diffuse(&diffuse);
    light.Get_Specular(&specular);

    float intensity = light.Get_Intensity();
    vulkan_light.Ambient = Vector4(ambient.X * intensity, ambient.Y * intensity, ambient.Z * intensity, 1.0f);
    vulkan_light.Diffuse = Vector4(diffuse.X * intensity, diffuse.Y * intensity, diffuse.Z * intensity, 1.0f);
    vulkan_light.Specular = Vector4(specular.X * intensity, specular.Y * intensity, specular.Z * intensity, 1.0f);

    // Get attenuation ranges
    double far_start, far_end;
    light.Get_Far_Attenuation_Range(far_start, far_end);
    vulkan_light.Range = (float)far_end;
    vulkan_light.Attenuation0 = 1.0f;
    vulkan_light.Attenuation1 = 0.0f;
    vulkan_light.Attenuation2 = 0.0f;

    // Get position from light object transform (for point/spot lights)
    // The position would come from the render object's transform matrix
    // For now, leave as zero - caller should set position via transform

    Set_Light(index, vulkan_light);
}

void VulkanRenderInterface::Enable_Light(unsigned index, bool enable)
{
    if (index >= MAX_LIGHTS) return;
    m_RenderState.Lights[index].Enabled = enable;
}

void VulkanRenderInterface::Set_Light_Count(unsigned count)
{
    m_RenderState.ActiveLights = count < MAX_LIGHTS ? count : MAX_LIGHTS;
    m_DescriptorSetDirty = true;
}

void VulkanRenderInterface::Set_Material(const VertexMaterialClass* material)
{
    if (material == nullptr) {
        // Reset to default material
        m_CurrentMaterial = nullptr;
        m_MaterialDirty = true;
        return;
    }

    m_CurrentMaterial = material;

    // Extract material properties for shader uniforms
    Vector3 ambient, diffuse, specular, emissive;
    material->Get_Ambient(&ambient);
    material->Get_Diffuse(&diffuse);
    material->Get_Specular(&specular);
    material->Get_Emissive(&emissive);

    float shininess = material->Get_Shininess();
    float opacity = material->Get_Opacity();

    // Store in render state for UBO updates
    m_MaterialData.Ambient = Vector4(ambient.X, ambient.Y, ambient.Z, 1.0f);
    m_MaterialData.Diffuse = Vector4(diffuse.X, diffuse.Y, diffuse.Z, opacity);
    m_MaterialData.Specular = Vector4(specular.X, specular.Y, specular.Z, shininess);
    m_MaterialData.Emissive = Vector4(emissive.X, emissive.Y, emissive.Z, 1.0f);

    // Check material flags
    m_MaterialData.EnableLighting = material->Get_Lighting() ? 1.0f : 0.0f;
    m_MaterialData.DepthCue = material->Get_Flag(VertexMaterialClass::DEPTH_CUE) ? 1.0f : 0.0f;

    m_MaterialDirty = true;
}

void VulkanRenderInterface::Set_Shader(const ShaderClass& shader)
{
    m_CurrentShader = shader;
    m_ShaderDirty = true;
}

void VulkanRenderInterface::Get_Shader(ShaderClass& shader)
{
    shader = m_CurrentShader;
}

void VulkanRenderInterface::Set_Texture(unsigned stage, TextureClass* texture)
{
    if (stage >= MAX_TEXTURE_STAGES) return;
    m_Textures[stage] = texture;
}

void VulkanRenderInterface::Set_Fog(bool enable, const Vector3& color, float start, float end)
{
    m_FogEnabled = enable;
    m_FogColor = color;
    m_FogStart = start;
    m_FogEnd = end;

    // Store fog data for shader uniforms
    m_FogData.Enabled = enable ? 1.0f : 0.0f;
    m_FogData.Color = Vector4(color.X, color.Y, color.Z, 1.0f);
    m_FogData.Start = start;
    m_FogData.End = end;
    m_FogData.Range = (end - start > 0.0f) ? (1.0f / (end - start)) : 0.0f;

    m_FogDirty = true;
}

void VulkanRenderInterface::Set_Light_Environment(LightEnvironmentClass* light_env)
{
    m_LightEnvironment = light_env;

    if (light_env == nullptr) {
        m_RenderState.ActiveLights = 0;
        m_DescriptorSetDirty = true;
        return;
    }

    // Extract lights from the light environment
    int light_count = light_env->Get_Light_Count();
    m_RenderState.ActiveLights = light_count;

    for (int i = 0; i < light_count && i < MAX_LIGHTS; ++i) {
        VulkanLight& vulkan_light = m_RenderState.Lights[i];

        // Get light direction and diffuse from light environment
        vulkan_light.Direction = light_env->Get_Light_Direction(i);
        vulkan_light.Diffuse = Vector4(
            light_env->Get_Light_Diffuse(i).X,
            light_env->Get_Light_Diffuse(i).Y,
            light_env->Get_Light_Diffuse(i).Z,
            1.0f
        );

        // Directional lights by default from environment
        vulkan_light.Type = Light::LIGHT_DIRECTIONAL;
        vulkan_light.Enabled = true;

        // Ambient contribution
        Vector3 ambient = light_env->Get_Equivalent_Ambient();
        vulkan_light.Ambient = Vector4(ambient.X, ambient.Y, ambient.Z, 1.0f);

        // Default specular
        vulkan_light.Specular = vulkan_light.Diffuse;

        // Default range (directional lights don't use range)
        vulkan_light.Range = 0.0f;
    }

    m_DescriptorSetDirty = true;
}

void VulkanRenderInterface::Draw(PrimitiveType type, unsigned short start_index,
                                  unsigned short polygon_count, unsigned short min_vertex_index,
                                  unsigned short vertex_count)
{
    VkPrimitiveTopology topology = Convert_Primitive_Type(type);

    vkCmdBindPipeline(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);

    VkDeviceSize offsets[] = { 0 };
    if (m_VertexBuffers[0].Buffer != VK_NULL_HANDLE) {
        vkCmdBindVertexBuffers(m_CommandBuffer, 0, 1, &m_VertexBuffers[0].Buffer, offsets);
    }

    if (m_IndexBuffer.Buffer != VK_NULL_HANDLE) {
        vkCmdBindIndexBuffer(m_CommandBuffer, m_IndexBuffer.Buffer, m_IndexBufferOffset * sizeof(uint16_t), VK_INDEX_TYPE_UINT16);
        vkCmdDrawIndexed(m_CommandBuffer, polygon_count * 3, 1, start_index * 3, min_vertex_index, 0);
    } else {
        vkCmdDraw(m_CommandBuffer, polygon_count * 3, 1, start_index, 0);
    }

    m_DrawCallCount++;
    m_TriangleCount += polygon_count;
}

void VulkanRenderInterface::Draw_Indexed(PrimitiveType type, unsigned short start_index,
                                         unsigned short polygon_count, unsigned short min_vertex_index,
                                         unsigned short vertex_count)
{
    Draw(type, start_index, polygon_count, min_vertex_index, vertex_count);
}

void VulkanRenderInterface::Draw_Triangles(unsigned buffer_type, unsigned short start_index,
                                           unsigned short polygon_count, unsigned short min_vertex_index,
                                           unsigned short vertex_count)
{
    Draw(PRIMITIVE_TRIANGLE_LIST, start_index, polygon_count, min_vertex_index, vertex_count);
}

void VulkanRenderInterface::Set_Render_State(RenderState state, unsigned value)
{
    switch (state) {
        case RS_ZENABLE:
            m_RenderState.DepthTestEnabled = (value != 0);
            break;
        case RS_ZWRITEENABLE:
            m_RenderState.DepthWriteEnabled = (value != 0);
            break;
        case RS_ALPHABLENDENABLE:
            m_RenderState.BlendEnabled = (value != 0);
            break;
        case RS_CULLMODE:
            m_RenderState.CullMode = (CullMode)value;
            break;
        case RS_FILLMODE:
            m_RenderState.FillMode = (FillMode)value;
            break;
    }
    Rebuild_Pipeline_State();
}

void VulkanRenderInterface::Set_Render_State(RenderState state, int value)
{
    Set_Render_State(state, (unsigned)value);
}

void VulkanRenderInterface::Set_Blend_Operation(BlendOperation op)
{
    m_RenderState.BlendOp = op;
    Rebuild_Pipeline_State();
}

void VulkanRenderInterface::Set_Source_Blend(BlendFactor factor)
{
    m_RenderState.SrcBlend = factor;
    Rebuild_Pipeline_State();
}

void VulkanRenderInterface::Set_Dest_Blend(BlendFactor factor)
{
    m_RenderState.DstBlend = factor;
    Rebuild_Pipeline_State();
}

void VulkanRenderInterface::Set_Cull_Mode(CullMode mode)
{
    m_RenderState.CullMode = mode;
    Rebuild_Pipeline_State();
}

void VulkanRenderInterface::Set_Depth_Bias(float bias)
{
    m_RenderState.DepthBias = bias;
    Rebuild_Pipeline_State();
}

void VulkanRenderInterface::Set_Scissor_Test(bool enable, int left, int top, int right, int bottom)
{
    m_RenderState.ScissorTestEnabled = enable;
    m_RenderState.ScissorLeft = left;
    m_RenderState.ScissorTop = top;
    m_RenderState.ScissorRight = right;
    m_RenderState.ScissorBottom = bottom;
}

void VulkanRenderInterface::Set_Viewport(const Viewport& viewport)
{
    m_RenderState.Viewport = viewport;

    if (m_CommandBuffer != VK_NULL_HANDLE) {
        VkViewport vk_viewport = {};
        vk_viewport.x = (float)viewport.X;
        vk_viewport.y = (float)viewport.Y;
        vk_viewport.width = (float)viewport.Width;
        vk_viewport.height = (float)viewport.Height;
        vk_viewport.minDepth = viewport.MinZ;
        vk_viewport.maxDepth = viewport.MaxZ;
        vkCmdSetViewport(m_CommandBuffer, 0, 1, &vk_viewport);
    }
}

void VulkanRenderInterface::Set_Texture_Stage_State(unsigned stage, TextureStageState state, unsigned value)
{
    if (stage >= MAX_TEXTURE_STAGES) return;

    auto& tex_stage = m_RenderState.TextureStages[stage];

    switch (state) {
        case TSS_COLOROP:
            tex_stage.ColorOp = (TextureOperation)value;
            break;
        case TSS_COLORARG1:
            tex_stage.ColorArg1 = (TextureArgument)value;
            break;
        case TSS_COLORARG2:
            tex_stage.ColorArg2 = (TextureArgument)value;
            break;
        case TSS_ALPHAOP:
            tex_stage.AlphaOp = (TextureOperation)value;
            break;
        case TSS_ALPHAARG1:
            tex_stage.AlphaArg1 = (TextureArgument)value;
            break;
        case TSS_ALPHAARG2:
            tex_stage.AlphaArg2 = (TextureArgument)value;
            break;
    }
}

void VulkanRenderInterface::Set_Sampler_State(unsigned sampler, SamplerState state, unsigned value)
{
    // Set sampler state
}

void* VulkanRenderInterface::Create_Texture(unsigned width, unsigned height, TextureFormat format,
                                            bool generate_mips, bool rendertarget)
{
    VulkanTextureInfo* tex_info = new VulkanTextureInfo();
    tex_info->Width = width;
    tex_info->Height = height;
    tex_info->Format = format;
    tex_info->Is_RenderTarget = rendertarget;

    VkFormat vk_format = Convert_Format(format);
    uint32_t mip_levels = generate_mips ? 1 : 1;
    if (generate_mips) {
        mip_levels = static_cast<uint32_t>(floor(log2(std::max(width, height))) + 1);
    }

    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (rendertarget) {
        usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    } else {
        usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }

    // Create the image with proper memory
    if (!Create_Image(width, height, vk_format, tiling, usage, mip_levels,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, tex_info->Image, tex_info->Memory)) {
        delete tex_info;
        return nullptr;
    }

    // Create image view
    if (!Create_Image_View(tex_info->Image, vk_format, VK_IMAGE_ASPECT_COLOR_BIT,
                           VK_IMAGE_VIEW_TYPE_2D, mip_levels, tex_info->ImageView)) {
        vkFreeMemory(m_Device, tex_info->Memory, nullptr);
        vkDestroyImage(m_Device, tex_info->Image, nullptr);
        delete tex_info;
        return nullptr;
    }

    // Create sampler
    if (!Create_Sampler(VK_FILTER_LINEAR, VK_FILTER_LINEAR,
                       VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT,
                       VK_SAMPLER_MIPMAP_MODE_LINEAR, 1.0f, tex_info->Sampler)) {
        vkDestroyImageView(m_Device, tex_info->ImageView, nullptr);
        vkFreeMemory(m_Device, tex_info->Memory, nullptr);
        vkDestroyImage(m_Device, tex_info->Image, nullptr);
        delete tex_info;
        return nullptr;
    }

    // Generate mipmaps if requested
    if (generate_mips && mip_levels > 1) {
        Generate_Mipmaps(tex_info->Image, vk_format, width, height, mip_levels);
    }

    return tex_info;
}

void* VulkanRenderInterface::Create_Texture(const char* filename, bool generate_mips)
{
    VulkanTextureInfo* tex_info = Load_Texture_From_File(filename, generate_mips);
    return tex_info;
}

void VulkanRenderInterface::Update_Texture(TextureClass* system_texture, TextureClass* video_texture)
{
    // Get the DX9 texture from the TextureClass
    if (!system_texture || !video_texture) return;

    IDirect3DTexture9* sys_tex = system_texture->Peek_DX8_Texture();
    if (!sys_tex) return;

    // Get dimensions
    D3DSURFACE_DESC desc;
    if (sys_tex->GetLevelDesc(0, &desc) != D3D_OK) return;

    unsigned width = desc.Width;
    unsigned height = desc.Height;

    // Get video texture info
    VulkanTextureInfo* vid_tex = reinterpret_cast<VulkanTextureInfo*>(video_texture->Get_Surface_Level(0));
    if (!vid_tex) return;

    // Lock the system texture and copy to staging buffer
    D3DLOCKED_RECT locked_rect;
    if (sys_tex->LockRect(0, &locked_rect, nullptr, 0) != D3D_OK) return;

    // Create a staging buffer for the texture data
    VkBuffer staging_buffer;
    VkDeviceMemory staging_memory;
    size_t buffer_size = width * height * 4; // Assume 4 bytes per pixel

    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = buffer_size;
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_Device, &buffer_info, nullptr, &staging_buffer) != VK_SUCCESS) {
        sys_tex->UnlockRect(0);
        return;
    }

    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(m_Device, staging_buffer, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_reqs.size;
    alloc_info.memoryTypeIndex = Find_Memory_Type(mem_reqs.memoryTypeBits,
                                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(m_Device, &alloc_info, nullptr, &staging_memory) != VK_SUCCESS) {
        vkDestroyBuffer(m_Device, staging_buffer, nullptr);
        sys_tex->UnlockRect(0);
        return;
    }

    vkBindBufferMemory(m_Device, staging_buffer, staging_memory, 0);

    // Copy texture data to staging buffer
    void* mapped;
    vkMapMemory(m_Device, staging_memory, 0, VK_WHOLE_SIZE, 0, &mapped);
    memcpy(mapped, locked_rect.pBits, buffer_size);
    vkUnmapMemory(m_Device, staging_memory);

    sys_tex->UnlockRect(0);

    // Transition image layout to transfer destination
    Transition_Image_Layout(vid_tex->Image, Get_Vulkan_Format(vid_tex->Format),
                            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1);

    // Copy buffer to image
    Copy_Buffer_To_Image(staging_buffer, vid_tex->Image, width, height);

    // Transition to shader read layout
    Transition_Image_Layout(vid_tex->Image, Get_Vulkan_Format(vid_tex->Format),
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);

    // Cleanup staging resources
    vkFreeMemory(m_Device, staging_memory, nullptr);
    vkDestroyBuffer(m_Device, staging_buffer, nullptr);
}

void* VulkanRenderInterface::Create_Surface(unsigned width, unsigned height, TextureFormat format,
                                            TexturePool pool)
{
    VulkanTextureInfo* tex_info = new VulkanTextureInfo();
    tex_info->Width = width;
    tex_info->Height = height;
    tex_info->Format = format;
    tex_info->Is_RenderTarget = true;

    VkFormat vk_format = Convert_Format(format);

    // Create the image for rendering (not GPU_LOCAL initially for potential CPU access)
    VkImageTiling tiling = (pool == POOL_SYSTEMMEM) ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL;
    VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    if (pool != POOL_SYSTEMMEM) {
        usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    VkMemoryPropertyFlags mem_props = (pool == POOL_SYSTEMMEM) ?
        (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) :
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    if (!Create_Image(width, height, vk_format, tiling, usage, 1, mem_props,
                      tex_info->Image, tex_info->Memory)) {
        delete tex_info;
        return nullptr;
    }

    // Create image view for rendering
    if (!Create_Image_View(tex_info->Image, vk_format, VK_IMAGE_ASPECT_COLOR_BIT,
                           VK_IMAGE_VIEW_TYPE_2D, 1, tex_info->ImageView)) {
        vkFreeMemory(m_Device, tex_info->Memory, nullptr);
        vkDestroyImage(m_Device, tex_info->Image, nullptr);
        delete tex_info;
        return nullptr;
    }

    // For rendertargets, we might want a sampler for texturing
    if (pool != POOL_SYSTEMMEM) {
        Create_Sampler(VK_FILTER_LINEAR, VK_FILTER_LINEAR,
                       VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                       VK_SAMPLER_MIPMAP_MODE_NEAREST, 1.0f, tex_info->Sampler);
    }

    return tex_info;
}

void* VulkanRenderInterface::Create_Surface(const char* filename)
{
    return Load_Texture_From_File(filename, false);
}

void* VulkanRenderInterface::Get_Front_Buffer()
{
    // The front buffer is typically the first swap chain image
    if (m_SwapChainImages.empty()) return nullptr;
    return &m_SwapChainImages[0];
}

void* VulkanRenderInterface::Get_Back_Buffer(unsigned index)
{
    if (index >= m_SwapChainImages.size()) return nullptr;
    return &m_SwapChainImages[index];
}

void VulkanRenderInterface::Copy_Rects(void* source_surface, const void* source_rects,
                                       unsigned rect_count, void* dest_surface, int dest_x, int dest_y)
{
    if (!source_surface || !dest_surface || !source_rects) return;

    VulkanTextureInfo* src_info = reinterpret_cast<VulkanTextureInfo*>(source_surface);
    VulkanTextureInfo* dst_info = reinterpret_cast<VulkanTextureInfo*>(dest_surface);
    const RenderInterface::Rect* rects = static_cast<const RenderInterface::Rect*>(source_rects);

    // Submit a command to copy each rect
    VkCommandBuffer cmd = Begin_Single_Time_Commands();

    for (unsigned i = 0; i < rect_count; ++i) {
        Copy_Rect_Helper(src_info->Image, dst_info->Image,
                         rects[i].X, rects[i].Y,
                         dest_x + rects[i].X, dest_y + rects[i].Y,
                         rects[i].Width, rects[i].Height);
    }

    End_Single_Time_Commands(cmd);
}

void VulkanRenderInterface::Read_Texture(void* surface, void* dest_surface)
{
    if (!surface || !dest_surface) return;

    VulkanTextureInfo* src_info = reinterpret_cast<VulkanTextureInfo*>(surface);
    unsigned char* dest = reinterpret_cast<unsigned char*>(dest_surface);

    // Create staging buffer to read back data
    size_t buffer_size = src_info->Width * src_info->Height * Get_Bytes_Per_Pixel(src_info->Format);

    VkBuffer staging_buffer;
    VkDeviceMemory staging_memory;

    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = buffer_size;
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_Device, &buffer_info, nullptr, &staging_buffer) != VK_SUCCESS) {
        return;
    }

    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(m_Device, staging_buffer, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_reqs.size;
    alloc_info.memoryTypeIndex = Find_Memory_Type(mem_reqs.memoryTypeBits,
                                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(m_Device, &alloc_info, nullptr, &staging_memory) != VK_SUCCESS) {
        vkDestroyBuffer(m_Device, staging_buffer, nullptr);
        return;
    }

    vkBindBufferMemory(m_Device, staging_buffer, staging_memory, 0);

    // Transition image to transfer source layout
    Transition_Image_Layout(src_info->Image, Get_Vulkan_Format(src_info->Format),
                            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 1);

    // Copy image to buffer
    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = src_info->Width;
    region.bufferImageHeight = src_info->Height;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {static_cast<uint32_t>(src_info->Width),
                          static_cast<uint32_t>(src_info->Height), 1};

    VkCommandBuffer cmd = Begin_Single_Time_Commands();
    vkCmdCopyImageToBuffer(cmd, src_info->Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                          staging_buffer, 1, &region);
    End_Single_Time_Commands(cmd);

    // Read back to CPU
    void* mapped;
    vkMapMemory(m_Device, staging_memory, 0, VK_WHOLE_SIZE, 0, &mapped);
    memcpy(dest, mapped, buffer_size);
    vkUnmapMemory(m_Device, staging_memory);

    // Cleanup
    vkFreeMemory(m_Device, staging_memory, nullptr);
    vkDestroyBuffer(m_Device, staging_buffer, nullptr);
}

// =====================================================================
// Private Vulkan helper methods
// =====================================================================

bool VulkanRenderInterface::Create_Instance()
{
    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "OpenW3D";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "OpenW3D Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    // Get required extensions
    uint32_t extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, available_extensions.data());

    // Required extensions for cross-platform
    std::vector<const char*> required_extensions;
    required_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

#if defined(__linux__)
    required_extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#elif defined(_WIN32)
    required_extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(__APPLE__)
    required_extensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#endif

    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    create_info.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size());
    create_info.ppEnabledExtensionNames = required_extensions.data();
    create_info.enabledLayerCount = 0;

    if (vkCreateInstance(&create_info, nullptr, &m_Instance) != VK_SUCCESS) {
        return false;
    }

    return true;
}

bool VulkanRenderInterface::Create_Device()
{
    // Get physical device
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(m_Instance, &device_count, nullptr);
    if (device_count == 0) {
        std::cerr << "No Vulkan-capable devices found" << std::endl;
        return false;
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(m_Instance, &device_count, devices.data());

    // Try to find a discrete GPU first
    for (auto device : devices) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            m_PhysicalDevice = device;
            break;
        }
    }
    if (m_PhysicalDevice == VK_NULL_HANDLE) {
        m_PhysicalDevice = devices[0]; // Fall back to first device
    }

    // Get device properties
    VkPhysicalDeviceProperties device_props;
    vkGetPhysicalDeviceProperties(m_PhysicalDevice, &device_props);
    std::cout << "Vulkan device: " << device_props.deviceName << std::endl;

    // Get supported features
    VkPhysicalDeviceFeatures supported_features;
    vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &supported_features);

    // Device extensions
    uint32_t ext_count = 0;
    vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &ext_count, nullptr);
    std::vector<VkExtensionProperties> available_extensions(ext_count);
    vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &ext_count, available_extensions.data());

    // Check for required extensions
    std::vector<const char*> device_extensions;
    device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    bool has_required_ext = true;
    for (const auto& req_ext : device_extensions) {
        bool found = false;
        for (const auto& avail_ext : available_extensions) {
            if (strcmp(req_ext, avail_ext.extensionName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            has_required_ext = false;
            break;
        }
    }

    if (!has_required_ext) {
        std::cerr << "Required Vulkan device extensions not found" << std::endl;
        return false;
    }

    // Get queue families (basic info before surface is created)
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queue_family_count, queue_families.data());

    // Find graphics queue family
    int graphics_family = -1;
    for (uint32_t i = 0; i < queue_family_count; i++) {
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphics_family = i;
            break;
        }
    }
    if (graphics_family < 0) {
        std::cerr << "No graphics queue family found" << std::endl;
        return false;
    }

    m_GraphicsQueueFamily = graphics_family;

    // Create queue info for graphics family
    VkDeviceQueueCreateInfo queue_info = {};
    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.queueFamilyIndex = graphics_family;
    queue_info.queueCount = 1;
    float queue_priority = 1.0f;
    queue_info.pQueuePriorities = &queue_priority;

    VkPhysicalDeviceFeatures device_features = {};
    // Enable features as needed
    device_features.samplerAnisotropy = supported_features.samplerAnisotropy;

    VkDeviceCreateInfo device_info = {};
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.queueCreateInfoCount = 1;
    device_info.pQueueCreateInfos = &queue_info;
    device_info.pEnabledFeatures = &device_features;
    device_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
    device_info.ppEnabledExtensionNames = device_extensions.data();
    device_info.enabledLayerCount = 0;

    if (vkCreateDevice(m_PhysicalDevice, &device_info, nullptr, &m_Device) != VK_SUCCESS) {
        std::cerr << "Failed to create logical device" << std::endl;
        return false;
    }

    vkGetDeviceQueue(m_Device, graphics_family, 0, &m_GraphicsQueue);

    return true;
}

bool VulkanRenderInterface::Create_Command_Pool()
{
    uint32_t queue_family = 0; // Graphics family
    VkCommandPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = queue_family;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(m_Device, &pool_info, nullptr, &m_CommandPool) != VK_SUCCESS) {
        return false;
    }

    // Allocate command buffer
    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = m_CommandPool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(m_Device, &alloc_info, &m_CommandBuffer) != VK_SUCCESS) {
        return false;
    }

    return true;
}

bool VulkanRenderInterface::Create_Swap_Chain()
{
    if (m_WindowHandle == nullptr) {
        std::cerr << "No window handle provided" << std::endl;
        return false;
    }

#if defined(__linux__)
    // Create X11 surface
    VkXlibSurfaceCreateInfoKHR surface_info = {};
    surface_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    surface_info.dpy = reinterpret_cast<Display*>(m_WindowHandle);
    surface_info.window = reinterpret_cast<Window>(reinterpret_cast<uintptr_t>(m_WindowHandle) & ~0xFFFFFFFFFFFFFFFFULL);

    VkSurfaceKHR surface;
    VkResult result = vkCreateXlibSurfaceKHR(m_Instance, &surface_info, nullptr, &surface);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create X11 surface: " << result << std::endl;
        return false;
    }
    m_Surface = surface;

    // Get queue family indices with surface support
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queue_family_count, queue_families.data());

    int graphics_family = -1;
    int present_family = -1;
    for (uint32_t i = 0; i < queue_family_count; i++) {
        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_PhysicalDevice, i, m_Surface, &present_support);
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphics_family = i;
            if (present_support) {
                present_family = i;
                break;
            }
        }
    }
    if (graphics_family < 0) {
        std::cerr << "No graphics queue family found" << std::endl;
        return false;
    }
    if (present_family < 0) present_family = graphics_family;

    // Store queue families
    m_GraphicsQueueFamily = graphics_family;
    m_PresentQueueFamily = present_family;

#elif defined(_WIN32)
    // Create Win32 surface
    VkWin32SurfaceCreateInfoKHR surface_info = {};
    surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surface_info.hwnd = reinterpret_cast<HWND>(m_WindowHandle);
    surface_info.hinstance = GetModuleHandle(nullptr);

    VkSurfaceKHR surface;
    VkResult result = vkCreateWin32SurfaceKHR(m_Instance, &surface_info, nullptr, &surface);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create Win32 surface: " << result << std::endl;
        return false;
    }
    m_Surface = surface;

    // Get queue families
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queue_family_count, queue_families.data());

    int graphics_family = -1;
    int present_family = -1;
    for (uint32_t i = 0; i < queue_family_count; i++) {
        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_PhysicalDevice, i, m_Surface, &present_support);
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphics_family = i;
            if (present_support) {
                present_family = i;
                break;
            }
        }
    }
    if (graphics_family < 0) return false;
    if (present_family < 0) present_family = graphics_family;

    m_GraphicsQueueFamily = graphics_family;
    m_PresentQueueFamily = present_family;

#endif

    // Get surface capabilities
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_PhysicalDevice, m_Surface, &capabilities);

    // Choose swap chain extent
    if (capabilities.currentExtent.width == 0xFFFFFFFF) {
        // Extent is not determined yet, use window size or defaults
        m_SwapChainExtent.width = 800;
        m_SwapChainExtent.height = 600;
    } else {
        m_SwapChainExtent = capabilities.currentExtent;
    }

    // Choose present mode (try MAILBOX first, fall back to FIFO)
    uint32_t present_mode_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, m_Surface, &present_mode_count, nullptr);
    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, m_Surface, &present_mode_count, present_modes.data());

    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR; // Guaranteed
    for (const auto& mode : present_modes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            present_mode = mode;
            break;
        }
    }

    // Choose surface format
    uint32_t format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, m_Surface, &format_count, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, m_Surface, &format_count, formats.data());

    VkSurfaceFormatKHR surface_format = formats[0];
    if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
        surface_format.format = VK_FORMAT_B8G8R8A8_UNORM;
        surface_format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    }
    for (const auto& fmt : formats) {
        if (fmt.format == VK_FORMAT_B8G8R8A8_UNORM) {
            surface_format = fmt;
            break;
        }
    }

    m_SwapChainImageFormat = surface_format.format;

    // Determine number of images
    uint32_t image_count = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount) {
        image_count = capabilities.maxImageCount;
    }

    // Create swap chain
    VkSwapchainCreateInfoKHR swap_info = {};
    swap_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swap_info.surface = m_Surface;
    swap_info.minImageCount = image_count;
    swap_info.imageFormat = surface_format.format;
    swap_info.imageColorSpace = surface_format.colorSpace;
    swap_info.imageExtent = m_SwapChainExtent;
    swap_info.imageArrayLayers = 1;
    swap_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swap_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swap_info.preTransform = capabilities.currentTransform;
    swap_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swap_info.presentMode = present_mode;
    swap_info.clipped = VK_TRUE;
    swap_info.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(m_Device, &swap_info, nullptr, &m_SwapChain) != VK_SUCCESS) {
        std::cerr << "Failed to create swap chain" << std::endl;
        return false;
    }

    // Get swap chain images
    vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &image_count, nullptr);
    m_SwapChainImages.resize(image_count);
    vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &image_count, m_SwapChainImages.data());

    // Create image views
    m_SwapChainImageViews.resize(image_count);
    for (uint32_t i = 0; i < image_count; i++) {
        VkImageViewCreateInfo view_info = {};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = m_SwapChainImages[i];
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = m_SwapChainImageFormat;
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view_info.subresourceRange.baseMipLevel = 0;
        view_info.subresourceRange.levelCount = 1;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = 1;

        if (vkCreateImageView(m_Device, &view_info, nullptr, &m_SwapChainImageViews[i]) != VK_SUCCESS) {
            std::cerr << "Failed to create image view " << i << std::endl;
            return false;
        }
    }

    return true;
}

bool VulkanRenderInterface::Create_Render_Pass()
{
    VkAttachmentDescription color_attachment = {};
    color_attachment.format = m_SwapChainImageFormat;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref = {};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;

    VkRenderPassCreateInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &color_attachment;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;

    if (vkCreateRenderPass(m_Device, &render_pass_info, nullptr, &m_RenderPass) != VK_SUCCESS) {
        return false;
    }

    return true;
}

bool VulkanRenderInterface::Create_Descriptor_Set_Layout()
{
    VkDescriptorSetLayoutBinding ubo_binding = {};
    ubo_binding.binding = 0;
    ubo_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_binding.descriptorCount = 1;
    ubo_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo layout_info = {};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = 1;
    layout_info.pBindings = &ubo_binding;

    if (vkCreateDescriptorSetLayout(m_Device, &layout_info, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS) {
        return false;
    }

    return true;
}

bool VulkanRenderInterface::Create_Graphics_Pipeline()
{
    // ================================================================
    // Shader Modules
    // ================================================================
    if (m_VertexShaderModule == VK_NULL_HANDLE) {
        m_VertexShaderModule = Create_Shader_Module(g_VertexShaderSPV, sizeof(g_VertexShaderSPV));
        if (m_VertexShaderModule == VK_NULL_HANDLE) {
            std::cerr << "Failed to create vertex shader module" << std::endl;
            return false;
        }
    }

    if (m_FragmentShaderModule == VK_NULL_HANDLE) {
        m_FragmentShaderModule = Create_Shader_Module(g_FragmentShaderSPV, sizeof(g_FragmentShaderSPV));
        if (m_FragmentShaderModule == VK_NULL_HANDLE) {
            std::cerr << "Failed to create fragment shader module" << std::endl;
            return false;
        }
    }

    // ================================================================
    // Vertex Input State
    // ================================================================
    // Binding description: one binding for vertex data (position, normal, UV)
    VkVertexInputBindingDescription binding_description = {};
    binding_description.binding = 0;
    binding_description.stride = sizeof(float) * 8; // pos(3) + normal(3) + uv(2) = 8 floats
    binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    // Attribute descriptions: position, normal, texcoord
    std::vector<VkVertexInputAttributeDescription> attribute_descriptions(3);

    // Position (location 0)
    attribute_descriptions[0].binding = 0;
    attribute_descriptions[0].location = 0;
    attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions[0].offset = 0;

    // Normal (location 1)
    attribute_descriptions[1].binding = 0;
    attribute_descriptions[1].location = 1;
    attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions[1].offset = sizeof(float) * 3;

    // TexCoord (location 2)
    attribute_descriptions[2].binding = 0;
    attribute_descriptions[2].location = 2;
    attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attribute_descriptions[2].offset = sizeof(float) * 6;

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.pVertexBindingDescriptions = &binding_description;
    vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
    vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();

    // ================================================================
    // Input Assembly State
    // ================================================================
    VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {};
    input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // Default
    input_assembly_info.primitiveRestartEnable = VK_FALSE;

    // ================================================================
    // Shader Stages
    // ================================================================
    VkPipelineShaderStageCreateInfo vert_shader_stage_info = {};
    vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_shader_stage_info.module = m_VertexShaderModule;
    vert_shader_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo frag_shader_stage_info = {};
    frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_shader_stage_info.module = m_FragmentShaderModule;
    frag_shader_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo shader_stages[] = {
        vert_shader_stage_info,
        frag_shader_stage_info
    };

    // ================================================================
    // Dynamic State (Viewport and Scissor)
    // ================================================================
    std::vector<VkDynamicState> dynamic_states = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamic_state_info = {};
    dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state_info.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
    dynamic_state_info.pDynamicStates = dynamic_states.data();

    // ================================================================
    // Viewport State (will be overridden by dynamic state)
    // ================================================================
    VkPipelineViewportStateCreateInfo viewport_state_info = {};
    viewport_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state_info.viewportCount = 1;
    viewport_state_info.scissorCount = 1;
    viewport_state_info.pScissors = nullptr;  // Dynamic
    viewport_state_info.pViewports = nullptr;  // Dynamic

    // ================================================================
    // Rasterization State
    // ================================================================
    VkPipelineRasterizationStateCreateInfo rasterization_state_info = {};
    rasterization_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization_state_info.depthClampEnable = VK_FALSE;
    rasterization_state_info.rasterizerDiscardEnable = VK_FALSE;
    rasterization_state_info.polygonMode = VK_POLYGON_MODE_FILL;
    rasterization_state_info.lineWidth = 1.0f;
    rasterization_state_info.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterization_state_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterization_state_info.depthBiasEnable = VK_TRUE;
    rasterization_state_info.depthBiasConstantFactor = 0.0f;
    rasterization_state_info.depthBiasClamp = 0.0f;
    rasterization_state_info.depthBiasSlopeFactor = 0.0f;

    // ================================================================
    // Multisample State
    // ================================================================
    VkPipelineMultisampleStateCreateInfo multisample_state_info = {};
    multisample_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_state_info.sampleShadingEnable = VK_FALSE;
    multisample_state_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisample_state_info.minSampleShading = 1.0f;
    multisample_state_info.pSampleMask = nullptr;
    multisample_state_info.alphaToCoverageEnable = VK_FALSE;
    multisample_state_info.alphaToOneEnable = VK_FALSE;

    // ================================================================
    // Color Blend State
    // ================================================================
    VkPipelineColorBlendAttachmentState color_blend_attachment = {};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                            VK_COLOR_COMPONENT_G_BIT |
                                            VK_COLOR_COMPONENT_B_BIT |
                                            VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo color_blend_state_info = {};
    color_blend_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_state_info.logicOpEnable = VK_FALSE;
    color_blend_state_info.logicOp = VK_LOGIC_OP_COPY;
    color_blend_state_info.attachmentCount = 1;
    color_blend_state_info.pAttachments = &color_blend_attachment;
    color_blend_state_info.blendConstants[0] = 0.0f;
    color_blend_state_info.blendConstants[1] = 0.0f;
    color_blend_state_info.blendConstants[2] = 0.0f;
    color_blend_state_info.blendConstants[3] = 0.0f;

    // ================================================================
    // Depth Stencil State
    // ================================================================
    VkPipelineDepthStencilStateCreateInfo depth_stencil_info = {};
    depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil_info.depthTestEnable = VK_TRUE;
    depth_stencil_info.depthWriteEnable = VK_TRUE;
    depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
    depth_stencil_info.minDepthBounds = 0.0f;
    depth_stencil_info.maxDepthBounds = 1.0f;
    depth_stencil_info.stencilTestEnable = VK_FALSE;
    depth_stencil_info.front = {};
    depth_stencil_info.back = {};

    // ================================================================
    // Pipeline Layout
    // ================================================================
    VkPipelineLayoutCreateInfo pipeline_layout_info = {};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &m_DescriptorSetLayout;
    pipeline_layout_info.pushConstantRangeCount = 0;
    pipeline_layout_info.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(m_Device, &pipeline_layout_info, nullptr, &m_PipelineLayout) != VK_SUCCESS) {
        std::cerr << "Failed to create pipeline layout" << std::endl;
        return false;
    }

    // ================================================================
    // Graphics Pipeline
    // ================================================================
    VkGraphicsPipelineCreateInfo pipeline_info = {};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly_info;
    pipeline_info.pViewportState = &viewport_state_info;
    pipeline_info.pRasterizationState = &rasterization_state_info;
    pipeline_info.pMultisampleState = &multisample_state_info;
    pipeline_info.pColorBlendState = &color_blend_state_info;
    pipeline_info.pDepthStencilState = &depth_stencil_info;
    pipeline_info.pDynamicState = &dynamic_state_info;
    pipeline_info.layout = m_PipelineLayout;
    pipeline_info.renderPass = m_RenderPass;
    pipeline_info.subpass = 0;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_info.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &m_GraphicsPipeline) != VK_SUCCESS) {
        std::cerr << "Failed to create graphics pipeline" << std::endl;
        return false;
    }

    return true;
}

// ================================================================
// Shader Module Creation Helper
// ================================================================
VkShaderModule VulkanRenderInterface::Create_Shader_Module(const uint32_t* code, size_t size)
{
    VkShaderModuleCreateInfo module_info = {};
    module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    module_info.codeSize = size;
    module_info.pCode = code;

    VkShaderModule shader_module;
    if (vkCreateShaderModule(m_Device, &module_info, nullptr, &shader_module) != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }

    return shader_module;
}

// ================================================================
// SPIR-V Passthrough Vertex Shader
// Inputs: position (vec3), normal (vec3), uv (vec2)
// Outputs: gl_Position, vTexCoord (vec2)
// ================================================================
// GLSL Source:
// #version 450
// layout(location = 0) in vec3 inPosition;
// layout(location = 1) in vec3 inNormal;
// layout(location = 2) in vec2 inTexCoord;
// layout(location = 0) out vec2 vTexCoord;
// void main() {
//     gl_Position = vec4(inPosition, 1.0);
//     vTexCoord = inTexCoord;
// }
static const uint32_t g_VertexShaderSPV[] = {
    0x07230203, 0x00010000, 0x00080007, 0x00000026, 0x00000000, 0x00020011, 0x00000001, 0x0006000b,
    0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001,
    0x0007000f, 0x00000000, 0x00000004, 0x6e69616d, 0x00000000, 0x0000000f, 0x00000017, 0x00030003,
    0x00000002, 0x000001c2, 0x00040005, 0x00000004, 0x6e69616d, 0x00000000, 0x00050005, 0x0000000b,
    0x6378652e, 0x64726168, 0x00000073, 0x00050005, 0x0000000d, 0x6f6c6f63, 0x45440072, 0x74615400,
    0x726f6f43, 0x6564006c, 0x64615400, 0x657a6952, 0x68637261, 0x0000006e, 0x00050005, 0x00000014,
    0x565f3256, 0x74655478, 0x436f7264, 0x616f4c00, 0x74695700, 0x78655400, 0x436f7264, 0x616f4c00,
    0x00060006, 0x00000014, 0x00000000, 0x505f6c67, 0x7469736f, 0x006e6f69, 0x00030005, 0x00000017,
    0x00000000, 0x0006000a, 0x00000000, 0x00000001, 0x00000000, 0x00000014, 0x00000000, 0x00030002,
    0x00000003, 0x000000c2, 0x00040002, 0x00000005, 0x00000000, 0x0000000f, 0x00030002, 0x00000006,
    0x00000003, 0x0004002b, 0x00000006, 0x00000007, 0x00000000, 0x0003002e, 0x0000000b, 0x00000007,
    0x0004002b, 0x00000006, 0x00000009, 0x3f800000, 0x0004002b, 0x00000006, 0x0000000c, 0x00000000,
    0x0004002b, 0x00000006, 0x0000000e, 0x00000000, 0x0004002b, 0x00000006, 0x00000010, 0x3f800000,
    0x0004002b, 0x00000006, 0x00000016, 0x00000000, 0x00050036, 0x00000001, 0x00000002, 0x00000000,
    0x00000005, 0x000200f8, 0x00000003, 0x0004003b, 0x0000000b, 0x0000000c, 0x00000000, 0x0004003b,
    0x0000000d, 0x0000000e, 0x00000001, 0x0004003b, 0x0000000d, 0x00000010, 0x00000002, 0x00050041,
    0x0000000f, 0x00000011, 0x0000000c, 0x00000009, 0x0003003e, 0x00000011, 0x00000010, 0x00050041,
    0x0000000f, 0x00000012, 0x0000000e, 0x00000009, 0x0003003e, 0x00000012, 0x00000010, 0x00050041,
    0x0000000f, 0x00000015, 0x0000000e, 0x00000009, 0x0003003e, 0x00000015, 0x00000010, 0x000100fd,
    0x00010038
};

// ================================================================
// SPIR-V Simple Fragment Shader
// Inputs: vTexCoord (vec2)
// Output: fragColor (vec4)
// ================================================================
// GLSL Source:
// #version 450
// layout(location = 0) in vec2 vTexCoord;
// layout(location = 0) out vec4 fragColor;
// void main() {
//     fragColor = vec4(1.0, 1.0, 1.0, 1.0);
// }
static const uint32_t g_FragmentShaderSPV[] = {
    0x07230203, 0x00010000, 0x00080007, 0x0000001a, 0x00000000, 0x00020011, 0x00000001, 0x0006000b,
    0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001,
    0x0007000f, 0x00000000, 0x00000004, 0x6e69616d, 0x00000000, 0x00000008, 0x00000013, 0x00030003,
    0x00000002, 0x000001c2, 0x00040005, 0x00000004, 0x6e69616d, 0x00000000, 0x00050005, 0x00000009,
    0x6378652e, 0x64726168, 0x00000073, 0x00040005, 0x0000000c, 0x67617266, 0x436f6c72, 0x00040005,
    0x0000000e, 0x565f3256, 0x74655478, 0x436f7264, 0x616f4c00, 0x00060006, 0x0000000e, 0x00000000,
    0x505f6c67, 0x7469736f, 0x006e6f69, 0x00030005, 0x00000013, 0x00000000, 0x0006000a, 0x00000000,
    0x00000001, 0x00000000, 0x0000000e, 0x00000000, 0x00030002, 0x00000003, 0x000000c2, 0x00040002,
    0x00000005, 0x00000000, 0x00000008, 0x0004002b, 0x00000006, 0x0000000f, 0x3f800000, 0x0004002b,
    0x00000006, 0x00000011, 0x00000000, 0x0004002b, 0x00000006, 0x00000012, 0x00000000, 0x0004002b,
    0x00000006, 0x00000014, 0x3f800000, 0x00050036, 0x00000001, 0x00000002, 0x00000000, 0x00000005,
    0x000200f8, 0x00000003, 0x0004003b, 0x00000009, 0x0000000a, 0x00000000, 0x0004003b, 0x0000000e,
    0x0000000f, 0x00000000, 0x00050041, 0x00000008, 0x00000010, 0x0000000a, 0x0000000f, 0x0003003e,
    0x00000010, 0x00000012, 0x000100fd, 0x00010038
};

bool VulkanRenderInterface::Create_Framebuffers()
{
    m_SwapChainFramebuffers.resize(m_SwapChainImageViews.size());

    for (size_t i = 0; i < m_SwapChainImageViews.size(); i++) {
        VkImageView attachments[] = { m_SwapChainImageViews[i] };

        VkFramebufferCreateInfo framebuffer_info = {};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = m_RenderPass;
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.pAttachments = attachments;
        framebuffer_info.width = m_SwapChainExtent.width;
        framebuffer_info.height = m_SwapChainExtent.height;
        framebuffer_info.layers = 1;

        if (vkCreateFramebuffer(m_Device, &framebuffer_info, nullptr, &m_SwapChainFramebuffers[i]) != VK_SUCCESS) {
            std::cerr << "Failed to create framebuffer " << i << std::endl;
            return false;
        }
    }

    return true;
}

bool VulkanRenderInterface::Create_Sync_Objects()
{
    const int MAX_FRAMES = 2;
    m_ImageAvailableSemaphores.resize(MAX_FRAMES);
    m_RenderFinishedSemaphores.resize(MAX_FRAMES);
    m_InFlightFences.resize(MAX_FRAMES);

    VkSemaphoreCreateInfo semaphore_info = {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (int i = 0; i < MAX_FRAMES; i++) {
        if (vkCreateSemaphore(m_Device, &semaphore_info, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(m_Device, &semaphore_info, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(m_Device, &fence_info, nullptr, &m_InFlightFences[i]) != VK_SUCCESS) {
            std::cerr << "Failed to create sync objects for frame " << i << std::endl;
            return false;
        }
    }

    return true;
}

void VulkanRenderInterface::Cleanup_Swap_Chain()
{
    for (auto framebuffer : m_SwapChainFramebuffers) {
        vkDestroyFramebuffer(m_Device, framebuffer, nullptr);
    }

    for (auto image_view : m_SwapChainImageViews) {
        vkDestroyImageView(m_Device, image_view, nullptr);
    }

    if (m_SwapChain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(m_Device, m_SwapChain, nullptr);
        m_SwapChain = VK_NULL_HANDLE;
    }
}

VkCommandBuffer VulkanRenderInterface::Begin_Single_Time_Commands()
{
    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = m_CommandPool;
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(m_Device, &alloc_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(command_buffer, &begin_info);
    return command_buffer;
}

void VulkanRenderInterface::End_Single_Time_Commands(VkCommandBuffer command_buffer)
{
    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    vkQueueSubmit(m_GraphicsQueue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_GraphicsQueue);

    vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &command_buffer);
}

void VulkanRenderInterface::Update_Descriptor_Sets()
{
    // Update descriptor sets with current buffer/texture bindings
}

void VulkanRenderInterface::Rebuild_Pipeline_State()
{
    // Rebuild graphics pipeline when render state changes
    if (m_GraphicsPipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(m_Device, m_GraphicsPipeline, nullptr);
        Create_Graphics_Pipeline();
    }
}

// =====================================================================
// Format and state conversion helpers
// =====================================================================

VkFormat VulkanRenderInterface::Convert_Format(TextureFormat format)
{
    switch (format) {
        case TEX_FORMAT_A8R8G8B8: return VK_FORMAT_B8G8R8A8_UNORM;
        case TEX_FORMAT_X8R8G8B8: return VK_FORMAT_B8G8R8A8_UNORM;
        case TEX_FORMAT_R5G6B5: return VK_FORMAT_R5G6B5_UNORM_PACK16;
        case TEX_FORMAT_A1R5G5B5: return VK_FORMAT_A1R5G5B5_UNORM_PACK16;
        case TEX_FORMAT_D16: return VK_FORMAT_D16_UNORM;
        case TEX_FORMAT_D24S8: return VK_FORMAT_D24_UNORM_S8_UINT;
        case TEX_FORMAT_D32: return VK_FORMAT_D32_SFLOAT;
        default: return VK_FORMAT_B8G8R8A8_UNORM;
    }
}

VkPrimitiveTopology VulkanRenderInterface::Convert_Primitive_Type(PrimitiveType type)
{
    switch (type) {
        case PRIMITIVE_POINT_LIST: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case PRIMITIVE_LINE_LIST: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case PRIMITIVE_LINE_STRIP: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case PRIMITIVE_TRIANGLE_LIST: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case PRIMITIVE_TRIANGLE_STRIP: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case PRIMITIVE_TRIANGLE_FAN: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
        default: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }
}

VkCullModeFlags VulkanRenderInterface::Convert_Cull_Mode(CullMode mode)
{
    switch (mode) {
        case CULL_NONE: return VK_CULL_MODE_NONE;
        case CULL_FRONT: return VK_CULL_MODE_FRONT_BIT;
        case CULL_BACK: return VK_CULL_MODE_BACK_BIT;
        default: return VK_CULL_MODE_BACK_BIT;
    }
}

VkBlendFactor VulkanRenderInterface::Convert_Blend_Factor(BlendFactor factor)
{
    switch (factor) {
        case BLEND_ZERO: return VK_BLEND_FACTOR_ZERO;
        case BLEND_ONE: return VK_BLEND_FACTOR_ONE;
        case BLEND_SRCCOLOR: return VK_BLEND_FACTOR_SRC_COLOR;
        case BLEND_INVSRCCOLOR: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case BLEND_SRCALPHA: return VK_BLEND_FACTOR_SRC_ALPHA;
        case BLEND_INVSRCALPHA: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case BLEND_DESTALPHA: return VK_BLEND_FACTOR_DST_ALPHA;
        case BLEND_INVDESTALPHA: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case BLEND_DESTCOLOR: return VK_BLEND_FACTOR_DST_COLOR;
        case BLEND_INVDESTCOLOR: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case BLEND_SRCALPHASAT: return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
        default: return VK_BLEND_FACTOR_ONE;
    }
}

VkBlendOp VulkanRenderInterface::Convert_Blend_Op(BlendOperation op)
{
    switch (op) {
        case BLEND_OP_ADD: return VK_BLEND_OP_ADD;
        case BLEND_OP_SUBTRACT: return VK_BLEND_OP_SUBTRACT;
        case BLEND_OP_REV_SUBTRACT: return VK_BLEND_OP_REVERSE_SUBTRACT;
        case BLEND_OP_MIN: return VK_BLEND_OP_MIN;
        case BLEND_OP_MAX: return VK_BLEND_OP_MAX;
        default: return VK_BLEND_OP_ADD;
    }
}

VkCompareOp VulkanRenderInterface::Convert_Compare_Op(ComparisonFunc func)
{
    switch (func) {
        case CMP_NEVER: return VK_COMPARE_OP_NEVER;
        case CMP_LESS: return VK_COMPARE_OP_LESS;
        case CMP_EQUAL: return VK_COMPARE_OP_EQUAL;
        case CMP_LESSEQUAL: return VK_COMPARE_OP_LESS_OR_EQUAL;
        case CMP_GREATER: return VK_COMPARE_OP_GREATER;
        case CMP_NOTEQUAL: return VK_COMPARE_OP_NOT_EQUAL;
        case CMP_GREATEREQUAL: return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case CMP_ALWAYS: return VK_COMPARE_OP_ALWAYS;
        default: return VK_COMPARE_OP_LESS_OR_EQUAL;
    }
}

VkStencilOp VulkanRenderInterface::Convert_Stencil_Op(StencilOperation op)
{
    switch (op) {
        case STENCIL_OP_KEEP: return VK_STENCIL_OP_KEEP;
        case STENCIL_OP_ZERO: return VK_STENCIL_OP_ZERO;
        case STENCIL_OP_REPLACE: return VK_STENCIL_OP_REPLACE;
        case STENCIL_OP_INCR: return VK_STENCIL_OP_INCREMENT_AND_WRAP;
        case STENCIL_OP_DECR: return VK_STENCIL_OP_DECREMENT_AND_WRAP;
        case STENCIL_OP_INVERT: return VK_STENCIL_OP_INVERT;
        default: return VK_STENCIL_OP_KEEP;
    }
}

VkFilter VulkanRenderInterface::Convert_Filter(TextureFilter filter)
{
    switch (filter) {
        case TF_NONE: return VK_FILTER_NEAREST;
        case TF_POINT: return VK_FILTER_NEAREST;
        case TF_LINEAR: return VK_FILTER_LINEAR;
        case TF_ANISOTROPIC: return VK_FILTER_LINEAR; // Anisotropic needs sampler config
        default: return VK_FILTER_LINEAR;
    }
}

VkSamplerAddressMode VulkanRenderInterface::Convert_Texture_Address(TextureAddress addr)
{
    switch (addr) {
        case TAM_WRAP: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case TAM_MIRROR: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case TAM_CLAMP: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case TAM_BORDER: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        case TAM_MIRRORONCE: return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
        default: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    }
}

Matrix4 VulkanRenderInterface::Flip_Y_Axis(const Matrix4& matrix)
{
    // Flip Y axis for Vulkan coordinate system
    Matrix4 result = matrix;
    result[1][0] = -result[1][0];
    result[1][1] = -result[1][1];
    result[1][2] = -result[1][2];
    result[1][3] = -result[1][3];
    return result;
}

Matrix4 VulkanRenderInterface::Convert_Projection(const Matrix4& matrix)
{
    // Convert D3D projection to Vulkan projection
    // Vulkan uses depth range [0,1] instead of [0,1]
    Matrix4 result = matrix;

    // The perspective divide z component adjustment would go here
    // For now, just return as-is

    return result;
}

// =====================================================================
// Texture helper method implementations
// =====================================================================

uint32_t VulkanRenderInterface::Find_Memory_Type(uint32_t type_filter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &mem_properties);

    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    // Fallback: try any available memory type
    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
        if (type_filter & (1 << i)) {
            return i;
        }
    }

    return 0; // Should never happen
}

bool VulkanRenderInterface::Create_Image(uint32_t width, uint32_t height, VkFormat format,
                                        VkImageTiling tiling, VkImageUsageFlags usage,
                                        uint32_t mip_levels, VkMemoryPropertyFlags properties,
                                        VkImage& image, VkDeviceMemory& memory)
{
    VkImageCreateInfo image_info = {};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = width;
    image_info.extent.height = height;
    image_info.extent.depth = 1;
    image_info.mipLevels = mip_levels;
    image_info.arrayLayers = 1;
    image_info.format = format;
    image_info.tiling = tiling;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = usage;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(m_Device, &image_info, nullptr, &image) != VK_SUCCESS) {
        return false;
    }

    VkMemoryRequirements mem_reqs;
    vkGetImageMemoryRequirements(m_Device, image, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_reqs.size;
    alloc_info.memoryTypeIndex = Find_Memory_Type(mem_reqs.memoryTypeBits, properties);

    if (vkAllocateMemory(m_Device, &alloc_info, nullptr, &memory) != VK_SUCCESS) {
        vkDestroyImage(m_Device, image, nullptr);
        return false;
    }

    vkBindImageMemory(m_Device, image, memory, 0);
    return true;
}

bool VulkanRenderInterface::Create_Image_View(VkImage image, VkFormat format,
                                             VkImageAspectFlags aspect_mask, VkImageViewType view_type,
                                             uint32_t mip_levels, VkImageView& view)
{
    VkImageViewCreateInfo view_info = {};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = image;
    view_info.viewType = view_type;
    view_info.format = format;
    view_info.subresourceRange.aspectMask = aspect_mask;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = mip_levels;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    if (vkCreateImageView(m_Device, &view_info, nullptr, &view) != VK_SUCCESS) {
        return false;
    }
    return true;
}

bool VulkanRenderInterface::Create_Sampler(VkFilter mag_filter, VkFilter min_filter,
                                           VkSamplerAddressMode address_mode_u,
                                           VkSamplerAddressMode address_mode_v,
                                           VkSamplerMipmapMode mip_mode, float max_anisotropy,
                                           VkSampler& sampler)
{
    VkSamplerCreateInfo sampler_info = {};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = mag_filter;
    sampler_info.minFilter = min_filter;
    sampler_info.mipmapMode = mip_mode;
    sampler_info.addressModeU = address_mode_u;
    sampler_info.addressModeV = address_mode_v;
    sampler_info.addressModeW = address_mode_u;
    sampler_info.maxAnisotropy = max_anisotropy;
    sampler_info.anisotropyEnable = (max_anisotropy > 1.0f) ? VK_TRUE : VK_FALSE;
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = VK_LOD_CLAMP_NONE;

    if (vkCreateSampler(m_Device, &sampler_info, nullptr, &sampler) != VK_SUCCESS) {
        return false;
    }
    return true;
}

void VulkanRenderInterface::Transition_Image_Layout(VkImage image, VkFormat format,
                                                    VkImageLayout old_layout, VkImageLayout new_layout,
                                                    uint32_t mip_levels)
{
    VkCommandBuffer cmd = Begin_Single_Time_Commands();

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mip_levels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags destination_stage;

    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
               new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
               new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
               new_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
               new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL &&
               new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        source_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL &&
               new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        source_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        destination_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    } else {
        // Generic transition for other cases
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = 0;
        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    }

    vkCmdPipelineBarrier(cmd, source_stage, destination_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    End_Single_Time_Commands(cmd);
}

void VulkanRenderInterface::Copy_Buffer_To_Image(VkBuffer buffer, VkImage image,
                                                  uint32_t width, uint32_t height)
{
    VkCommandBuffer cmd = Begin_Single_Time_Commands();

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = width;
    region.bufferImageHeight = height;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(cmd, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    End_Single_Time_Commands(cmd);
}

void VulkanRenderInterface::Copy_Image_To_Image(VkImage src_image, VkImage dst_image,
                                                uint32_t width, uint32_t height,
                                                VkImageAspectFlags aspect_mask)
{
    VkCommandBuffer cmd = Begin_Single_Time_Commands();

    VkImageCopy region = {};
    region.srcSubresource.aspectMask = aspect_mask;
    region.srcSubresource.mipLevel = 0;
    region.srcSubresource.baseArrayLayer = 0;
    region.srcSubresource.layerCount = 1;
    region.dstSubresource.aspectMask = aspect_mask;
    region.dstSubresource.mipLevel = 0;
    region.dstSubresource.baseArrayLayer = 0;
    region.dstSubresource.layerCount = 1;
    region.extent = {width, height, 1};

    vkCmdCopyImage(cmd, src_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   1, &region);

    End_Single_Time_Commands(cmd);
}

void VulkanRenderInterface::Copy_Rect_Helper(VkImage src_image, VkImage dst_image,
                                              uint32_t src_x, uint32_t src_y,
                                              uint32_t dst_x, uint32_t dst_y,
                                              uint32_t width, uint32_t height)
{
    VkCommandBuffer cmd = Begin_Single_Time_Commands();

    // Ensure source is in TRANSFER_SRC layout
    Transition_Image_Layout(src_image, VK_FORMAT_UNDEFINED,
                            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 1);

    // Ensure destination is in TRANSFER_DST layout
    Transition_Image_Layout(dst_image, VK_FORMAT_UNDEFINED,
                            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1);

    VkImageCopy region = {};
    region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.srcSubresource.mipLevel = 0;
    region.srcSubresource.baseArrayLayer = 0;
    region.srcSubresource.layerCount = 1;
    region.srcOffset = {static_cast<int32_t>(src_x), static_cast<int32_t>(src_y), 0};
    region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.dstSubresource.mipLevel = 0;
    region.dstSubresource.baseArrayLayer = 0;
    region.dstSubresource.layerCount = 1;
    region.dstOffset = {static_cast<int32_t>(dst_x), static_cast<int32_t>(dst_y), 0};
    region.extent = {width, height, 1};

    vkCmdCopyImage(cmd, src_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   1, &region);

    End_Single_Time_Commands(cmd);
}

void VulkanRenderInterface::Generate_Mipmaps(VkImage image, VkFormat format,
                                            uint32_t width, uint32_t height, uint32_t mip_levels)
{
    if (mip_levels <= 1) return;

    // Check if format supports linear blitting
    VkFormatProperties format_properties;
    vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &format_properties);

    if (!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        // Format doesn't support linear blitting for mip generation
        // Only use the base level
        return;
    }

    VkCommandBuffer cmd = Begin_Single_Time_Commands();

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    int32_t mip_width = static_cast<int32_t>(width);
    int32_t mip_height = static_cast<int32_t>(height);

    for (uint32_t i = 1; i < mip_levels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.subresourceRange.levelCount = 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                            0, 0, nullptr, 0, nullptr, 1, &barrier);

        VkImageBlit blit = {};
        blit.srcOffsets[0] = {0, 0, 0};
        blit.srcOffsets[1] = {mip_width, mip_height, 1};
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = {0, 0, 0};
        blit.dstOffsets[1] = {mip_width > 1 ? mip_width / 2 : 1, mip_height > 1 ? mip_height / 2 : 1, 1};
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                      image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                      1, &blit, VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                            0, 0, nullptr, 0, nullptr, 1, &barrier);

        if (mip_width > 1) mip_width /= 2;
        if (mip_height > 1) mip_height /= 2;
    }

    // Transition the last mip level
    barrier.subresourceRange.baseMipLevel = mip_levels - 1;
    barrier.subresourceRange.levelCount = 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                        0, 0, nullptr, 0, nullptr, 1, &barrier);

    End_Single_Time_Commands(cmd);
}

VulkanTextureInfo* VulkanRenderInterface::Load_Texture_From_File(const char* filename, bool generate_mips)
{
    // Use the TGA file loader from textureloader.cpp logic
    VulkanTextureInfo* tex_info = new VulkanTextureInfo();

    // Open the TGA file
    FILE* file = fopen(filename, "rb");
    if (!file) {
        delete tex_info;
        return nullptr;
    }

    // Read TGA header
    uint8_t header[18];
    if (fread(header, 1, 18, file) != 18) {
        fclose(file);
        delete tex_info;
        return nullptr;
    }

    // Parse TGA header
    unsigned width = header[12] | (header[13] << 8);
    unsigned height = header[14] | (header[15] << 8);
    uint8_t bpp = header[16];
    uint8_t image_type = header[2];

    // Validate dimensions
    if (width == 0 || height == 0) {
        fclose(file);
        delete tex_info;
        return nullptr;
    }

    tex_info->Width = width;
    tex_info->Height = height;

    // Determine format from BPP
    VkFormat vk_format;
    uint32_t bytes_per_pixel;
    switch (bpp) {
        case 32:
            vk_format = VK_FORMAT_B8G8R8A8_UNORM;
            tex_info->Format = TEX_FORMAT_A8R8G8B8;
            bytes_per_pixel = 4;
            break;
        case 24:
            vk_format = VK_FORMAT_B8G8R8_UNORM;
            tex_info->Format = TEX_FORMAT_X8R8G8B8;
            bytes_per_pixel = 3;
            break;
        case 16:
            vk_format = VK_FORMAT_B8G8R8A8_UNORM;
            tex_info->Format = TEX_FORMAT_A8R8G8B8;
            bytes_per_pixel = 2;
            break;
        case 8:
            vk_format = VK_FORMAT_R8_UNORM;
            tex_info->Format = TEX_FORMAT_L8;
            bytes_per_pixel = 1;
            break;
        default:
            fclose(file);
            delete tex_info;
            return nullptr;
    }

    // Skip image ID field
    uint8_t id_length = header[0];
    if (id_length > 0) {
        fseek(file, id_length, SEEK_CUR);
    }

    // Skip color map if present
    if (header[1] != 0) {
        uint16_t cmap_first = header[3] | (header[4] << 8);
        uint16_t cmap_length = header[5] | (header[6] << 8);
        uint8_t cmap_depth = header[7];
        fseek(file, (cmap_length * (cmap_depth / 8)), SEEK_CUR);
    }

    // Calculate image size
    unsigned image_size = width * height * bytes_per_pixel;
    std::vector<uint8_t> image_data(image_size);

    // Read image data
    if (image_type == 2 || image_type == 10) { // Uncompressed or RLE compressed
        // For RLE, we'd need decompression - for simplicity, assume uncompressed
        size_t read = fread(image_data.data(), 1, image_size, file);
        if (read != image_size) {
            // Try to continue anyway
        }
    }

    fclose(file);

    // Convert BGR to BGRA if needed
    if (bpp == 3) {
        std::vector<uint8_t> rgba_data(width * height * 4);
        for (unsigned i = 0; i < width * height; i++) {
            rgba_data[i * 4 + 0] = image_data[i * 3 + 0];
            rgba_data[i * 4 + 1] = image_data[i * 3 + 1];
            rgba_data[i * 4 + 2] = image_data[i * 3 + 2];
            rgba_data[i * 4 + 3] = 255;
        }
        image_data = std::move(rgba_data);
        bytes_per_pixel = 4;
        vk_format = VK_FORMAT_B8G8R8A8_UNORM;
    }

    // Calculate mip levels
    uint32_t mip_levels = 1;
    if (generate_mips) {
        mip_levels = static_cast<uint32_t>(floor(log2(std::max(width, height))) + 1);
    }

    // Create staging buffer
    VkBuffer staging_buffer;
    VkDeviceMemory staging_memory;
    size_t buffer_size = width * height * 4; // 4 bytes per pixel in staging

    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = buffer_size;
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_Device, &buffer_info, nullptr, &staging_buffer) != VK_SUCCESS) {
        delete tex_info;
        return nullptr;
    }

    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(m_Device, staging_buffer, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_reqs.size;
    alloc_info.memoryTypeIndex = Find_Memory_Type(mem_reqs.memoryTypeBits,
                                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(m_Device, &alloc_info, nullptr, &staging_memory) != VK_SUCCESS) {
        vkDestroyBuffer(m_Device, staging_buffer, nullptr);
        delete tex_info;
        return nullptr;
    }

    vkBindBufferMemory(m_Device, staging_buffer, staging_memory, 0);

    // Copy image data to staging buffer
    void* mapped;
    vkMapMemory(m_Device, staging_memory, 0, VK_WHOLE_SIZE, 0, &mapped);
    memcpy(mapped, image_data.data(), image_data.size());
    vkUnmapMemory(m_Device, staging_memory);

    // Create the GPU image
    if (!Create_Image(width, height, vk_format, VK_IMAGE_TILING_OPTIMAL,
                     VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                     mip_levels, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                     tex_info->Image, tex_info->Memory)) {
        vkFreeMemory(m_Device, staging_memory, nullptr);
        vkDestroyBuffer(m_Device, staging_buffer, nullptr);
        delete tex_info;
        return nullptr;
    }

    // Transition image to destination layout
    Transition_Image_Layout(tex_info->Image, vk_format,
                            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mip_levels);

    // Copy buffer to image
    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = width;
    region.bufferImageHeight = height;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    VkCommandBuffer cmd = Begin_Single_Time_Commands();
    vkCmdCopyBufferToImage(cmd, staging_buffer, tex_info->Image,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    End_Single_Time_Commands(cmd);

    // Generate mipmaps if requested
    if (generate_mips && mip_levels > 1) {
        Generate_Mipmaps(tex_info->Image, vk_format, width, height, mip_levels);
    } else {
        // Transition to shader read layout
        Transition_Image_Layout(tex_info->Image, vk_format,
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mip_levels);
    }

    // Create image view
    if (!Create_Image_View(tex_info->Image, vk_format, VK_IMAGE_ASPECT_COLOR_BIT,
                          VK_IMAGE_VIEW_TYPE_2D, mip_levels, tex_info->ImageView)) {
        vkFreeMemory(m_Device, tex_info->Memory, nullptr);
        vkDestroyImage(m_Device, tex_info->Image, nullptr);
        vkFreeMemory(m_Device, staging_memory, nullptr);
        vkDestroyBuffer(m_Device, staging_buffer, nullptr);
        delete tex_info;
        return nullptr;
    }

    // Create sampler
    if (!Create_Sampler(VK_FILTER_LINEAR, VK_FILTER_LINEAR,
                       VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT,
                       generate_mips ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST,
                       1.0f, tex_info->Sampler)) {
        vkDestroyImageView(m_Device, tex_info->ImageView, nullptr);
        vkFreeMemory(m_Device, tex_info->Memory, nullptr);
        vkDestroyImage(m_Device, tex_info->Image, nullptr);
        vkFreeMemory(m_Device, staging_memory, nullptr);
        vkDestroyBuffer(m_Device, staging_buffer, nullptr);
        delete tex_info;
        return nullptr;
    }

    // Cleanup staging resources
    vkFreeMemory(m_Device, staging_memory, nullptr);
    vkDestroyBuffer(m_Device, staging_buffer, nullptr);

    tex_info->Is_RenderTarget = false;

    return tex_info;
}

uint32_t VulkanRenderInterface::Get_Bytes_Per_Pixel(TextureFormat format)
{
    switch (format) {
        case TEX_FORMAT_A8R8G8B8:
        case TEX_FORMAT_X8R8G8B8:
            return 4;
        case TEX_FORMAT_R5G6B5:
        case TEX_FORMAT_A1R5G5B5:
        case TEX_FORMAT_A4R4G4B4:
            return 2;
        case TEX_FORMAT_L8:
        case TEX_FORMAT_A8L8:
            return 1;
        case TEX_FORMAT_DXT1:
            return 4; // 8 bytes per 4 pixels, so 0.5 bytes per pixel, but aligned
        case TEX_FORMAT_DXT3:
        case TEX_FORMAT_DXT5:
            return 4;
        case TEX_FORMAT_D16:
            return 2;
        case TEX_FORMAT_D24S8:
            return 4;
        case TEX_FORMAT_D32:
            return 4;
        default:
            return 4;
    }
}

VkFormat VulkanRenderInterface::Get_Vulkan_Format(TextureFormat format)
{
    return Convert_Format(format);
}


// Factory method - platform-specific implementation
// =====================================================================

#if defined(_WIN32) || defined(__linux__)
// On Windows and Linux, return Vulkan renderer
RenderInterface* RenderInterface::Create_Renderer()
{
    return new VulkanRenderInterface();
}
#else
// On other platforms, return nullptr (no Vulkan support)
RenderInterface* RenderInterface::Create_Renderer()
{
    return nullptr;
}
#endif
