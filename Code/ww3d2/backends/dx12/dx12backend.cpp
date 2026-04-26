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
    m_pipeline_state(nullptr),
    m_srv_heap(nullptr),
    m_dsv_heap(nullptr),
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
    m_default_render_target(nullptr),
    m_rtv_descriptor_size(0),
    m_srv_descriptor_size(0),
    m_dsv_descriptor_size(0),
    m_fence_value(0),
    m_fence_event(nullptr),
    m_staging_texture(nullptr),
    m_staging_width(0),
    m_staging_height(0),
    m_dx8_fill_mode(D3DFILL_SOLID),
    m_dx8_cull_mode(D3DCULL_NONE),
    m_dx8_zenable(D3DZB_TRUE),
    m_dx8_fill_solid(1),
    m_state_dirty(false),
    m_root_signature(nullptr),
    m_dirty_matrix(false),
    m_lighting_enabled(true),
    m_fog_enabled(false),
    m_vs_blob(nullptr),
    m_ps_blob(nullptr),
    m_stored_shader(nullptr),
    m_vertex_buffer(nullptr),
    m_index_buffer(nullptr),
    m_vertex_buffer_stride(0),
    m_vertex_buffer_offset(0),
    m_vertex_count(0),
    m_index_count(0),
    m_vertex_buffer_view({}),
    m_index_buffer_view({})
{
    // Identity matrices
    const float identity[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    memcpy(m_world_matrix, identity, sizeof(m_world_matrix));
    memcpy(m_view_matrix, identity, sizeof(m_view_matrix));
    memcpy(m_projection_matrix, identity, sizeof(m_projection_matrix));
    for (unsigned int i = 0; i < 8; i++) {
        m_bound_textures[i] = nullptr;
        m_textures[i].resource = nullptr;
        m_textures[i].width = 0;
        m_textures[i].height = 0;
        m_textures[i].stride = 0;
    }
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
    // Release all texture resources
    for (unsigned int i = 0; i < 8; i++) {
        if (m_textures[i].resource) {
            SafeRelease(reinterpret_cast<ID3D12Resource**>(&m_textures[i].resource));
            m_textures[i].resource = nullptr;
            m_textures[i].width = 0;
            m_textures[i].height = 0;
            m_textures[i].stride = 0;
        }
    }

    if (m_device) {
        Wait_for_GPU();
    }

    SafeRelease(reinterpret_cast<IDXGISwapChain**>(&m_swap_chain));
    SafeRelease(reinterpret_cast<ID3D12DescriptorHeap**>(&m_rtv_heap));
    SafeRelease(reinterpret_cast<ID3D12DescriptorHeap**>(&m_srv_heap));
    SafeRelease(reinterpret_cast<ID3D12DescriptorHeap**>(&m_dsv_heap));
    SafeRelease(reinterpret_cast<ID3D12Resource**>(&m_depthStencil));
    SafeRelease(reinterpret_cast<ID3D12PipelineState**>(&m_pipeline_state));
    SafeRelease(reinterpret_cast<ID3D12RootSignature**>(&m_root_signature));
    SafeRelease(reinterpret_cast<ID3DBlob**>(&m_vs_blob));
    SafeRelease(reinterpret_cast<ID3DBlob**>(&m_ps_blob));
    SafeRelease(reinterpret_cast<ID3D12Resource**>(&m_vertex_buffer));
    SafeRelease(reinterpret_cast<ID3D12Resource**>(&m_index_buffer));
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

    // Create descriptor heaps (SRV and DSV)
    if (!Create_Descriptor_Heaps()) {
        return false;
    }

    // Create depth stencil (needs DSV heap from Create_Descriptor_Heaps)
    // Note: dimensions come from Create_Swapchain below

    // Create swap chain
    if (!Create_Swapchain(m_width, m_height)) {
        return false;
    }

    // Create depth stencil after swap chain so we have correct dimensions
    if (!Create_DepthStencil()) {
        return false;
    }

    // Create default render target view for the swap chain back buffer
    if (!Create_Default_Render_Target()) {
        return false;
    }

    // Create default PSO
    if (!Create_Default_PSO()) {
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
 * DX12Backend::Create_Descriptor_Heaps -- Create SRV and DSV descriptor heaps                   *
 ************************************************************************************************/
bool DX12Backend::Create_Descriptor_Heaps()
{
    if (!m_device) return false;


    // Create SRV heap for shader resource views (textures, constant buffers)
    D3D12_DESCRIPTOR_HEAP_DESC srv_heap_desc = {};
    srv_heap_desc.NumDescriptors = 256;
    srv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    ID3D12DescriptorHeap* srv_heap = nullptr;
    HRESULT hr = static_cast<ID3D12Device*>(m_device)->CreateDescriptorHeap(&srv_heap_desc, IID_PPV_ARGS(&srv_heap));
    if (FAILED(hr)) {
        WWDEBUG_SAY(("DX12: CreateDescriptorHeap (SRV) failed: %x\n", hr));
        return false;
    }
    m_srv_heap = srv_heap;
    m_srv_descriptor_size = static_cast<ID3D12Device*>(m_device)->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // Create DSV heap for depth/stencil views
    D3D12_DESCRIPTOR_HEAP_DESC dsv_heap_desc = {};
    dsv_heap_desc.NumDescriptors = 16;
    dsv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    ID3D12DescriptorHeap* dsv_heap = nullptr;
    hr = static_cast<ID3D12Device*>(m_device)->CreateDescriptorHeap(&dsv_heap_desc, IID_PPV_ARGS(&dsv_heap));
    if (FAILED(hr)) {
        WWDEBUG_SAY(("DX12: CreateDescriptorHeap (DSV) failed: %x\n", hr));
        return false;
    }
    m_dsv_heap = dsv_heap;
    m_dsv_descriptor_size = static_cast<ID3D12Device*>(m_device)->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    return true;
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
    SafeRelease(reinterpret_cast<ID3D12Resource**>(&m_staging_texture));

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
 * DX12Backend::Create_Default_Render_Target -- Create RTV for swap chain back buffer          *
 ************************************************************************************************/
bool DX12Backend::Create_Default_Render_Target()
{
    if (!m_device || !m_swap_chain) return false;

    // Get the current back buffer
    ID3D12Resource* back_buffer = nullptr;
    unsigned int idx = static_cast<IDXGISwapChain*>(m_swap_chain)->GetCurrentBackBufferIndex();
    HRESULT hr = static_cast<IDXGISwapChain*>(m_swap_chain)->GetBuffer(idx, IID_PPV_ARGS(&back_buffer));
    if (FAILED(hr)) {
        WWDEBUG_SAY(("DX12: GetBuffer for default render target failed: %x\n", hr));
        return false;
    }

    // Store the back buffer resource as the default render target
    m_default_render_target = back_buffer;

    // Create RTV for the default render target
    D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = static_cast<ID3D12DescriptorHeap*>(m_rtv_heap)->GetCPUDescriptorHandleForHeapStart();
    static_cast<ID3D12Device*>(m_device)->CreateRenderTargetView(back_buffer, nullptr, rtv_handle);

    return true;
}

/************************************************************************************************
 * DX12Backend::Rebuild_PSO_From_State -- Rebuild PSO from current DX8 render state           *
 ************************************************************************************************/
bool DX12Backend::Rebuild_PSO_From_State()
{
    if (!m_device) return false;

    SafeRelease(reinterpret_cast<ID3D12PipelineState**>(&m_pipeline_state));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};

    // Root signature
    pso_desc.pRootSignature = static_cast<ID3D12RootSignature*>(m_root_signature);

    // Vertex shader - from blob if available
    if (m_vs_blob) {
        ID3DBlob* vs = static_cast<ID3DBlob*>(m_vs_blob);
        pso_desc.VS.pShaderBytecode = vs->GetBufferPointer();
        pso_desc.VS.BytecodeLength = vs->GetBufferSize();
    } else {
        pso_desc.VS.pShaderBytecode = nullptr;
        pso_desc.VS.BytecodeLength = 0;
    }

    // Pixel shader - from blob if available
    if (m_ps_blob) {
        ID3DBlob* ps = static_cast<ID3DBlob*>(m_ps_blob);
        pso_desc.PS.pShaderBytecode = ps->GetBufferPointer();
        pso_desc.PS.BytecodeLength = ps->GetBufferSize();
    } else {
        pso_desc.PS.pShaderBytecode = nullptr;
        pso_desc.PS.BytecodeLength = 0;
    }

    // Rasterizer state from DX8 render state
    switch (m_dx8_fill_mode) {
        case D3DFILL_WIREFRAME:
            pso_desc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
            break;
        case D3DFILL_POINT:
            // DX12 has no POINT fill - fall back to wireframe
            pso_desc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
            break;
        case D3DFILL_SOLID:
        default:
            pso_desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
            break;
    }

    switch (m_dx8_cull_mode) {
        case D3DCULL_FRONT:
            pso_desc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
            break;
        case D3DCULL_BACK:
            pso_desc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
            break;
        case D3DCULL_NONE:
        default:
            pso_desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
            break;
    }

    pso_desc.RasterizerState.FrontCounterClockwise = FALSE;
    pso_desc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    pso_desc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    pso_desc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    pso_desc.RasterizerState.DepthClipEnable = TRUE;
    pso_desc.RasterizerState.MultisampleEnable = FALSE;
    pso_desc.RasterizerState.AntialiasedLineEnable = FALSE;
    pso_desc.RasterizerState.ForcedSampleCount = 0;
    pso_desc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    // Blend state - no blending
    pso_desc.BlendState.AlphaToCoverageEnable = FALSE;
    pso_desc.BlendState.IndependentBlendEnable = FALSE;
    for (unsigned i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++) {
        pso_desc.BlendState.RenderTarget[i].BlendEnable = FALSE;
        pso_desc.BlendState.RenderTarget[i].LogicOpEnable = FALSE;
        pso_desc.BlendState.RenderTarget[i].LogicOp = D3D12_LOGIC_OP_NOOP;
        pso_desc.BlendState.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    }

    // Depth stencil state from DX8 z-enable
    switch (m_dx8_zenable) {
        case D3DZB_FALSE:
            pso_desc.DepthStencilState.DepthEnable = FALSE;
            pso_desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
            break;
        case D3DZB_TRUE:
        default:
            pso_desc.DepthStencilState.DepthEnable = TRUE;
            pso_desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
            break;
    }
    pso_desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    pso_desc.DepthStencilState.StencilEnable = FALSE;
    pso_desc.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    pso_desc.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;

    // Input layout - none
    pso_desc.InputLayout.pInputElements = nullptr;
    pso_desc.InputLayout.NumElements = 0;

    // Primitive topology
    pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    // Render target formats
    pso_desc.NumRenderTargets = 1;
    pso_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    for (unsigned i = 1; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++) {
        pso_desc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
    }

    // Depth stencil format
    pso_desc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    // Sample desc
    pso_desc.SampleDesc.Count = 1;
    pso_desc.SampleDesc.Quality = 0;
    pso_desc.SampleMask = UINT_MAX;

    HRESULT hr = static_cast<ID3D12Device*>(m_device)->CreateGraphicsPipelineState(
        &pso_desc, IID_PPV_ARGS(reinterpret_cast<ID3D12PipelineState**>(&m_pipeline_state)));
    if (FAILED(hr)) {
        WWDEBUG_SAY(("DX12: CreateGraphicsPipelineState failed: %x\n", hr));
        return false;
    }

    m_state_dirty = false;
    return true;
}

/************************************************************************************************
 * DX12Backend::Create_Default_Shaders -- Compile minimal VS and PS shaders                    *
 ************************************************************************************************/
bool DX12Backend::Create_Default_Shaders()
{
#if defined(_WIN32) && defined(D3DCompile)
    // Only available on Windows with D3DCompiler
    ID3DBlob* vs_blob = nullptr;
    ID3DBlob* ps_blob = nullptr;
    ID3DBlob* error_blob = nullptr;

    // Minimal vertex shader: outputs position in clip space, passes through color
    const char* vs_hlsl = R"(
        struct VS_INPUT {
            float3 position : POSITION;
            float4 color : COLOR;
        };
        struct VS_OUTPUT {
            float4 position : SV_POSITION;
            float4 color : COLOR;
        };
        VS_OUTPUT main(VS_INPUT input) {
            VS_OUTPUT output;
            output.position = float4(input.position, 1.0f);
            output.color = input.color;
            return output;
        }
    )";

    HRESULT hr = D3DCompile(vs_hlsl, strlen(vs_hlsl), "VS", nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_4_0",
        D3D10_SHADER_ENABLE_STRICTNESS | D3D10_SHADER_PACK_MATRIX_ROW_MAJOR, 0,
        &vs_blob, &error_blob);
    if (FAILED(hr)) {
        WWDEBUG_SAY(("DX12: VS compilation failed: %x\n", hr));
        if (error_blob) error_blob->Release();
        return false;
    }
    SafeRelease(&error_blob);

    // Minimal pixel shader: outputs flat color
    const char* ps_hlsl = R"(
        struct PS_INPUT {
            float4 position : SV_POSITION;
            float4 color : COLOR;
        };
        float4 main(PS_INPUT input) : SV_TARGET {
            return input.color;
        }
    )";

    hr = D3DCompile(ps_hlsl, strlen(ps_hlsl), "PS", nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_4_0",
        D3D10_SHADER_ENABLE_STRICTNESS | D3D10_SHADER_PACK_MATRIX_ROW_MAJOR, 0,
        &ps_blob, &error_blob);
    if (FAILED(hr)) {
        WWDEBUG_SAY(("DX12: PS compilation failed: %x\n", hr));
        if (error_blob) error_blob->Release();
        vs_blob->Release();
        return false;
    }

    m_vs_blob = vs_blob;
    m_ps_blob = ps_blob;
    return true;
#else
    // No D3DCompiler available - stub shaders
    WWDEBUG_SAY(("DX12: D3DCompile not available, using null shaders\n"));
    return true;
#endif
}

/************************************************************************************************
 * DX12Backend::Create_Default_PSO -- Create a minimal pass-through graphics PSO                *
 ************************************************************************************************/
bool DX12Backend::Create_Default_PSO()
{
    // Create the default shaders first
    if (!Create_Default_Shaders()) {
        WWDEBUG_SAY(("DX12: Create_Default_Shaders failed, using null shaders\n"));
    }

    // Use member variables for consistency with Rebuild_PSO_From_State
    return Rebuild_PSO_From_State();
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

    // Set pipeline state
    if (m_pipeline_state) {
        static_cast<ID3D12GraphicsCommandList*>(m_command_list)->SetPipelineState(
            reinterpret_cast<ID3D12PipelineState*>(m_pipeline_state));
    }

    // Apply matrix transforms
    Apply_Matrices();

    // Set render targets
    if (m_rtv_heap && m_swap_chain) {
        D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = static_cast<ID3D12DescriptorHeap*>(m_rtv_heap)->GetCPUDescriptorHandleForHeapStart();
        D3D12_CPU_DESCRIPTOR_HANDLE dsv_handle = static_cast<ID3D12DescriptorHeap*>(m_dsv_heap)->GetCPUDescriptorHandleForHeapStart();
        static_cast<ID3D12GraphicsCommandList*>(m_command_list)->OMSetRenderTargets(1, &rtv_handle, FALSE, &dsv_handle);
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
 * DX12Backend::Create_Vertex_Buffer -- Allocate a DEFAULT heap vertex buffer                    *
 ************************************************************************************************/
bool DX12Backend::Create_Vertex_Buffer(unsigned int size_bytes)
{
    if (!m_device || size_bytes == 0) return false;

    SafeRelease(reinterpret_cast<ID3D12Resource**>(&m_vertex_buffer));

    D3D12_HEAP_PROPERTIES heap_props = {};
    heap_props.Type = D3D12_HEAP_TYPE_DEFAULT;
    heap_props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heap_props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heap_props.MipLevels = 1;
    heap_props.PlaneMipDepth = 1;
    heap_props.Flags = D3D12_HEAP_FLAG_NONE;

    D3D12_RESOURCE_DESC res_desc = {};
    res_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    res_desc.Width = size_bytes;
    res_desc.Height = 1;
    res_desc.MipLevels = 1;
    res_desc.DepthOrArraySize = 1;
    res_desc.Format = DXGI_FORMAT_UNKNOWN;
    res_desc.SampleDesc.Count = 1;
    res_desc.SampleDesc.Quality = 0;
    res_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    res_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ID3D12Resource* vb = nullptr;
    HRESULT hr = static_cast<ID3D12Device*>(m_device)->CreateCommittedResource(
        &heap_props,
        D3D12_HEAP_FLAG_NONE,
        &res_desc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&vb));
    if (FAILED(hr)) {
        WWDEBUG_SAY(("DX12: CreateCommittedResource (vertex buffer) failed: %x\n", hr));
        return false;
    }
    m_vertex_buffer = vb;
    return true;
}

/************************************************************************************************
 * DX12Backend::Create_Index_Buffer -- Allocate a DEFAULT heap index buffer                      *
 ************************************************************************************************/
bool DX12Backend::Create_Index_Buffer(unsigned int size_bytes)
{
    if (!m_device || size_bytes == 0) return false;

    SafeRelease(reinterpret_cast<ID3D12Resource**>(&m_index_buffer));

    D3D12_HEAP_PROPERTIES heap_props = {};
    heap_props.Type = D3D12_HEAP_TYPE_DEFAULT;
    heap_props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heap_props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heap_props.MipLevels = 1;
    heap_props.PlaneMipDepth = 1;
    heap_props.Flags = D3D12_HEAP_FLAG_NONE;

    D3D12_RESOURCE_DESC res_desc = {};
    res_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    res_desc.Width = size_bytes;
    res_desc.Height = 1;
    res_desc.MipLevels = 1;
    res_desc.DepthOrArraySize = 1;
    res_desc.Format = DXGI_FORMAT_UNKNOWN;
    res_desc.SampleDesc.Count = 1;
    res_desc.SampleDesc.Quality = 0;
    res_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    res_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ID3D12Resource* ib = nullptr;
    HRESULT hr = static_cast<ID3D12Device*>(m_device)->CreateCommittedResource(
        &heap_props,
        D3D12_HEAP_FLAG_NONE,
        &res_desc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&ib));
    if (FAILED(hr)) {
        WWDEBUG_SAY(("DX12: CreateCommittedResource (index buffer) failed: %x\n", hr));
        return false;
    }
    m_index_buffer = ib;
    return true;
}

/************************************************************************************************
 * DX12Backend::Set_Vertex_Buffer -- Upload vertex data to the GPU buffer                       *
 ************************************************************************************************/
void DX12Backend::Set_Vertex_Buffer(void* data, unsigned int stride, unsigned int vertex_count)
{
    if (!data || !m_device || !m_command_list) return;

    unsigned int size_bytes = stride * vertex_count;
    if (size_bytes == 0) return;

    // Create buffer if needed or too small
    ID3D12Resource* vb = static_cast<ID3D12Resource*>(m_vertex_buffer);
    if (!vb) {
        if (!Create_Vertex_Buffer(size_bytes)) return;
        vb = static_cast<ID3D12Resource*>(m_vertex_buffer);
    }

    // Store metadata
    m_vertex_buffer_stride = stride;
    m_vertex_buffer_offset = 0;
    m_vertex_count = vertex_count;

    // Create upload heap and copy data
    D3D12_HEAP_PROPERTIES upload_heap_props = {};
    upload_heap_props.Type = D3D12_HEAP_TYPE_UPLOAD;
    upload_heap_props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    upload_heap_props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    upload_heap_props.MipLevels = 1;
    upload_heap_props.PlaneMipDepth = 1;
    upload_heap_props.Flags = D3D12_HEAP_FLAG_NONE;

    D3D12_RESOURCE_DESC upload_res_desc = {};
    upload_res_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    upload_res_desc.Width = size_bytes;
    upload_res_desc.Height = 1;
    upload_res_desc.MipLevels = 1;
    upload_res_desc.DepthOrArraySize = 1;
    upload_res_desc.Format = DXGI_FORMAT_UNKNOWN;
    upload_res_desc.SampleDesc.Count = 1;
    upload_res_desc.SampleDesc.Quality = 0;
    upload_res_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    upload_res_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ID3D12Resource* upload_buffer = nullptr;
    HRESULT hr = static_cast<ID3D12Device*>(m_device)->CreateCommittedResource(
        &upload_heap_props,
        D3D12_HEAP_FLAG_NONE,
        &upload_res_desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&upload_buffer));
    if (FAILED(hr)) {
        WWDEBUG_SAY(("DX12: CreateCommittedResource (upload buffer) failed: %x\n", hr));
        return;
    }

    // Copy data to upload buffer
    D3D12_SUBRESOURCE_DATA sub_data = {};
    sub_data.pData = data;
    sub_data.RowPitch = size_bytes;
    sub_data.SlicePitch = size_bytes;

    ID3D12GraphicsCommandList* cmd_list = static_cast<ID3D12GraphicsCommandList*>(m_command_list);

    // Transition vertex buffer to COPY_DEST
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = vb;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    cmd_list->ResourceBarrier(1, &barrier);

    // Upload data via staging buffer
    D3D12_RANGE read_range{0, 0};
    void* mapped = nullptr;
    upload_buffer->Map(0, &read_range, &mapped);
    if (mapped) {
        memcpy(mapped, data, size_bytes);
    }
    upload_buffer->Unmap(0, nullptr);

    // Copy from upload to vertex buffer
    cmd_list->CopyBufferRegion(vb, 0, upload_buffer, 0, size_bytes);

    // Transition back to vertex buffer state
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    cmd_list->ResourceBarrier(1, &barrier);

    // Build vertex buffer view
    m_vertex_buffer_view.BufferLocation = vb->GetGPUVirtualAddress();
    m_vertex_buffer_view.SizeInBytes = size_bytes;
    m_vertex_buffer_view.StrideInBytes = stride;

    // Execute and wait
    cmd_list->Close();
    ID3D12CommandList* cmd_lists[] = { cmd_list };
    static_cast<ID3D12CommandQueue*>(m_command_queue)->ExecuteCommandLists(1, cmd_lists);
    Wait_for_GPU();
    static_cast<ID3D12CommandAllocator*>(m_command_allocator)->Reset();
    cmd_list->Reset(static_cast<ID3D12CommandAllocator*>(m_command_allocator), nullptr);

    upload_buffer->Release();
}

/************************************************************************************************
 * DX12Backend::Set_Index_Buffer -- Upload index data to the GPU buffer                         *
 ************************************************************************************************/
void DX12Backend::Set_Index_Buffer(void* data, unsigned int index_count)
{
    if (!data || !m_device || !m_command_list) return;

    // Use 16-bit indices by default
    unsigned int size_bytes = sizeof(unsigned short) * index_count;
    if (size_bytes == 0) return;

    // Create buffer if needed or too small
    ID3D12Resource* ib = static_cast<ID3D12Resource*>(m_index_buffer);
    if (!ib) {
        if (!Create_Index_Buffer(size_bytes)) return;
        ib = static_cast<ID3D12Resource*>(m_index_buffer);
    }

    // Store metadata
    m_index_count = index_count;

    // Create upload heap and copy data
    D3D12_HEAP_PROPERTIES upload_heap_props = {};
    upload_heap_props.Type = D3D12_HEAP_TYPE_UPLOAD;
    upload_heap_props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    upload_heap_props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    upload_heap_props.MipLevels = 1;
    upload_heap_props.PlaneMipDepth = 1;
    upload_heap_props.Flags = D3D12_HEAP_FLAG_NONE;

    D3D12_RESOURCE_DESC upload_res_desc = {};
    upload_res_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    upload_res_desc.Width = size_bytes;
    upload_res_desc.Height = 1;
    upload_res_desc.MipLevels = 1;
    upload_res_desc.DepthOrArraySize = 1;
    upload_res_desc.Format = DXGI_FORMAT_UNKNOWN;
    upload_res_desc.SampleDesc.Count = 1;
    upload_res_desc.SampleDesc.Quality = 0;
    upload_res_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    upload_res_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ID3D12Resource* upload_buffer = nullptr;
    HRESULT hr = static_cast<ID3D12Device*>(m_device)->CreateCommittedResource(
        &upload_heap_props,
        D3D12_HEAP_FLAG_NONE,
        &upload_res_desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&upload_buffer));
    if (FAILED(hr)) {
        WWDEBUG_SAY(("DX12: CreateCommittedResource (upload buffer) failed: %x\n", hr));
        return;
    }

    ID3D12GraphicsCommandList* cmd_list = static_cast<ID3D12GraphicsCommandList*>(m_command_list);

    // Transition index buffer to COPY_DEST
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = ib;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_INDEX_BUFFER;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    cmd_list->ResourceBarrier(1, &barrier);

    // Copy data to upload buffer
    D3D12_RANGE read_range{0, 0};
    void* mapped = nullptr;
    upload_buffer->Map(0, &read_range, &mapped);
    if (mapped) {
        memcpy(mapped, data, size_bytes);
    }
    upload_buffer->Unmap(0, nullptr);

    // Copy from upload to index buffer
    cmd_list->CopyBufferRegion(ib, 0, upload_buffer, 0, size_bytes);

    // Transition back to index buffer state
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_INDEX_BUFFER;
    cmd_list->ResourceBarrier(1, &barrier);

    // Build index buffer view (using 16-bit indices)
    m_index_buffer_view.BufferLocation = ib->GetGPUVirtualAddress();
    m_index_buffer_view.SizeInBytes = size_bytes;
    m_index_buffer_view.Format = DXGI_FORMAT_R16_UINT;

    // Execute and wait
    cmd_list->Close();
    ID3D12CommandList* cmd_lists[] = { cmd_list };
    static_cast<ID3D12CommandQueue*>(m_command_queue)->ExecuteCommandLists(1, cmd_lists);
    Wait_for_GPU();
    static_cast<ID3D12CommandAllocator*>(m_command_allocator)->Reset();
    cmd_list->Reset(static_cast<ID3D12CommandAllocator*>(m_command_allocator), nullptr);

    upload_buffer->Release();
}

/************************************************************************************************
 * DX12Backend::Draw_Primitive -- Draw non-indexed primitives                                   *
 ************************************************************************************************/
void DX12Backend::Draw_Primitive(unsigned int vertex_count, unsigned int start_vertex)
{
    if (!m_command_list) return;

    ID3D12GraphicsCommandList* cmd_list = static_cast<ID3D12GraphicsCommandList*>(m_command_list);

    if (m_state_dirty || !m_pipeline_state) {
        Rebuild_PSO_From_State();
    }
    if (m_pipeline_state) {
        cmd_list->SetPipelineState(reinterpret_cast<ID3D12PipelineState*>(m_pipeline_state));
    }

    // Set vertex buffer
    cmd_list->IASetVertexBuffers(0, 1, &m_vertex_buffer_view);

    // Set primitive topology (triangle list is most common for Renegade-style games)
    cmd_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Draw
    cmd_list->DrawInstanced(vertex_count, 1, start_vertex, 0);
}

/************************************************************************************************
 * DX12Backend::Draw_Indexed -- Draw indexed primitives                                          *
 ************************************************************************************************/
void DX12Backend::Draw_Indexed(unsigned int index_count, unsigned int start_index, unsigned int base_vertex)
{
    if (!m_command_list) return;

    ID3D12GraphicsCommandList* cmd_list = static_cast<ID3D12GraphicsCommandList*>(m_command_list);


    if (m_state_dirty || !m_pipeline_state) {
        Rebuild_PSO_From_State();
    }
    if (m_pipeline_state) {
        cmd_list->SetPipelineState(reinterpret_cast<ID3D12PipelineState*>(m_pipeline_state));
    }

    // Set index buffer
    cmd_list->IASetIndexBuffer(&m_index_buffer_view);

    // Set primitive topology
    cmd_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Draw indexed
    cmd_list->DrawIndexedInstanced(index_count, 1, start_index, base_vertex, 0);
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

    // Clear depth/stencil if requested and DSV heap exists
    if (clear_z_stencil && m_dsv_heap) {
        D3D12_CPU_DESCRIPTOR_HANDLE dsv = static_cast<ID3D12DescriptorHeap*>(m_dsv_heap)->GetCPUDescriptorHandleForHeapStart();
        cmd_list->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, z, static_cast<UINT8>(stencil), 0, nullptr);
    }
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
 * DX12Backend::Bind_Texture -- Bind a texture to a shader slot                                *
 ************************************************************************************************/
void DX12Backend::Bind_Texture(unsigned int slot, void* texture)
{
    if (slot >= 8) return; // Out of bounds

    // Check if texture is a BackendSurfaceHandle*
    if (texture != nullptr) {
        BackendSurfaceHandle* surface = static_cast<BackendSurfaceHandle*>(texture);
        if (surface->BackendData != nullptr) {
            // It's a backend surface handle - extract the resource
            ID3D12Resource* resource = static_cast<ID3D12Resource*>(surface->BackendData);

            // Create SRV for this resource
            D3D12_CPU_DESCRIPTOR_HANDLE srv_handle = {};
            srv_handle.ptr = 0;
            if (m_srv_heap) {
                D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {};
                static_cast<ID3D12DescriptorHeap*>(m_srv_heap)->GetDesc(&heap_desc);
                srv_handle = static_cast<ID3D12DescriptorHeap*>(m_srv_heap)->GetCPUDescriptorHandleForHeapStart();
                srv_handle.ptr += static_cast<size_t>(slot) * m_srv_descriptor_size;

                D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
                srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                srv_desc.Texture2D.MipLevels = 1;
                srv_desc.Texture2D.MostDetailedMip = 0;
                srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

                static_cast<ID3D12Device*>(m_device)->CreateShaderResourceView(resource, &srv_desc, srv_handle);

                // Store in m_textures slot
                m_textures[slot].resource = resource;
                m_textures[slot].width = 0; // Unknown from surface handle
                m_textures[slot].height = 0;
                m_textures[slot].stride = 0;
            }
        } else {
            // Raw pixel data pointer - need dimensions to create texture
            // Caller should use Create_Texture_From_Data directly with dimensions
            return;
        }
    }

    m_bound_textures[slot] = texture;

    // Bind the SRV to the graphics pipeline
    if (m_srv_heap && m_command_list) {
        D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle = {};
        gpu_handle.ptr = 0;
        if (m_srv_heap) {
            D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {};
            static_cast<ID3D12DescriptorHeap*>(m_srv_heap)->GetDesc(&heap_desc);
            gpu_handle = static_cast<ID3D12DescriptorHeap*>(m_srv_heap)->GetGPUDescriptorHandleForHeapStart();
            gpu_handle.ptr += static_cast<size_t>(slot) * m_srv_descriptor_size;

            static_cast<ID3D12GraphicsCommandList*>(m_command_list)->SetGraphicsRootDescriptorTable(slot, gpu_handle);
        }
    }
}

/************************************************************************************************
 * DX12Backend::Create_Texture_From_Data -- Create and upload a texture from raw pixel data    *
 ************************************************************************************************/
bool DX12Backend::Create_Texture_From_Data(void* data, unsigned int width, unsigned int height, unsigned int stride, unsigned int slot)
{
    if (slot >= 8 || !m_device || !data) return false;

    // Release existing texture in this slot if present
    if (m_textures[slot].resource) {
        SafeRelease(reinterpret_cast<ID3D12Resource**>(&m_textures[slot].resource));
        m_textures[slot].resource = nullptr;
    }

    ID3D12Device* device = static_cast<ID3D12Device*>(m_device);

    // Describe the texture resource
    D3D12_RESOURCE_DESC tex_desc = {};
    tex_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    tex_desc.Width = width;
    tex_desc.Height = height;
    tex_desc.DepthOrArraySize = 1;
    tex_desc.MipLevels = 1;
    tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    tex_desc.SampleDesc.Count = 1;
    tex_desc.SampleDesc.Quality = 0;
    tex_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    tex_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

    // Create the default heap texture
    D3D12_HEAP_PROPERTIES heap_props = {};
    heap_props.Type = D3D12_HEAP_TYPE_DEFAULT;
    heap_props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heap_props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heap_props.VisibleNodeMask = 0;
    heap_props.CreationNodeMask = 0;

    ID3D12Resource* texture = nullptr;
    HRESULT hr = device->CreateCommittedResource(
        &heap_props,
        D3D12_HEAP_FLAG_NONE,
        &tex_desc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&texture));
    if (FAILED(hr)) {
        WWDEBUG_SAY(("DX12: CreateCommittedResource (texture) failed: %x\n", hr));
        return false;
    }

    // Calculate subresource layout
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout = {};
    unsigned int num_rows = 0;
    unsigned long long row_size_bytes = 0;
    device->GetCopyableFootprints(&tex_desc, 0, 1, 0, &layout, &num_rows, &row_size_bytes, nullptr);

    // Create upload heap buffer for staging
    D3D12_HEAP_PROPERTIES upload_heap_props = {};
    upload_heap_props.Type = D3D12_HEAP_TYPE_UPLOAD;
    upload_heap_props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    upload_heap_props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    upload_heap_props.VisibleNodeMask = 0;
    upload_heap_props.CreationNodeMask = 0;

    D3D12_RESOURCE_DESC upload_desc = {};
    upload_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    upload_desc.Width = layout.Footprint.RowPitch * num_rows;
    upload_desc.Height = 1;
    upload_desc.DepthOrArraySize = 1;
    upload_desc.MipLevels = 1;
    upload_desc.Format = DXGI_FORMAT_UNKNOWN;
    upload_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    upload_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ID3D12Resource* upload_buffer = nullptr;
    hr = device->CreateCommittedResource(
        &upload_heap_props,
        D3D12_HEAP_FLAG_NONE,
        &upload_desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&upload_buffer));
    if (FAILED(hr)) {
        WWDEBUG_SAY(("DX12: CreateCommittedResource (upload) failed: %x\n", hr));
        texture->Release();
        return false;
    }

    // Copy data to upload buffer
    void* mapped_data = nullptr;
    upload_buffer->Map(0, nullptr, &mapped_data);
    if (mapped_data) {
        // Copy row by row to handle stride differences
        unsigned char* src = static_cast<unsigned char*>(data);
        unsigned char* dst = static_cast<unsigned char*>(mapped_data);
        for (unsigned int row = 0; row < height; row++) {
            memcpy(dst + row * layout.Footprint.RowPitch, src + row * stride, width * 4);
        }
        upload_buffer->Unmap(0, nullptr);
    }

    // Reset command list for the copy operation
    if (m_command_allocator && m_command_list) {
        static_cast<ID3D12CommandAllocator*>(m_command_allocator)->Reset();
        static_cast<ID3D12GraphicsCommandList*>(m_command_list)->Reset(
            static_cast<ID3D12CommandAllocator*>(m_command_allocator), nullptr);

        // Copy from upload buffer to texture
        D3D12_TEXTURE_COPY_LOCATION src_location = {};
        src_location.pResource = upload_buffer;
        src_location.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        src_location.PlacedFootprint = layout;

        D3D12_TEXTURE_COPY_LOCATION dst_location = {};
        dst_location.pResource = texture;
        dst_location.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dst_location.SubresourceIndex = 0;

        static_cast<ID3D12GraphicsCommandList*>(m_command_list)->CopyTextureRegion(&dst_location, 0, 0, 0, &src_location, nullptr);

        // Transition texture to shader read state
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = texture;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        static_cast<ID3D12GraphicsCommandList*>(m_command_list)->ResourceBarrier(1, &barrier);

        // Close and execute command list
        static_cast<ID3D12GraphicsCommandList*>(m_command_list)->Close();
        ID3D12CommandList* cmd_lists[] = { static_cast<ID3D12GraphicsCommandList*>(m_command_list) };
        static_cast<ID3D12CommandQueue*>(m_command_queue)->ExecuteCommandLists(1, cmd_lists);

        // Wait for GPU to finish the copy
        Wait_for_GPU();
    }

    // Release the upload buffer
    SafeRelease(reinterpret_cast<ID3D12Resource**>(&upload_buffer));

    // Create SRV for the texture
    if (m_srv_heap) {
        D3D12_CPU_DESCRIPTOR_HANDLE srv_handle = static_cast<ID3D12DescriptorHeap*>(m_srv_heap)->GetCPUDescriptorHandleForHeapStart();
        srv_handle.ptr += static_cast<size_t>(slot) * m_srv_descriptor_size;

        D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
        srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srv_desc.Texture2D.MipLevels = 1;
        srv_desc.Texture2D.MostDetailedMip = 0;
        srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        device->CreateShaderResourceView(texture, &srv_desc, srv_handle);
    }

    // Store the texture in our slot
    m_textures[slot].resource = texture;
    m_textures[slot].width = width;
    m_textures[slot].height = height;
    m_textures[slot].stride = stride;
    m_bound_textures[slot] = texture;

    return true;
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
        if (m_default_render_target && m_rtv_heap) {
            D3D12_CPU_DESCRIPTOR_HANDLE rtv = static_cast<ID3D12DescriptorHeap*>(m_rtv_heap)->GetCPUDescriptorHandleForHeapStart();
            D3D12_CPU_DESCRIPTOR_HANDLE dsv = static_cast<ID3D12DescriptorHeap*>(m_dsv_heap)->GetCPUDescriptorHandleForHeapStart();
            static_cast<ID3D12GraphicsCommandList*>(m_command_list)->OMSetRenderTargets(1, &rtv, FALSE, &dsv);
        }
        return;
    }

    // target is a BackendSurfaceHandle* - extract the DX12 resource
    BackendSurfaceHandle* surface = static_cast<BackendSurfaceHandle*>(target);
    if (surface->BackendData) {
        m_render_target = surface->BackendData;
        // Create an RTV for this resource and bind it
        // For now, use the first slot in the RTV heap
        if (m_rtv_heap) {
            D3D12_CPU_DESCRIPTOR_HANDLE rtv = static_cast<ID3D12DescriptorHeap*>(m_rtv_heap)->GetCPUDescriptorHandleForHeapStart();
            static_cast<ID3D12Device*>(m_device)->CreateRenderTargetView(
                static_cast<ID3D12Resource*>(m_render_target), nullptr, rtv);
            static_cast<ID3D12GraphicsCommandList*>(m_command_list)->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
        }
    }
}

/************************************************************************************************
 * DX12Backend::Set_DX8_Render_State -- Translate DX8 render state to DX12 PSO                *
 ************************************************************************************************/
void DX12Backend::Set_DX8_Render_State(int state, unsigned value)
{
    switch (state) {
        case D3DRS_FILLMODE:
            m_dx8_fill_mode = value;
            m_state_dirty = true;
            break;
        case D3DRS_CULLMODE:
            m_dx8_cull_mode = value;
            m_state_dirty = true;
            break;
        case D3DRS_ZENABLE:
            m_dx8_zenable = value;
            m_state_dirty = true;
            break;
        case D3DRS_ZWRITEENABLE:
            // Depth write enable tracked for future PSO expansion
            m_state_dirty = true;
            break;
        case D3DRS_ALPHABLENDENABLE:
            // Alpha blend enable tracked for future PSO expansion
            m_state_dirty = true;
            break;
        default:
            // Ignore unimplemented states
            break;
    }

    if (m_state_dirty) {
        Rebuild_PSO_From_State();
    }
}

/************************************************************************************************
 * DX12Backend::Set_Lighting -- Enable or disable lighting                                      *
 ************************************************************************************************/
void DX12Backend::Set_Lighting(bool enable)
{
    m_lighting_enabled = enable;
    m_dirty_matrix = true;  // lighting affects vertex processing
    Rebuild_PSO_From_State();
}

/************************************************************************************************
 * DX12Backend::Set_Fog -- Enable or disable fog                                                *
 ************************************************************************************************/
void DX12Backend::Set_Fog(bool enable)
{
    m_fog_enabled = enable;
    m_dirty_matrix = true;  // fog is part of pixel shader
    Rebuild_PSO_From_State();
}

/************************************************************************************************
 * DX12Backend::Set_Light_Environment -- Store light environment (stub)                         *
 ************************************************************************************************/
void DX12Backend::Set_Light_Environment(const void* /*env*/)
{
    // DX12 equivalent - deferred until pipeline state is implemented
}

/************************************************************************************************
 * DX12Backend::Create_Staging_Texture -- Create a CPU-readable staging texture for readback    *
 ************************************************************************************************/
bool DX12Backend::Create_Staging_Texture(int width, int height)
{
    if (!m_device) return false;

    // Release old staging texture if dimensions differ
    if (m_staging_texture && (m_staging_width != width || m_staging_height != height)) {
        SafeRelease(reinterpret_cast<ID3D12Resource**>(&m_staging_texture));
        m_staging_texture = nullptr;
        m_staging_width = 0;
        m_staging_height = 0;
    }

    // Create new staging texture if needed
    if (!m_staging_texture) {
        D3D12_RESOURCE_DESC tex_desc = {};
        tex_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        tex_desc.Width = width;
        tex_desc.Height = height;
        tex_desc.DepthOrArraySize = 1;
        tex_desc.MipLevels = 1;
        tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        tex_desc.SampleDesc.Count = 1;
        tex_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        tex_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

        D3D12_HEAP_PROPERTIES heap_props = {};
        heap_props.Type = D3D12_HEAP_TYPE_READBACK;
        heap_props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heap_props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

        ID3D12Resource* staging = nullptr;
        HRESULT hr = static_cast<ID3D12Device*>(m_device)->CreateCommittedResource(
            &heap_props,
            D3D12_HEAP_FLAG_NONE,
            &tex_desc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&staging));
        if (FAILED(hr)) {
            WWDEBUG_SAY(("DX12: CreateCommittedResource (staging) failed: %x\n", hr));
            return false;
        }
        m_staging_texture = staging;
        m_staging_width = width;
        m_staging_height = height;
    }
    return true;
}

/************************************************************************************************
 * DX12Backend::Copy_To_Staging -- Copy the back buffer to the staging texture                  *
 ************************************************************************************************/
bool DX12Backend::Copy_To_Staging(int width, int height)
{
    if (!m_device || !m_swap_chain || !m_command_list) return false;

    // Ensure staging texture exists with correct size
    if (!Create_Staging_Texture(width, height)) {
        return false;
    }

    // Get current back buffer
    ID3D12Resource* back_buffer = nullptr;
    unsigned int idx = static_cast<IDXGISwapChain*>(m_swap_chain)->GetCurrentBackBufferIndex();
    HRESULT hr = static_cast<IDXGISwapChain*>(m_swap_chain)->GetBuffer(idx, IID_PPV_ARGS(&back_buffer));
    if (FAILED(hr)) {
        WWDEBUG_SAY(("DX12: GetBuffer for staging copy failed: %x\n", hr));
        return false;
    }

    // Transition back buffer from RENDER_TARGET to COPY_SOURCE
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = back_buffer;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    static_cast<ID3D12GraphicsCommandList*>(m_command_list)->ResourceBarrier(1, &barrier);

    // Copy back buffer to staging texture
    D3D12_TEXTURE_COPY_LOCATION src = {};
    src.pResource = back_buffer;
    src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    src.SubresourceIndex = 0;

    D3D12_TEXTURE_COPY_LOCATION dst = {};
    dst.pResource = static_cast<ID3D12Resource*>(m_staging_texture);
    dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dst.SubresourceIndex = 0;

    static_cast<ID3D12GraphicsCommandList*>(m_command_list)->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

    // Transition back buffer back to RENDER_TARGET
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    static_cast<ID3D12GraphicsCommandList*>(m_command_list)->ResourceBarrier(1, &barrier);

    // Execute command list to complete the copy
    static_cast<ID3D12GraphicsCommandList*>(m_command_list)->Close();
    ID3D12CommandList* cmd_lists[] = { static_cast<ID3D12GraphicsCommandList*>(m_command_list) };
    static_cast<ID3D12CommandQueue*>(m_command_queue)->ExecuteCommandLists(1, cmd_lists);

    // Wait for GPU to finish
    Wait_for_GPU();

    // Reset command list for next frame
    static_cast<ID3D12CommandAllocator*>(m_command_allocator)->Reset();
    static_cast<ID3D12GraphicsCommandList*>(m_command_list)->Reset(
        static_cast<ID3D12CommandAllocator*>(m_command_allocator), nullptr);

    back_buffer->Release();
    return true;
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

    // Copy back buffer to staging texture for CPU readback
    Copy_To_Staging(m_width, m_height);

    out_handle->BackendData = m_staging_texture;
    out_handle->D3DSurface = nullptr;
}

/************************************************************************************************
 * DX12Backend::Lock_Front_Buffer_Surface -- Lock the front buffer for reading                 *
 ************************************************************************************************/
void DX12Backend::Lock_Front_Buffer_Surface(BackendSurfaceHandle* /*handle*/, int width, int /*height*/, SurfaceLockData* out_data)
{
    if (!out_data) return;
    out_data->Valid = false;
    out_data->PixelData = nullptr;
    out_data->RowPitch = 0;

    if (!m_staging_texture) return;

    D3D12_RANGE readRange{0, 0}; // Map the whole resource
    void* pData = nullptr;
    HRESULT hr = static_cast<ID3D12Resource*>(m_staging_texture)->Map(0, &readRange, &pData);
    if (FAILED(hr)) return;

    out_data->PixelData = pData;
    out_data->RowPitch = width * 4; // RGBA
    out_data->Valid = true;
}

/************************************************************************************************
 * DX12Backend::Unlock_Front_Buffer_Surface -- Unlock the front buffer                          *
 ************************************************************************************************/
void DX12Backend::Unlock_Front_Buffer_Surface(BackendSurfaceHandle* /*handle*/)
{
    if (m_staging_texture) {
        static_cast<ID3D12Resource*>(m_staging_texture)->Unmap(0, nullptr);
    }
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
        Create_DepthStencil();
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

/************************************************************************************************
 * DX12Backend::Set_World_Matrix -- Set world transform matrix                                  *
 ************************************************************************************************/
void DX12Backend::Set_World_Matrix(const float* matrix4x4)
{
    if (matrix4x4) {
        memcpy(m_world_matrix, matrix4x4, sizeof(m_world_matrix));
        m_dirty_matrix = true;
    }
}

/************************************************************************************************
 * DX12Backend::Set_View_Matrix -- Set view transform matrix                                  *
 ************************************************************************************************/
void DX12Backend::Set_View_Matrix(const float* matrix4x4)
{
    if (matrix4x4) {
        memcpy(m_view_matrix, matrix4x4, sizeof(m_view_matrix));
        m_dirty_matrix = true;
    }
}

/************************************************************************************************
 * DX12Backend::Set_Projection_Matrix -- Set projection transform matrix                       *
 ************************************************************************************************/
void DX12Backend::Set_Projection_Matrix(const float* matrix4x4)
{
    if (matrix4x4) {
        memcpy(m_projection_matrix, matrix4x4, sizeof(m_projection_matrix));
        m_dirty_matrix = true;
    }
}

/************************************************************************************************
 * DX12Backend::Apply_Matrices -- Create root signature if needed and set matrix constants     *
 ************************************************************************************************/
void DX12Backend::Apply_Matrices()
{
    if (!m_dirty_matrix) return;
    if (!m_device || !m_command_list) return;

    // Create root signature once if not already created
    if (!m_root_signature) {
        // Root signature with 3 constant ranges (world, view, projection)
        // Each matrix is 16 floats = 48 D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS total
        D3D12_ROOT_PARAMETER root_params[3] = {};

        // World matrix at root slot 0
        root_params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        root_params[0].Constants.ShaderRegister = 0;
        root_params[0].Constants.RegisterSpace = 0;
        root_params[0].Constants.Num32BitValues = 16;
        root_params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        // View matrix at root slot 1
        root_params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        root_params[1].Constants.ShaderRegister = 1;
        root_params[1].Constants.RegisterSpace = 0;
        root_params[1].Constants.Num32BitValues = 16;
        root_params[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        // Projection matrix at root slot 2
        root_params[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        root_params[2].Constants.ShaderRegister = 2;
        root_params[2].Constants.RegisterSpace = 0;
        root_params[2].Constants.Num32BitValues = 16;
        root_params[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        D3D12_ROOT_SIGNATURE_DESC rs_desc = {};
        rs_desc.NumParameters = 3;
        rs_desc.pParameters = root_params;
        rs_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

        ID3DBlob* blob = nullptr;
        ID3DBlob* error = nullptr;
        HRESULT hr = D3D12SerializeRootSignature(&rs_desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error);
        if (FAILED(hr)) {
            WWDEBUG_SAY(("DX12: D3D12SerializeRootSignature failed: %x\n", hr));
            if (error) error->Release();
            return;
        }

        hr = static_cast<ID3D12Device*>(m_device)->CreateRootSignature(
            0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(reinterpret_cast<ID3D12RootSignature**>(&m_root_signature)));
        blob->Release();
        if (FAILED(hr)) {
            WWDEBUG_SAY(("DX12: CreateRootSignature failed: %x\n", hr));
            return;
        }

        // Rebuild PSO with the new root signature
        Rebuild_PSO_From_State();
    }

    // Set matrix constants
    ID3D12GraphicsCommandList* cmd_list = static_cast<ID3D12GraphicsCommandList*>(m_command_list);
    cmd_list->SetGraphicsRoot32BitConstants(0, 16, m_world_matrix, 0);
    cmd_list->SetGraphicsRoot32BitConstants(1, 16, m_view_matrix, 0);
    cmd_list->SetGraphicsRoot32BitConstants(2, 16, m_projection_matrix, 0);

    m_dirty_matrix = false;
}

/************************************************************************************************
 * DX12Backend::DX8_Set_Transform -- Set transform matrix by type                             *
 ************************************************************************************************/
void DX12Backend::DX8_Set_Transform(int type, const float* matrix4x4)
{
    switch (type) {
        case 0:  // D3DTS_WORLD
            Set_World_Matrix(matrix4x4);
            break;
        case 1:  // D3DTS_VIEW
            Set_View_Matrix(matrix4x4);
            break;
        case 2:  // D3DTS_PROJECTION
            Set_Projection_Matrix(matrix4x4);
            break;
        default:
            break;
    }
    Apply_Matrices();
}

/************************************************************************************************
 * DX12Backend::DX8_Set_World_Identity -- Set world matrix to identity                        *
 ************************************************************************************************/
void DX12Backend::DX8_Set_World_Identity()
{
    const float identity[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    Set_World_Matrix(identity);
    Apply_Matrices();
}

/************************************************************************************************
 * DX12Backend::DX8_Set_View_Identity -- Set view matrix to identity                         *
 ************************************************************************************************/
void DX12Backend::DX8_Set_View_Identity()
{
    const float identity[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    Set_View_Matrix(identity);
    Apply_Matrices();
}

/************************************************************************************************
 * DX12Backend::DX8_Set_Vertex_Buffer -- Create and upload vertex buffer                       *
 ************************************************************************************************/
void DX12Backend::DX8_Set_Vertex_Buffer(unsigned int /*buffer_slot*/, void* vertex_data, unsigned int vertex_count, unsigned int stride)
{
    if (!vertex_data || vertex_count == 0 || stride == 0) return;
    Set_Vertex_Buffer(vertex_data, stride, vertex_count);
}

/************************************************************************************************
 * DX12Backend::DX8_Set_Index_Buffer -- Create and upload index buffer                          *
 ************************************************************************************************/
void DX12Backend::DX8_Set_Index_Buffer(void* index_data, unsigned int index_count)
{
    if (!index_data || index_count == 0) return;
    Set_Index_Buffer(index_data, index_count);
}

/************************************************************************************************
 * DX12Backend::DX8_Draw_Triangles -- Draw non-indexed triangles                               *
 ************************************************************************************************/
void DX12Backend::DX8_Draw_Triangles(unsigned int start_vertex, unsigned int vertex_count, unsigned int /*start_index*/)
{
    Draw_Primitive(vertex_count, start_vertex);
}

/************************************************************************************************
 * DX12Backend::DX8_Draw_Indexed -- Draw indexed primitives                                      *
 ************************************************************************************************/
void DX12Backend::DX8_Draw_Indexed(unsigned int index_count, unsigned int start_index, unsigned int base_vertex)
{
    Draw_Indexed(index_count, start_index, base_vertex);
}

/************************************************************************************************
 * DX12Backend::DX8_Set_Texture -- Bind texture to a stage                                      *
 ************************************************************************************************/
void DX12Backend::DX8_Set_Texture(unsigned int stage, void* texture_data)
{
    if (stage >= 8) return;
    // If non-null, bind the texture; if null, unbind
    Bind_Texture(stage, texture_data);
}

/************************************************************************************************
 * DX12Backend::DX8_Set_Material -- Apply material colors via render state                     *
 ************************************************************************************************/
void DX12Backend::DX8_Set_Material(const void* material)
{
    if (!material) return;
    // D3DMATERIAL9 structure has: Diffuse, Ambient, Specular, Emissive, Power
    // We'll extract diffuse and emissive colors
    // For now, just use the stored shader
    // Material application would go through Set_DX8_Render_State calls
    (void)material;
}

/************************************************************************************************
 * DX12Backend::DX8_Set_Shader -- Store shader pointer for later use                            *
 ************************************************************************************************/
void DX12Backend::DX8_Set_Shader(void* shader)
{
    m_stored_shader = shader;
}

/************************************************************************************************
 * DX12Backend::DX8_Set_Viewport -- Set viewport (delegates to existing method)                *
 ************************************************************************************************/
void DX12Backend::DX8_Set_Viewport(const void* viewport)
{
    Set_Viewport(viewport);
}

/************************************************************************************************
 * DX12Backend::DX8_Convert_Color -- Convert ARGB color to DX12 format                          *
 ************************************************************************************************/
unsigned int DX12Backend::DX8_Convert_Color(unsigned int argb, float opacity)
{
    // Extract components from ARGB
    unsigned int a = (argb >> 24) & 0xFF;
    unsigned int r = (argb >> 16) & 0xFF;
    unsigned int g = (argb >> 8) & 0xFF;
    unsigned int b = argb & 0xFF;
    // Apply opacity
    if (opacity < 1.0f) {
        a = static_cast<unsigned int>(a * opacity);
    }
    // Return as ARGB
    return (a << 24) | (r << 16) | (g << 8) | b;
}

/************************************************************************************************
 * DX12Backend::Create_DepthStencil -- Create depth stencil buffer and DSV                        *
 ************************************************************************************************/
bool DX12Backend::Create_DepthStencil()
{
    if (!m_device) return false;

    // Release existing depth stencil if present
    SafeRelease(reinterpret_cast<ID3D12Resource**>(&m_depthStencil));

    // Depth stencil texture desc - match swap chain dimensions
    D3D12_RESOURCE_DESC depth_desc = {};
    depth_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depth_desc.Width = m_width;
    depth_desc.Height = m_height;
    depth_desc.DepthOrArraySize = 1;
    depth_desc.MipLevels = 1;
    depth_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depth_desc.SampleDesc.Count = 1;
    depth_desc.SampleDesc.Quality = 0;
    depth_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE depth_clear_value = {};
    depth_clear_value.Format = depth_desc.Format;
    depth_clear_value.DepthStencil.Depth = 1.0f;
    depth_clear_value.DepthStencil.Stencil = 0;

    ID3D12Resource* depth_resource = nullptr;
    HRESULT hr = static_cast<ID3D12Device*>(m_device)->CreateCommittedResource(
        &D3D12_HEAP_PROPERTIES{D3D12_HEAP_TYPE_DEFAULT, D3D12_MEMORY_POOL_UNKNOWN, 0, 0},
        D3D12_HEAP_FLAG_NONE,
        &depth_desc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &depth_clear_value,
        IID_PPV_ARGS(&depth_resource));
    if (FAILED(hr)) {
        WWDEBUG_SAY(("DX12: CreateCommittedResource (depth) failed: %x\n", hr));
        return false;
    }
    m_depthStencil = depth_resource;

    // Create DSV descriptor in the DSV heap (first slot at offset 0)
    D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc = {};
    dsv_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsv_desc.Texture2D.MipSlice = 0;

    D3D12_CPU_DESCRIPTOR_HANDLE dsv_handle = static_cast<ID3D12DescriptorHeap*>(m_dsv_heap)->GetCPUDescriptorHandleForHeapStart();
    static_cast<ID3D12Device*>(m_device)->CreateDepthStencilView(m_depthStencil, &dsv_desc, dsv_handle);

    return true;
}

#else // !_WIN32

// DX12 is Windows-only - provide stubs for non-Windows builds

// DX8 render state constants (stub values for non-Windows compilation)
#ifndef D3DFILL_SOLID
#define D3DFILL_SOLID 3
#endif
#ifndef D3DFILL_WIREFRAME
#define D3DFILL_WIREFRAME 2
#endif
#ifndef D3DFILL_POINT
#define D3DFILL_POINT 1
#endif
#ifndef D3DCULL_NONE
#define D3DCULL_NONE 1
#endif
#ifndef D3DCULL_FRONT
#define D3DCULL_FRONT 2
#endif
#ifndef D3DCULL_BACK
#define D3DCULL_BACK 3
#endif
#ifndef D3DZB_TRUE
#define D3DZB_TRUE 1
#endif
#ifndef D3DZB_FALSE
#define D3DZB_FALSE 0
#endif
#ifndef D3DRS_FILLMODE
#define D3DRS_FILLMODE 8
#endif
#ifndef D3DRS_CULLMODE
#define D3DRS_CULLMODE 3
#endif
#ifndef D3DRS_ZENABLE
#define D3DRS_ZENABLE 7
#endif
#ifndef D3DRS_ZWRITEENABLE
#define D3DRS_ZWRITEENABLE 8
#endif
#ifndef D3DRS_ALPHABLENDENABLE
#define D3DRS_ALPHABLENDENABLE 19
#endif

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
    m_srv_heap(nullptr),
    m_dsv_heap(nullptr),
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
    m_default_render_target(nullptr),
    m_rtv_descriptor_size(0),
    m_srv_descriptor_size(0),
    m_dsv_descriptor_size(0),
    m_fence_value(0),
    m_fence_event(nullptr),
    m_pipeline_state(nullptr),
    m_staging_texture(nullptr),
    m_staging_width(0),
    m_staging_height(0),
    m_dx8_fill_mode(D3DFILL_SOLID),
    m_dx8_cull_mode(D3DCULL_NONE),
    m_dx8_zenable(D3DZB_TRUE),
    m_dx8_fill_solid(1),
    m_state_dirty(false),
    m_root_signature(nullptr),
    m_dirty_matrix(false),
    m_lighting_enabled(true),
    m_fog_enabled(false),
    m_vs_blob(nullptr),
    m_ps_blob(nullptr),
    m_vertex_buffer(nullptr),
    m_index_buffer(nullptr),
    m_vertex_buffer_stride(0),
    m_vertex_buffer_offset(0),
    m_vertex_count(0),
    m_index_count(0),
    m_stored_shader(nullptr)
{
    // Identity matrices
    const float identity[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    memcpy(m_world_matrix, identity, sizeof(m_world_matrix));
    memcpy(m_view_matrix, identity, sizeof(m_view_matrix));
    memcpy(m_projection_matrix, identity, sizeof(m_projection_matrix));
    // Initialize bound textures array
    for (unsigned int i = 0; i < 8; i++) {
        m_bound_textures[i] = nullptr;
        m_textures[i].resource = nullptr;
        m_textures[i].width = 0;
        m_textures[i].height = 0;
        m_textures[i].stride = 0;
    }
}

DX12Backend::~DX12Backend() { Shutdown(); }
void DX12Backend::Shutdown() {
    for (unsigned int i = 0; i < 8; i++) {
        m_textures[i].resource = nullptr;
        m_textures[i].width = 0;
        m_textures[i].height = 0;
        m_textures[i].stride = 0;
    }
    m_pipeline_state = nullptr;
    m_staging_texture = nullptr;
    m_root_signature = nullptr;
    m_vs_blob = nullptr;
    m_ps_blob = nullptr;
    m_initialized = false;
}
bool DX12Backend::Create_Descriptor_Heaps() { return false; }
bool DX12Backend::Create_Default_Shaders() { return false; }
bool DX12Backend::Create_Default_PSO() { return false; }
void DX12Backend::Set_Lighting(bool) {}
void DX12Backend::Set_Fog(bool) {}
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
void DX12Backend::Bind_Texture(unsigned int, void*) {}
void DX12Backend::Set_Viewport(const void*) {}
void DX12Backend::Set_Render_Target(void*) {}
void DX12Backend::Set_DX8_Render_State(int, unsigned) {}
void DX12Backend::Set_Light_Environment(const void*) {}
void DX12Backend::Get_Front_Buffer_Surface(BackendSurfaceHandle* h) { if (h) { h->D3DSurface = nullptr; h->BackendData = nullptr; } }
void DX12Backend::Lock_Front_Buffer_Surface(BackendSurfaceHandle*, int, int, SurfaceLockData* d) { if (d) d->Valid = false; }
void DX12Backend::Unlock_Front_Buffer_Surface(BackendSurfaceHandle*) {}
bool DX12Backend::Create_Staging_Texture(int, int) { return false; }
bool DX12Backend::Copy_To_Staging(int, int) { return false; }
bool DX12Backend::Create_Texture_From_Data(void*, unsigned int, unsigned int, unsigned int, unsigned int) { return false; }
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
void DX12Backend::Set_World_Matrix(const float*) {}
void DX12Backend::Set_View_Matrix(const float*) {}
void DX12Backend::Set_Projection_Matrix(const float*) {}
void DX12Backend::Apply_Matrices() {}
bool DX12Backend::Create_Vertex_Buffer(unsigned int) { return false; }
bool DX12Backend::Create_Index_Buffer(unsigned int) { return false; }
void DX12Backend::Set_Vertex_Buffer(void*, unsigned int, unsigned int) {}
void DX12Backend::Set_Index_Buffer(void*, unsigned int) {}
void DX12Backend::Draw_Primitive(unsigned int, unsigned int) {}
void DX12Backend::Draw_Indexed(unsigned int, unsigned int, unsigned int) {}

// DX8-style interface stubs for non-Windows builds
void DX12Backend::DX8_Set_Transform(int, const float*) {}
void DX12Backend::DX8_Set_World_Identity() {}
void DX12Backend::DX8_Set_View_Identity() {}
void DX12Backend::DX8_Set_Vertex_Buffer(unsigned int, void*, unsigned int, unsigned int) {}
void DX12Backend::DX8_Set_Index_Buffer(void*, unsigned int) {}
void DX12Backend::DX8_Draw_Triangles(unsigned int, unsigned int, unsigned int) {}
void DX12Backend::DX8_Draw_Indexed(unsigned int, unsigned int, unsigned int) {}
void DX12Backend::DX8_Set_Texture(unsigned int, void*) {}
void DX12Backend::DX8_Set_Material(const void*) {}
void DX12Backend::DX8_Set_Shader(void*) {}
void DX12Backend::DX8_Set_Viewport(const void*) {}
unsigned int DX12Backend::DX8_Convert_Color(unsigned int argb, float opacity) { (void)opacity; return argb; }

#endif // !_WIN32
