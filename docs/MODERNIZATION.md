# OpenW3D modernization architecture

## Product boundary

The modern runtime is a new product surface. It does not load the retail executable or proprietary runtime DLLs. It **does** support retail Renegade content through read-only importers and converters for MIX archives, Always.dat, localized resources, models, textures, maps, missions, definitions, and gameplay data. Retail formats are source inputs, never live runtime dependencies. Legacy code remains available only for selective migration and tooling.

## Runtime layers

```text
Modern/runtime
  application loop, lifecycle, fixed timestep
Modern/platform
  SDL3 windowing, input, filesystem, device discovery
Modern/render
  renderer-independent RHI; Vulkan/D3D12/Metal/WebGPU implementations
Modern/assets
  UUID assets, glTF, KTX2/BasisU, Opus, versioned packages
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

Retail content is imported offline and cooked into a new runtime representation:

```text
MIX / Always.dat / localized resources / W3D assets
  -> retail importer
  -> normalized asset graph
  -> modern cooker
  -> versioned OpenW3D package
```

glTF 2.0 is the initial normalized mesh/material representation, KTX2/BasisU is the texture format, and Opus/Vorbis is the audio format. Packages are versioned, content-addressed, dependency-indexed, and safe to hot reload. Retail importers live in tools and never in the runtime. The importer must preserve stable source IDs and gameplay references so converted missions, objects, maps, and scripts can resolve against the new asset database.

## Migration rules

1. New code may depend on Modern APIs; Modern runtime code may not depend on legacy engine headers.
2. Retail-format readers are tool-only and communicate through normalized, versioned data structures.
3. Legacy code can consume a compatibility adapter temporarily, but the adapter cannot leak legacy types across the Modern boundary.
4. Every subsystem gets a narrow interface and a test/null implementation before a platform implementation.
5. No new proprietary SDKs, 32-bit assumptions, global mutable singleton state, raw ownership, or platform-specific file paths.
6. Keep the legacy build green while the Modern target grows independently.

## Phase status

### Phase 0 — boundary and foundation: complete

### Phase 1 — retail asset reconnaissance: complete

The first inspector implementation is available as `openw3d-asset-inspect`. It accepts explicit roots, validates retail content signatures, scores candidates, and emits schema-versioned JSON inventories. The Windows build was verified against the local Steam installation: `Always.dat` detected, 27 MIX archives detected, 88 data files scanned, and the candidate accepted with score 155.

The next phase is asset parsing/cooking: begin with MIX/archive indexing and extract one real retail asset into the normalized OpenW3D package format.

## Milestones

1. Foundation: SDL3 lifecycle, logging, fixed timestep, platform paths, tests.
2. RHI: device/swapchain, buffers, textures, shaders, pipelines, render graph, null backend.
3. Content: asset IDs, retail MIX/Always.dat importer, importer/cooker, package reader, glTF scene loader, hot reload.
4. Playable slice: camera, action input, scene graph/ECS, collision, one authored test scene.
5. Systems: miniaudio/Opus, UI, networking, save/replay, profiling and crash reports.
6. Platform release: Windows, Linux, macOS, Android, iOS, WebGPU builds with CI and packaged content.

A build is not called playable until it launches, loads cooked content originating from retail assets, accepts input, renders a scene, runs a migrated gameplay slice, and exits cleanly on the target platform. It is not required to launch the original retail executable or load its DLLs.
