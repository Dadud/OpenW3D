/*
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

// DX12 backend implementation.

#include "dx12backend.h"
#include "backends/backend_surface_handle.h"

// Constants - visible to both Windows and non-Windows builds
#define DEFAULT_WIDTH 640
#define DEFAULT_HEIGHT 480
#define DEFAULT_BIT_DEPTH 32
#define DEFAULT_TEXTURE_DEPTH 32
#define FRAME_COUNT 2

// DX12 and DXGI headers - Windows only
#ifdef _WIN32
#include "rddesc.h"
#include <dxgi1_6.h>
#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <windows.h>

#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

// Helper toQIID a COM interface
template<typename Dest, typename Src>
static inline Dest* Castcom(Src* ptr) {
    if (!ptr) return nullptr;
    Dest* out = nullptr;
    ptr->QueryInterface(IID_PPV_ARGS(&out));
    return out;
}

// Helper to Release and null a COM object
template<typename T>
static inline void SafeRelease(T** ppObj) {
    if (*ppObj) {
        (*ppObj)->Release();
        *ppObj = nullptr;
    }
}

#define DEFAULT_WIDTH 640
#define DEFAULT_HEIGHT 480
#define DEFAULT_BIT_DEPTH 32
#define DEFAULT_TEXTURE_DEPTH 32
#define FRAME_COUNT 2

/************************************************************************************************
 * DX12Backend::DX12Backend -- Constructor                                                       *
 ************************************************************************************************/
DX12Backend::DX12Backend() :
    m_dxgi_factory(nullptr),
    m_adapter(nullptr),
    m_device(nullptr),
    m_command_queue(nullptr),
    m_swap_chain(nullptr),
    m_rtv_heap(nullptr),
    m_command_allocator(nullptr),
    m_command_list(nullptr),
    m_fence(nullptr),
    m_hwnd(nullptr),
    m_width(DEFAULT_WIDTH),
    m_height(DEFAULT_HEIGHT),
    m_bit_depth(DEFAULT_BIT_DEPTH),
    m_windowed(true),
    m_initialized(false),
    m_vsync(1),
    m_texture_bit_depth(DEFAULT_TEXTURE_DEPTH),
    m_current_device_index(0),
    m_render_device(0),
    m_render_target(nullptr),
    m_rtv_descriptor_size(0),
    m_fence_value(0),
    m_fence_event(nullptr)
{
}

/************************************************************************************************
 * DX12Backend::~DX12Backend -- Destructor                                                      *
 ************************************************************************************************/
DX12Backend::~DX12Backend()
{
    Shutdown();
}

/************************************************************************************************
 * DX12Backend::Shutdown -- Release all DX12 resources                                           *
 ************************************************************************************************/
void DX12Backend::Shutdown()
{
    if (m_device) {
        Wait_for_GPU();
    }

    SafeRelease(reinterpret_cast<IDXGISwapChain**>(&m_swap_chain));
    SafeRelease(reinterpret_cast<ID3D12DescriptorHeap**>(&m_rtv_heap));
    SafeRelease(reinterpret_cast<ID3D12CommandList**>(&m_command_list));
    SafeRelease(reinterpret_cast<ID3D12CommandAllocator**>(&m_command_allocator));
    SafeRelease(reinterpret_cast<ID3D12Fence**>(&m_fence));
    SafeRelease(reinterpret_cast<ID3D12Device**>(&m_device));
    SafeRelease(reinterpret_cast<IDXGIAdapter**>(&m_adapter));
    SafeRelease(reinterpret_cast<IDXGIFactory**>(&m_dxgi_factory));

    if (m_fence_event) {
        CloseHandle(m_fence_event);
        m_fence_event = nullptr;
    }

    m_initialized = false;
}

/************************************************************************************************
 * DX12Backend::Init -- Initialize DX12 device, command queue, and swap chain                  *
 ************************************************************************************************/
bool DX12Backend::Init(void * hwnd, bool /*lite*/)
{
    if (m_initialized) {
        Shutdown();
    }

    m_hwnd = hwnd;

    // Create DXGI Factory
    IDXGIFactory6* factory = nullptr;
    HRESULT hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));
    if (FAILED(hr)) {
        WWDEBUG_SAY(("DX12: CreateDXGIFactory2 failed: %x\n", hr));
        return false;
    }
    m_dxgi_factory = factory;

    // Enumerate adapters and create device with first suitable adapter
    IDXGIAdapter1* adapter = nullptr;
    for (unsigned int i = 0; factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);
        
        // Skip software adapters
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
            adapter->Release();
            continue;
        }

        // Try to create device on this adapter
        ID3D12Device* device = nullptr;
        hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
        if (SUCCEEDED(hr)) {
            m_adapter = adapter;
            m_device = device;
            m_current_device_index = i;
            break;
        }
        adapter->Release();
    }

    if (!m_device) {
        WWDEBUG_SAY(("DX12: No suitable adapter found\n"));
        SafeRelease(reinterpret_cast<IDXGIFactory**>(&m_dxgi_factory));
        return false;
    }

    // Create command queue
    D3D12_COMMAND_QUEUE_DESC queue_desc = {};
    queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    
    ID3D12CommandQueue* cmd_queue = nullptr;
    hr = static_cast<ID3D12Device*>(m_device)->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&cmd_queue));
    if (FAILED(hr)) {
        WWDEBUG_SAY(("DX12: CreateCommandQueue failed: %x\n", hr));
        return false;
    }
    m_command_queue = cmd_queue;

    // Create command allocator and list
    if (!Create_Command_Objects()) {
        return false;
    }

    // Create fence and event for synchronization
    ID3D12Fence* fence = nullptr;
    hr = static_cast<ID3D12Device*>(m_device)->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
    if (FAILED(hr)) {
        WWDEBUG_SAY(("DX12: CreateFence failed: %x\n", hr));
        return false;
    }
    m_fence = fence;
    m_fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!m_fence_event) {
        WWDEBUG_SAY(("DX12: CreateEvent failed\n"));
        return false;
    }

    // Get RTV descriptor size
    m_rtv_descriptor_size = static_cast<ID3D12Device*>(m_device)->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // Create swap chain
    if (!Create_Swapchain(m_width, m_height)) {
        return false;
    }

    m_initialized = true;
    return true;
}

/************************************************************************************************
 * DX12Backend::Create_Command_Objects -- Allocate command allocator and command list             *
 ************************************************************************************************/
bool DX12Backend::Create_Command_Objects()
{
    if (!m_device) return false;

    ID3D12CommandAllocator* alloc = nullptr;
    HRESULT hr = static_cast<ID3D12Device*>(m_device)->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&alloc));
    if (FAILED(hr)) {
        WWDEBUG_SAY(("DX12: CreateCommandAllocator failed: %x\n", hr));
        return false;
    }
    m_command_allocator = alloc;

    ID3D12GraphicsCommandList* list = nullptr;
    hr = static_cast<ID3D12Device*>(m_device)->CreateCommandList(
        0, D3D12_COMMAND_LIST_TYPE_DIRECT,
        static_cast<ID3D12CommandAllocator*>(m_command_allocator),
        nullptr, IID_PPV_ARGS(&list));
    if (FAILED(hr)) {
        WWDEBUG_SAY(("DX12: CreateCommandList failed: %x\n", hr));
        return false;
    }
    list->Close();
    m_command_list = list;

    return true;
}

/************************************************************************************************
 * DX12Backend::Wait_for_GPU -- Block until GPU is idle                                          *
 ************************************************************************************************/
void DX12Backend::Wait_for_GPU()
{
    if (!m_fence || !m_fence_event) return;

    ID3D12Fence* fence = static_cast<ID3D12Fence*>(m_fence);
    fence->Signal(m_fence_value);
    fence->SetEventOnCompletion(m_fence_value, m_fence_event);
    WaitForSingleObject(m_fence_event, INFINITE);
    m_fence_value++;
}

/************************************************************************************************
 * DX12Backend::Create_Swapchain -- Create or resize the DX12 swap chain                        *
 ************************************************************************************************/
bool DX12Backend::Create_Swapchain(int width, int height)
{
    if (!m_dxgi_factory || !m_command_queue) return false;

    Wait_for_GPU();

    // Release existing swap chain
    SafeRelease(reinterpret_cast<IDXGISwapChain**>(&m_swap_chain));
    SafeRelease(reinterpret_cast<ID3D12DescriptorHeap**>(&m_rtv_heap));

    DXGI_SWAP_CHAIN_DESC1 swap_desc = {};
    swap_desc.Width = width > 0 ? width : DEFAULT_WIDTH;
    swap_desc.Height = height > 0 ? height : DEFAULT_HEIGHT;
    swap_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_desc.BufferCount = FRAME_COUNT;
    swap_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swap_desc.SampleDesc.Count = 1;
    swap_desc.SampleDesc.Quality = 0;
    swap_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    IDXGISwapChain1* swap_chain = nullptr;
    HWND hwnd = static_cast<HWND>(m_hwnd);
    HRESULT hr = static_cast<IDXGIFactory*>(m_dxgi_factory)->CreateSwapChainForHwnd(
        static_cast<ID3D12CommandQueue*>(m_command_queue),
        hwnd,
        &swap_desc,
        nullptr,
        nullptr,
        &swap_chain);
    if (FAILED(hr)) {
        WWDEBUG_SAY(("DX12: CreateSwapChainForHwnd failed: %x\n", hr));
        return false;
    }

    // Disable Alt+Enter fullscreen toggle (we handle it ourselves)
    static_cast<IDXGIFactory*>(m_dxgi_factory)->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

    m_swap_chain = swap_chain;
    m_width = width > 0 ? width : DEFAULT_WIDTH;
    m_height = height > 0 ? height : DEFAULT_HEIGHT;

    // Create RTV descriptor heap for the swap chain buffers
    D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc = {};
    rtv_heap_desc.NumDescriptors = FRAME_COUNT;
    rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    ID3D12DescriptorHeap* rtv_heap = nullptr;
    hr = static_cast<ID3D12Device*>(m_device)->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(&rtv_heap));
    if (FAILED(hr)) {
        WWDEBUG_SAY(("DX12: CreateDescriptorHeap (RTV) failed: %x\n", hr));
        return false;
    }
    m_rtv_heap = rtv_heap;

    // Create RTVs for each frame
    D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = rtv_heap->GetCPUDescriptorHandleForHeapStart();
    for (unsigned int i = 0; i < FRAME_COUNT; i++) {
        ID3D12Resource* resource = nullptr;
        static_cast<IDXGISwapChain*>(m_swap_chain)->GetBuffer(i, IID_PPV_ARGS(&resource));
        if (resource) {
            static_cast<ID3D12Device*>(m_device)->CreateRenderTargetView(resource, nullptr, rtv_handle);
            resource->Release();
        }
        rtv_handle.ptr += m_rtv_descriptor_size;
    }

    return true;
}

/************************************************************************************************
 * DX12Backend::Begin_Scene -- Reset command list and prepare for rendering                     *
 ************************************************************************************************/
void DX12Backend::Begin_Scene()
{
    if (!m_command_allocator || !m_command_list) return;

    // Reset command allocator and list for new frame
    static_cast<ID3D12CommandAllocator*>(m_command_allocator)->Reset();
    static_cast<ID3D12GraphicsCommandList*>(m_command_list)->Reset(
        static_cast<ID3D12CommandAllocator*>(m_command_allocator), nullptr);

    // Set pipeline state and root signature here when we have them
    // For now, set render targets
    if (m_rtv_heap && m_swap_chain) {
        D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = static_cast<ID3D12DescriptorHeap*>(m_rtv_heap)->GetCPUDescriptorHandleForHeapStart();
        static_cast<ID3D12GraphicsCommandList*>(m_command_list)->OMSetRenderTargets(1, &rtv_handle, FALSE, nullptr);
    }
}

/************************************************************************************************
 * DX12Backend::End_Scene -- Execute command list and present                                   *
 ************************************************************************************************/
void DX12Backend::End_Scene(bool /*flip_frame*/)
{
    if (!m_command_list) return;

    // Close and execute command list
    static_cast<ID3D12GraphicsCommandList*>(m_command_list)->Close();
    ID3D12CommandList* cmd_lists[] = { static_cast<ID3D12GraphicsCommandList*>(m_command_list) };
    static_cast<ID3D12CommandQueue*>(m_command_queue)->ExecuteCommandLists(1, cmd_lists);

    // Present
    if (m_swap_chain) {
        unsigned int sync_interval = m_vsync ? 1 : 0;
        unsigned int flags = 0;
        static_cast<IDXGISwapChain*>(m_swap_chain)->Present(sync_interval, flags);
    }

    // Signal fence
    MoveToNextFrame();
}

/************************************************************************************************
 * DX12Backend::MoveToNextFrame -- Advance frame index and signal fence                         *
 ************************************************************************************************/
void DX12Backend::MoveToNextFrame()
{
    if (!m_fence || !m_command_queue) return;

    ID3D12Fence* fence = static_cast<ID3D12Fence*>(m_fence);
    ID3D12CommandQueue* cmd_queue = static_cast<ID3D12CommandQueue*>(m_command_queue);

    // Schedule a Signal command in the queue
    cmd_queue->Signal(fence, m_fence_value);

    // Wait until the next frame is ready
    fence->SetEventOnCompletion(m_fence_value, m_fence_event);
    WaitForSingleObject(m_fence_event, INFINITE);
    m_fence_value++;
}

/************************************************************************************************
 * DX12Backend::Clear -- Clear render target with color and/or depth                             *
 ************************************************************************************************/
void DX12Backend::Clear(bool clear_color, bool clear_z_stencil, const Vector3 &color, float z, unsigned int /*stencil*/)
{
    if (!m_command_list || !m_rtv_heap) return;

    ID3D12GraphicsCommandList* cmd_list = static_cast<ID3D12GraphicsCommandList*>(m_command_list);
    D3D12_CPU_DESCRIPTOR_HANDLE rtv = static_cast<ID3D12DescriptorHeap*>(m_rtv_heap)->GetCPUDescriptorHandleForHeapStart();

    if (clear_color) {
        float rgba[4] = { color.X, color.Y, color.Z, 1.0f };
        cmd_list->ClearRenderTargetView(rtv, rgba, 0, nullptr);
    }

    // Note: depth/stencil clear would need a DSV heap - deferred
}

/************************************************************************************************
 * DX12Backend::Flip_To_Primary -- Present the swap chain                                        *
 ************************************************************************************************/
void DX12Backend::Flip_To_Primary()
{
    // Present is handled in End_Scene
}

/************************************************************************************************
 * DX12Backend::Set_Swap_Interval -- Set vsync on/off                                           *
 ************************************************************************************************/
void DX12Backend::Set_Swap_Interval(int swap)
{
    m_vsync = (swap != 0) ? 1 : 0;
}

/************************************************************************************************
 * DX12Backend::Get_Swap_Interval -- Get current vsync setting                                  *
 ************************************************************************************************/
int DX12Backend::Get_Swap_Interval()
{
    return m_vsync;
}

/************************************************************************************************
 * DX12Backend::Set_Texture_Bitdepth -- Set preferred texture bitdepth                          *
 ************************************************************************************************/
void DX12Backend::Set_Texture_Bitdepth(int depth)
{
    m_texture_bit_depth = depth;
}

/************************************************************************************************
 * DX12Backend::Get_Texture_Bitdepth -- Get current texture bitdepth                             *
 ************************************************************************************************/
int DX12Backend::Get_Texture_Bitdepth()
{
    return m_texture_bit_depth;
}

/************************************************************************************************
 * DX12Backend::Set_Viewport -- Set the viewport for rendering                                  *
 ************************************************************************************************/
void DX12Backend::Set_Viewport(const void* viewport)
{
    if (!m_command_list || !viewport) return;

    // viewport points to a D3DVIEWPORT9 or generic viewport struct
    // Extract dimensions and set DX12 viewport
    int x = 0, y = 0, w = m_width, h = m_height;
    float minZ = 0.0f, maxZ = 1.0f;

    // Try to extract from D3DVIEWPORT9 if available
    // For now, use stored dimensions
    D3D12_VIEWPORT vp = {};
    vp.Width = static_cast<float>(w);
    vp.Height = static_cast<float>(h);
    vp.MinDepth = minZ;
    vp.MaxDepth = maxZ;
    vp.TopLeftX = static_cast<float>(x);
    vp.TopLeftY = static_cast<float>(y);

    static_cast<ID3D12GraphicsCommandList*>(m_command_list)->RSSetViewports(1, &vp);
}

/************************************************************************************************
 * DX12Backend::Set_Render_Target -- Bind a render target                                        *
 ************************************************************************************************/
void DX12Backend::Set_Render_Target(void* target)
{
    if (!m_command_list) return;

    // nullptr = reset to default (swap chain back buffer)
    if (target == nullptr) {
        if (m_rtv_heap) {
            D3D12_CPU_DESCRIPTOR_HANDLE rtv = static_cast<ID3D12DescriptorHeap*>(m_rtv_heap)->GetCPUDescriptorHandleForHeapStart();
            static_cast<ID3D12GraphicsCommandList*>(m_command_list)->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
        }
        return;
    }

    // target would be a BackendSurfaceHandle* in a full implementation
    // For now, just use the swap chain back buffer
    Set_Render_Target(nullptr);
}

/************************************************************************************************
 * DX12Backend::Set_DX8_Render_State -- Store render state (DX12 translation deferred)          *
 ************************************************************************************************/
void DX12Backend::Set_DX8_Render_State(int state, unsigned value)
{
    // DX8 render state to DX12 translation would happen here
    // For now, just store it for later use when creating pipeline state
    (void)state;
    (void)value;
}

/************************************************************************************************
 * DX12Backend::Set_Light_Environment -- Store light environment (stub)                         *
 ************************************************************************************************/
void DX12Backend::Set_Light_Environment(const void* /*env*/)
{
    // DX12 equivalent - deferred until pipeline state is implemented
}

/************************************************************************************************
 * DX12Backend::Get_Front_Buffer_Surface -- Get the current back buffer as a surface            *
 ************************************************************************************************/
void DX12Backend::Get_Front_Buffer_Surface(BackendSurfaceHandle* out_handle)
{
    if (!out_handle || !m_swap_chain) {
        if (out_handle) out_handle->D3DSurface = nullptr;
        return;
    }

    // Get the current back buffer
    ID3D12Resource* buffer = nullptr;
    unsigned int idx = static_cast<IDXGISwapChain*>(m_swap_chain)->GetCurrentBackBufferIndex();
    static_cast<IDXGISwapChain*>(m_swap_chain)->GetBuffer(idx, IID_PPV_ARGS(&buffer));
    
    if (buffer) {
        out_handle->D3DSurface = static_cast<IDirect3DSurface9*>(buffer); // Note: this is actually a DX12 resource, not D3D9
        out_handle->BackendData = buffer;
    } else {
        out_handle->D3DSurface = nullptr;
        out_handle->BackendData = nullptr;
    }
}

/************************************************************************************************
 * DX12Backend::Lock_Front_Buffer_Surface -- Lock the front buffer for reading                 *
 ************************************************************************************************/
void DX12Backend::Lock_Front_Buffer_Surface(BackendSurfaceHandle* handle, int width, int height, SurfaceLockData* out_data)
{
    if (!out_data) return;
    out_data->Valid = false;
    out_data->PixelData = nullptr;
    out_data->RowPitch = 0;

    // DX12 doesn't support CPU reading of back buffers directly
    // Would need to copy to a staging resource - deferred
}

/************************************************************************************************
 * DX12Backend::Unlock_Front_Buffer_Surface -- Unlock the front buffer                          *
 ************************************************************************************************/
void DX12Backend::Unlock_Front_Buffer_Surface(BackendSurfaceHandle* handle)
{
    if (handle && handle->BackendData) {
        // Release the staging resource if one was created
        static_cast<ID3D12Resource*>(handle->BackendData)->Release();
        handle->BackendData = nullptr;
    }
    handle->D3DSurface = nullptr;
}

//=============================================================================
// Device enumeration stubs - return minimal valid data
//=============================================================================

bool DX12Backend::Set_Any_Render_Device() { return m_initialized; }
bool DX12Backend::Set_Render_Device(const char*, int, int, int, int, bool) { return m_initialized; }
bool DX12Backend::Set_Render_Device(int, int, int, int, int, bool) { return m_initialized; }
bool DX12Backend::Set_Next_Render_Device() { return m_initialized; }
bool DX12Backend::Toggle_Windowed() { m_windowed = !m_windowed; return true; }
bool DX12Backend::Is_Windowed() { return m_windowed; }

int DX12Backend::Get_Render_Device_Count()
{
    if (!m_dxgi_factory) return 1;
    unsigned int count = 0;
    IDXGIAdapter1* adapter = nullptr;
    while (static_cast<IDXGIFactory*>(m_dxgi_factory)->EnumAdapters1(count, &adapter) != DXGI_ERROR_NOT_FOUND) {
        count++;
        if (adapter) adapter->Release();
    }
    return count > 0 ? count : 1;
}

int DX12Backend::Get_Render_Device() { return m_render_device; }

const char* DX12Backend::Get_Render_Device_Name(int device_index)
{
    static char name[256] = "DX12 Adapter";
    if (m_adapter) {
        DXGI_ADAPTER_DESC1 desc;
        static_cast<IDXGIAdapter*>(m_adapter)->GetDesc1(&desc);
        WideCharToMultiByte(CP_ACP, 0, desc.Description, -1, name, sizeof(name), nullptr, nullptr);
    }
    return name;
}

const RenderDeviceDescClass & DX12Backend::Get_Render_Device_Desc(int /*deviceidx*/)
{
    static RenderDeviceDescClass desc;
    return desc;
}

bool DX12Backend::Set_Device_Resolution(int width, int height, int bits, int windowed, bool)
{
    if (width > 0 && height > 0) {
        m_width = width;
        m_height = height;
    }
    if (bits > 0) m_bit_depth = bits;
    if (windowed >= 0) m_windowed = (windowed != 0);

    if (m_initialized) {
        Create_Swapchain(m_width, m_height);
    }
    return true;
}

void DX12Backend::Get_Device_Resolution(int & w, int & h, int & bits, bool & windowed)
{
    w = m_width;
    h = m_height;
    bits = m_bit_depth;
    windowed = m_windowed;
}

void DX12Backend::Get_Render_Target_Resolution(int & w, int & h, int & bits, bool & windowed)
{
    Get_Device_Resolution(w, h, bits, windowed);
}

int DX12Backend::Get_Device_Resolution_Width() { return m_width; }
int DX12Backend::Get_Device_Resolution_Height() { return m_height; }

#else // !_WIN32

// DX12 is Windows-only - provide stubs for non-Windows builds
struct RenderDeviceDescClass {};

DX12Backend::DX12Backend() :
    m_dxgi_factory(nullptr),
    m_adapter(nullptr),
    m_device(nullptr),
    m_command_queue(nullptr),
    m_swap_chain(nullptr),
    m_rtv_heap(nullptr),
    m_command_allocator(nullptr),
    m_command_list(nullptr),
    m_fence(nullptr),
    m_hwnd(nullptr),
    m_width(DEFAULT_WIDTH),
    m_height(DEFAULT_HEIGHT),
    m_bit_depth(DEFAULT_BIT_DEPTH),
    m_windowed(true),
    m_initialized(false),
    m_vsync(1),
    m_texture_bit_depth(DEFAULT_TEXTURE_DEPTH),
    m_current_device_index(0),
    m_render_device(0),
    m_render_target(nullptr),
    m_rtv_descriptor_size(0),
    m_fence_value(0),
    m_fence_event(nullptr)
{
}

DX12Backend::~DX12Backend() { Shutdown(); }
void DX12Backend::Shutdown() { m_initialized = false; }
bool DX12Backend::Init(void*, bool) { return false; }
bool DX12Backend::Set_Any_Render_Device() { return false; }
bool DX12Backend::Set_Render_Device(const char*, int, int, int, int, bool) { return false; }
bool DX12Backend::Set_Render_Device(int, int, int, int, int, bool) { return false; }
bool DX12Backend::Set_Next_Render_Device() { return false; }
bool DX12Backend::Toggle_Windowed() { return false; }
bool DX12Backend::Is_Windowed() { return false; }
bool DX12Backend::Create_Swapchain(int, int) { return false; }
void DX12Backend::Begin_Scene() {}
void DX12Backend::End_Scene(bool) {}
void DX12Backend::Flip_To_Primary() {}
void DX12Backend::Clear(bool, bool, const Vector3&, float, unsigned) {}
void DX12Backend::Set_Swap_Interval(int) {}
int DX12Backend::Get_Swap_Interval() { return 0; }
void DX12Backend::Set_Texture_Bitdepth(int) {}
int DX12Backend::Get_Texture_Bitdepth() { return 0; }
void DX12Backend::Set_Viewport(const void*) {}
void DX12Backend::Set_Render_Target(void*) {}
void DX12Backend::Set_DX8_Render_State(int, unsigned) {}
void DX12Backend::Set_Light_Environment(const void*) {}
void DX12Backend::Get_Front_Buffer_Surface(BackendSurfaceHandle* h) { if (h) { h->D3DSurface = nullptr; h->BackendData = nullptr; } }
void DX12Backend::Lock_Front_Buffer_Surface(BackendSurfaceHandle*, int, int, SurfaceLockData* d) { if (d) d->Valid = false; }
void DX12Backend::Unlock_Front_Buffer_Surface(BackendSurfaceHandle*) {}
int DX12Backend::Get_Render_Device_Count() { return 0; }
int DX12Backend::Get_Render_Device() { return 0; }
const char* DX12Backend::Get_Render_Device_Name(int) { return "DX12 (unavailable on this platform)"; }
const RenderDeviceDescClass& DX12Backend::Get_Render_Device_Desc(int) { static RenderDeviceDescClass _stub_dummy; return _stub_dummy; }
bool DX12Backend::Set_Device_Resolution(int, int, int, int, bool) { return false; }
void DX12Backend::Get_Device_Resolution(int& w, int& h, int& b, bool& win) { w = 0; h = 0; b = 0; win = false; }
void DX12Backend::Get_Render_Target_Resolution(int& w, int& h, int& b, bool& win) { w = 0; h = 0; b = 0; win = false; }
int DX12Backend::Get_Device_Resolution_Width() { return 0; }
int DX12Backend::Get_Device_Resolution_Height() { return 0; }
bool DX12Backend::Registry_Save_Render_Device(const char*) { return true; }
bool DX12Backend::Registry_Load_Render_Device(const char*, bool) { return true; }
bool DX12Backend::Registry_Save_Render_Device(const char*, int, int, int, int, bool, int) { return true; }
bool DX12Backend::Registry_Load_Render_Device(const char*, char*, int, int&, int&, int&, int&, int&) { return true; }

#endif // !_WIN32
