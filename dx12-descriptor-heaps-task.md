# DX12 Descriptor Heaps Task

## Context
You are implementing descriptor heaps (SRV/DSV/CBV) for the DX12 backend in ww3d2.

## Codebase
`/tmp/openw3d-analyze/Code/ww3d2/backends/dx12/`

## Current State
- `dx12backend.h` and `dx12backend.cpp` exist with a working DX12 backend skeleton
- All 37 WW3DBackend methods declared
- Device, swapchain, command queue, fences, Begin_Scene/End_Scene working
- Clear working (color only)

## Task: Add Descriptor Heap Infrastructure

### What to add to dx12backend.h:
1. New private members:
   - `void* m_srv_heap` — ID3D12DescriptorHeap for shader resource views (textures, constant buffers)
   - `void* m_dsv_heap` — ID3D12DescriptorHeap for depth/stencil views
   - `unsigned int m_srv_descriptor_size`
   - `unsigned int m_dsv_descriptor_size`
   - `bool Create_Descriptor_Heaps()` method declaration

2. Update Shutdown() to release these heaps

### What to add to dx12backend.cpp:
1. In Init() after swapchain creation, call `Create_Descriptor_Heaps()`
2. Implement `Create_Descriptor_Heaps()`:
   - Create SRV heap: `D3D12_DESCRIPTOR_HEAP_DESC{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 256, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE }`
   - Create DSV heap: `D3D12_DESCRIPTOR_HEAP_DESC{ D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 16, D3D12_DESCRIPTOR_HEAP_FLAG_NONE }`
   - Store descriptor sizes from `m_device->GetDescriptorHandleIncrementSize()`
3. In Shutdown(), release both heaps

### IMPORTANT RULES:
- Store all DX12 COM interfaces as `void*` in the header (no d3d12.h in header)
- Include d3d12.h and dxgi.h only in the .cpp under `#ifdef _WIN32`
- Use the same code style as existing methods in dx12backend.cpp
- All methods should compile cleanly under `#ifdef _WIN32`
- Add `#ifdef _WIN32` stubs for non-Windows builds

### After completing:
- Verify with: `cd /tmp/openw3d-analyze && /usr/bin/c++ -O3 -DNDEBUG -std=c++20 -DDIRECTINPUT_VERSION=0x800 -DNOMINMAX -DOPENW3D_SDL3=1 -DWEBBROWSER_ENABLED=0 -DWIN32_LEAN_AND_MEAN -DWW3D_DX12_BACKEND -D_CRT_NONSTDC_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -D_WIN32_WINNT=0x0601 -D__cdecl=\"\" -D__stdcall=\"\" -D_stdcall=\"\" -Dstricmp=strcasecmp -Dstrnicmp=strncasecmp -Dwcsicmp=wcscasecmp -Dwcsnicmp=wcsncasecmp -I/tmp/openw3d-analyze/Code/ww3d2 -I/tmp/openw3d-analyze/build-ci-linux/_deps/gamespy-src/include -I/tmp/openw3d-analyze/Code/dxvk_wrapper -I/usr/include/dxvk -I/tmp/openw3d-analyze/Code/wwlib -I/tmp/openw3d-analyze/Code/WWMath -I/tmp/openw3d-analyze/Code/ww3d2 -DWW3D_RDDESC_NO_D3D -fsyntax-only Code/ww3d2/backends/dx12/dx12backend.cpp`
- If compile fails, fix the errors and retry
- Once clean, write "DONE" to /tmp/openw3d-analyze/dx12-descriptor-heaps-status.txt with a brief summary of what was added
