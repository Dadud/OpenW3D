# OpenW3D Renderer Backend Development Plan
### Status: Phase 7 (DX12) In Progress

---

## Completed Phases

### Phase 1 — Render Device Abstraction ✅
| PR | Title | Status |
|----|-------|--------|
| R1 | `ww3dbackend.h: add RenderDeviceCapabilities struct` | ✅ Done |
| R2 | `WW3DBackend: add Render_Device enumeration API` | ✅ Done |
| R3 | `ww3d.cpp: route device enumeration through backend` | ✅ Done |

---

### Phase 2 — Swapchain & Presentation Abstraction ✅
| PR | Title | Status |
|----|-------|--------|
| S1-S5 | All swapchain/presentation methods | ✅ Done |

---

### Phase 3 — Viewport & Render Target Abstraction ✅
| PR | Title | Status |
|----|-------|--------|
| V1-V4 | ViewportDesc + Set_Render_Target | ✅ Done |

---

### Phase 5 — Texture & Surface Abstraction (Partial) ✅
| PR | Title | Status |
|----|-------|--------|
| T1 | BackendSurfaceHandle for screenshot/movie | ✅ Done |
| T2-T3 | Create_Texture, Blt_Fast, header cleanup | ⏳ Deferred |

---

## In Progress

### Phase 7 — DX12 Backend 🔄
| Component | Status | Notes |
|-----------|--------|-------|
| WW3DBackend interface (all 37 methods) | ✅ Done | |
| Device enumeration | ✅ Done | |
| Swap chain creation/resize | ✅ Done | |
| Command queue/allocator/list | ✅ Done | |
| Begin_Scene / End_Scene / Present | ✅ Done | |
| Clear (color) | ✅ Done | |
| Set_Viewport | ✅ Done | |
| Set_Render_Target | ✅ Done | |
| Set_DX8_Render_State | ✅ Stub | DX8→DX12 state translation deferred |
| Pipeline State Objects (PSO) | ⏳ TODO | Needed before drawing |
| Descriptor heaps (SRV/DSV) | ⏳ TODO | Needed for textures and depth |
| Texture/Surface creation | ⏳ TODO | Create_Texture, Blt_Fast |
| Depth/stencil view | ⏳ TODO | Clear depth requires DSV |
| Screenshot/Movie capture | ⏳ TODO | Lock_Front_Buffer needs staging resource |

---

### Phase 8 — Vulkan Backend ⏳
Not started.

---

## Deferred / Not Started

| Phase | Description |
|-------|-------------|
| Phase 4 | `RenderStateBlock` — DX8→DX12/Vulkan state translation |
| Phase 5 T2-T3 | Texture creation, surface blit, D3D header removal |
| Phase 6 | Shader/material abstraction (`BackendShaderHandle`, `Compile_Shader`) |

---

## Build Notes

**DX12 backend** (`ENABLE_DX12_BACKEND`):
- CMake option: `ENABLE_DX12_BACKEND=ON` (Windows only)
- Preprocessor: `WW3D_DX12_BACKEND`
- Requires Windows SDK (d3d12.h, dxgi.h)
- Committed to `develop-rebased` as `b3d26ddb`

**Vulkan backend** (`ENABLE_VULKAN_BACKEND`):
- CMake option: `ENABLE_VULKAN_BACKEND=ON` (all platforms)
- Preprocessor: `WW3D_VULKAN_BACKEND`
- Requires Vulkan SDK

---

## Git Log (develop-rebased, recent)

```
b3d26ddb feat(render): add DX12 backend skeleton (Phase 7 initial)
e177e70c fix(vulkanbackend): add missing virtual keyword to Create_Swapchain override
7feacfea docs: update plan - core phases complete, further work deferred
7e7766b2 fix(render Phase 2 S4): DX8Backend actually stores and manages swapchain
9a5e4803 feat(render Phase 5 T1): BackendSurfaceHandle abstraction
376e77bc feat(render Phase 3): add ViewportDesc and Set_Render_Target
eb78b2d9 feat(render): add Create_Swapchain... (Phase 2 S1-S2)
941a8fa6 fix(headers): replace EA copyright with community contribution notice
```

---

*Last updated: 2026-04-26*
