# OpenW3D modernization architecture

## Product boundary

The product goal is **modern Renegade**: preserve the existing game, maps, missions, objects, weapons, vehicles, AI, and gameplay while replacing obsolete renderer/platform dependencies. The original retail executable and proprietary runtime DLLs are not dependencies. Retail content remains usable directly during migration; offline import/cooking is an optimization and portability path, not a prerequisite for a playable build.

## Runtime layers

```text
Modern/runtime
  application loop, lifecycle, fixed timestep
Modern/platform
  SDL3 windowing, input, filesystem, device discovery
Modern/render
  renderer-independent RHI; initially adapted to the existing WW3D/gameplay boundary
Modern/assets
  retail MIX/W3D compatibility first; optional cooked packages later
Modern/scene
  entities, transforms, cameras, render snapshots
Modern/gameplay
  simulation systems and action commands
Modern/audio
  portable mixer and spatial audio
Modern/net
  transport-neutral replication and session services
Modern/tools
  retail import, convert, cook, validate, package, hot reload
```

Install discovery rules are documented in `docs/RETAIL_INSTALL_DISCOVERY.md`: launcher metadata and explicit paths are preferred, historical defaults are hints, and content signatures decide whether a candidate is valid.

## Backend policy

- SDL3 is the platform boundary.
- The public renderer API must not expose D3D9, fixed-function state, COM, or platform window handles.
- Preferred graphics target is an explicit modern RHI implemented first with WebGPU/Dawn or wgpu-native, then native Vulkan/D3D12/Metal backends as needed.
- The null backend is mandatory for tests and headless tools.
- D3D9/DXVK code is transitional legacy infrastructure and must not gain new runtime dependencies.

## Content policy

Retail content is supported directly first, then optionally imported/cooked for faster loading, streaming, mobile, and WebGPU packaging:

```text
MIX / Always.dat / localized resources / W3D assets
  -> retail importer
  -> normalized asset graph
  -> modern cooker
  -> versioned OpenW3D package
```

glTF 2.0/KTX2/BasisU/Opus remain future cooked targets. Retail importers live in tools, while direct retail loading remains available during renderer/platform migration. Stable source IDs and gameplay references must survive either path.

## Migration rules

1. New renderer/platform code is isolated behind adapters; existing gameplay is not rewritten unnecessarily.
2. Retail-format readers are tools, while the existing runtime path remains supported during migration.
3. Legacy engine types may remain behind the transitional WW3D/gameplay boundary; they must not leak into new platform backends.
4. Every subsystem gets a narrow interface and a test/null implementation before a platform implementation.
5. No new proprietary SDKs, 32-bit assumptions, global mutable singleton state, raw ownership, or platform-specific file paths.
6. Keep the legacy build green while the Modern target grows independently.

## Phase status

### Phase 0 — boundary and foundation: complete

### Phase 1 — retail asset reconnaissance: complete

The first inspector implementation is available as `openw3d-asset-inspect`. It accepts explicit roots, validates retail content signatures, scores candidates, and emits schema-versioned JSON inventories. The Windows build was verified against the local Steam installation: `Always.dat` detected, 27 MIX archives detected, 88 data files scanned, and the candidate accepted with score 155.

The next phase is asset parsing/cooking: begin with MIX/archive indexing and extract one real retail asset into the normalized OpenW3D package format.

### Phase 2 — retail archive indexing: complete

`openw3d-mix-index` parses the retail `MIX1` container, validates offsets/counts/name tables, emits a JSON archive index, extracts named entries, and writes the first versioned OpenW3D package (`OWPK`). It was verified against `C&C_Canyon.mix`: 77 entries indexed, `mp_canyon.wlt` extracted at 94,512 bytes, and a 9,995,822-byte `canyon.owpkg` generated with the `OWPK` magic/version header. The package is an initial archive-preserving intermediate; typed W3D/terrain/material conversion is the next content milestone.

### Incremental client slice — Windows SDL3 client build: complete

The existing client now has an explicit renderer boundary: gameplay links the `ww3d2` compatibility implementation behind a target-level marker, while the modern runtime exposes a backend-neutral selection API. The modern runtime smoke output reports `backend=null modern-api=yes`; the real client continues to use the compatibility renderer until a backend adapter is implemented.

### Revised execution priority

The next milestone is not a ground-up runtime. It is a playable incremental port:

```text
existing Renegade gameplay/content
  -> modern build/platform boundary
  -> modern renderer adapter
  -> higher FPS, geometry/effects budgets, and cross-platform backends
```

The first implementation target is the existing client/render boundary, followed by a modern backend or renderer adapter that can boot a real retail map. Asset cooking remains supporting infrastructure rather than the gate to gameplay.

1. Foundation: SDL3 lifecycle, logging, fixed timestep, platform paths, tests.
2. RHI: device/swapchain, buffers, textures, shaders, pipelines, render graph, null backend.
3. Content: asset IDs, retail MIX/Always.dat importer, importer/cooker, package reader, glTF scene loader, hot reload.
4. Playable slice: camera, action input, scene graph/ECS, collision, one authored test scene.
5. Systems: miniaudio/Opus, UI, networking, save/replay, profiling and crash reports.
6. Platform release: Windows, Linux, macOS, Android, iOS, WebGPU builds with CI and packaged content.

A build is not called playable until it launches, loads cooked content originating from retail assets, accepts input, renders a scene, runs a migrated gameplay slice, and exits cleanly on the target platform. It is not required to launch the original retail executable or load its DLLs.
