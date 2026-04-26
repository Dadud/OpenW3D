# OpenW3D Renderer Backend Development Plan
### Status: Core Phases Complete — Further Work Deferred

---

## Completed Phases

### Phase 1 — Render Device Abstraction ✅
| PR | Title | Status |
|----|-------|--------|
| R1 | `ww3dbackend.h: add RenderDeviceCapabilities struct` | ✅ Done |
| R2 | `WW3DBackend: add Render_Device enumeration API` | ✅ Done |
| R3 | `ww3d.cpp: route device enumeration through backend` | ✅ Done |

**Result:** `develop-rebased` commit `b29f62b5`

---

### Phase 2 — Swapchain & Presentation Abstraction ✅
| PR | Title | Status |
|----|-------|--------|
| S1 | `ww3dbackend.h: add PresentationDescriptor struct` | ✅ Done |
| S2 | `WW3DBackend: add Create_Swapchain(), Present() methods` | ✅ Done |
| S3 | `ww3d.cpp: route presentation through backend` | ✅ Done |
| S4 | `DX8Backend: implement swapchain via DX8` | ✅ Done |
| S5 | `NullBackend: stub swapchain` | ✅ Done |

**Result:** `develop-rebased` commit `7e7766b2`

---

### Phase 3 — Viewport & Render Target Abstraction ✅
| PR | Title | Status |
|----|-------|--------|
| V1 | `ww3dbackend.h: add ViewportDesc struct` | ✅ Done |
| V2 | `WW3DBackend: add Set_Render_Target(), Set_Viewport()` | ✅ Done |
| V3 | `DX8Backend: implement via DX8` | ✅ Done |
| V4 | `NullBackend: stub` | ✅ Done |

**Result:** `develop-rebased` commit `376e77bc`

---

### Phase 5 — Texture & Surface Abstraction (Partial) ✅ T1
| PR | Title | Status |
|----|-------|--------|
| T1 | `BackendSurfaceHandle` for screenshot/movie capture | ✅ Done |
| T2 | `Create_Texture()`, `Blt_Fast()` | ⏳ Deferred |
| T3 | Remove `IDirect3D*` from public headers | ⏳ Deferred |

**Result:** `develop-rebased` commit `9a5e4803`

---

## Deferred / Not Started

| Phase | Description | Blocking |
|-------|-------------|----------|
| Phase 4 | `RenderStateBlock` — D3D state translation | DX12/Vulkan need real implementations |
| Phase 5 T2-T3 | Texture creation, surface blit, header cleanup | Lower priority |
| Phase 6 | Shader/material abstraction (`BackendShaderHandle`, `Compile_Shader`) | Only needed for shader hot-reload |
| Phase 7 | Real DX12 backend | Blocked on Phases 4-6 |
| Phase 8 | Real Vulkan backend | Blocked on Phases 4-6 |

---

## What Was Achieved

The `develop-rebased` branch now has a clean `WW3DBackend` interface with:
- Device enumeration through backend (Phase 1)
- Swapchain/presentation abstraction (Phase 2)
- Viewport and render target routing (Phase 3)
- Backend-agnostic surface handles for screenshot/movie capture (Phase 5 T1)
- Community contribution copyright notices on all new files
- Linux build fixes for `_MAX_FNAME`, `_splitpath`, `HANDLE` typedef

**Key win:** `ww3d.cpp` no longer calls `DX8Wrapper` directly for device enumeration, viewport/render-target setup, or screenshot/movie capture. Call sites go through `Backend->`.

---

## Git Log (develop-rebased)

```
7e7766b2 fix(render Phase 2 S4): DX8Backend actually stores and manages swapchain
9a5e4803 feat(render Phase 5 T1): BackendSurfaceHandle abstraction for screenshot/movie capture
376e77bc feat(render Phase 3): add ViewportDesc and Set_Render_Target to WW3DBackend
eb78b2d9 feat(render): add Create_Swapchain to WW3DBackend interface (Phase 2 S1-S2)
941a8fa6 fix(headers): replace EA copyright with community contribution notice
f14b56ee fix(linux): add _lrotl alias and guard LaunchWeb.cpp on non-Windows
be071148 fix(linux): replace OutputDebugStringA #ifdef blocks with WWDEBUG_SAY
2639cf90 feat(linux): add Linux stubs for Win32 API types and exception handling
25200510 feat(render/vulkan): implement Vulkan backend with instance, device, surface, and swapchain
b2674966 fix(linux): guard Win32-only build paths
```

---

*Last updated: 2026-04-26*
