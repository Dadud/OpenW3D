# DX12 Texture Creation Task

## Context
You are implementing texture creation and surface operations for the DX12 backend in ww3d2.

## Codebase
`/tmp/openw3d-analyze/Code/ww3d2/backends/dx12/`

## Current State
- `dx12backend.h` and `dx12backend.cpp` exist with a working DX12 backend skeleton
- All 37 WW3DBackend methods declared
- Device, swapchain, command queue, fences, Begin_Scene/End_Scene working
- Descriptor heaps (SRV/DSV) are NOT yet added — work only with what exists

## Task: Add Create_Texture and Blt_Fast stubs + Improve Set_Render_Target

### What to add to dx12backend.h:
1. New private members for texture management:
   - `void* m_default_render_target` — ID3D12Resource for current render target
   - Track current render target state
2. Add method declarations:
   - `bool Create_Default_Render_Target()` — create the main back-buffer render target view
   - `void Bind_Texture(unsigned int slot, void* texture)` — bind a texture to a shader slot (stub)

### What to add to dx12backend.cpp:
1. In Init() after Create_Swap_Chain_Buffers(), call `Create_Default_Render_Target()`
2. Implement `Create_Default_Render_Target()`:
   - Get the current swap chain buffer
   - Create an RTV for it and store as m_default_render_target
   - This is used by Set_Render_Target(nullptr) to restore default
3. Improve `Set_Render_Target(void* target)`:
   - `nullptr` → bind to m_default_render_target (swap chain back buffer)
   - Non-null → target is a BackendSurfaceHandle*, extract DX12 resource from it
4. Add stub for `Bind_Texture`:
   - Just store the resource for now
   - Real implementation would descriptor UAV/SRV binding

### IMPORTANT RULES:
- Store all DX12 COM interfaces as `void*` in the header (no d3d12.h in header)
- Include d3d12.h and dxgi.h only in the .cpp under `#ifdef _WIN32`
- Use the same code style as existing methods in dx12backend.cpp
- All methods should compile cleanly under `#ifdef _WIN32`
- Add `#ifdef _WIN32` stubs for non-Windows builds

### After completing:
- Verify with: `cd /tmp/openw3d-analyze && /usr/bin/c++ -O3 -DNDEBUG -std=c++20 -DDIRECTINPUT_VERSION=0x800 -DNOMINMAX -DOPENW3D_SDL3=1 -DWEBBROWSER_ENABLED=0 -DWIN32_LEAN_AND_MEAN -DWW3D_DX12_BACKEND -D_CRT_NONSTDC_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -D_WIN32_WINNT=0x0601 -D__cdecl=\"\" -D__stdcall=\"\" -D_stdcall=\"\" -Dstricmp=strcasecmp -Dstrnicmp=strncasecmp -Dwcsicmp=wcscasecmp -Dwcsnicmp=wcsncasecmp -I/tmp/openw3d-analyze/Code/ww3d2 -I/tmp/openw3d-analyze/build-ci-linux/_deps/gamespy-src/include -I/tmp/openw3d-analyze/Code/dxvk_wrapper -I/usr/include/dxvk -I/tmp/openw3d-analyze/Code/wwlib -I/tmp/openw3d-analyze/Code/WWMath -I/tmp/openw3d-analyze/Code/ww3d2 -DWW3D_RDDESC_NO_D3D -fsyntax-only Code/ww3d2/backends/dx12/dx12backend.cpp`
- If compile fails, fix the errors and retry
- Once clean, write "DONE" to /tmp/openw3d-analyze/dx12-texture-status.txt with a brief summary of what was added
