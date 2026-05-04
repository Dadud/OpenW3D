# OpenW3D Architecture

This document describes the high-level architecture of the OpenW3D engine, based on the `electronicarts/CnC_Renegade` source release as modernized by the OpenW3D project.

## Overview

OpenW3D is organized as a set of static libraries linked together to form the game client, dedicated server, and mod tools.

```
┌─────────────────────────────────────────────┐
│              renegade / combat              │  ← Client / Server executables
├─────────────────────────────────────────────┤
│  Code/Combat/         Game logic & rules    │
│  Code/wwui/           UI framework          │
│  Code/ww3d2/          3D renderer + W3D    │
│  Code/WWAudio/        Audio subsystem       │
│  Code/wwnet/          Networking            │
├─────────────────────────────────────────────┤
│  Code/wwphys/         Physics engine        │
│  Code/WWMath/         Math library          │
│  Code/wwlib/          Core utilities        │
│  Code/wwutil/         General utilities     │
│  Code/wwdebug/        Debug & profiling     │
│  Code/wwbitpack/      Bitstream I/O        │
│  Code/wwsaveload/     Serialization        │
│  Code/wwtranslatedb/  INI caching          │
├─────────────────────────────────────────────┤
│  BGFX / OpenAL / FFmpeg / SDL3             │  ← External libraries
└─────────────────────────────────────────────┘
```

## Static Libraries

### WWMath (`Code/WWMath/`)

Pure math library. No dependencies on any other `Code/` library. Provides:

- **Vector math:** `Vector2`, `Vector3`, `Vector4`, `Vector2i`, `Vector3i`
- **Matrix math:** `Matrix3`, `Matrix3D`, `Matrix4`
- **Quaternion:** `Quat` — for 3D rotations
- **Planes:** `Plane` — signed distance planes
- **Collision detection:** AABB, OBB, sphere, ray, frustum tests in `colmath*.cpp`
- **Spatial culling:** `AABTreeCull`, `GridCull`
- **Curves:** Cardinal splines, Catmull-Rom, TCB splines in `curve.cpp`
- **Utility:** lookup tables, random vectors (`v3_rnd.cpp`)

**No external dependencies.** Standard C++ math library only.

---

### wwutil (`Code/wwutil/`)

General-purpose utilities. No dependencies on other `Code/` libraries.

- **String:** `wwstring`, argument parsing (`argv.cpp`)
- **Memory:** custom allocators, memory pool
- **Containers:** dynamic arrays, sorted/unsorted vectors
- **Threading:** critsections (critical sections)
- **Encoding:** base64 pipes/streams

**No external dependencies.**

---

### wwdebug (`Code/wwdebug/`)

Debug-only instrumentation (compiled into all builds).

- `wwprofile.h/cpp` — profiling system with scoped timers
- `wwmemlog.h/cpp` — memory allocation logging
- `wwdebug.h` — assertion macros (`WWASSERT`)

---

### wwbitpack (`Code/wwbitpack/`)

Bitstream packing for network packets and binary file I/O. Reads/writes arbitrary bit-width integer fields from byte buffers.

- `BitPacker` — sequential bit writer
- `BitStream` — sequential bit reader
- `EncoderList` — manages multiple encoder types

Used by `wwnet/` for packet serialization.

---

### wwlib (`Code/wwlib/`)

Core I/O and platform abstraction. Platform-specific code is guarded with `#if defined(_WIN32)`.

- **File I/O:** `FileClass`, `BufferClass`, `ChunkIO`
- **INI parsing:** `INIClass` — reads/writes `.ini`/`.tbl` config files
- **MIX archives:** `MixFileFactoryClass` — handles Renegade's `.mix` package format (collection of files in a binary archive)
- **CRC:** `CRCEngine` — fast CRC32
- **Threading:** `ThreadClass`, mutex, message loop
- **Registry:** `RegistryClass` — Windows registry abstraction (stubbed on Linux)
- **Unicode:** UTF-8/UTF-16 conversion via ICU4C or internal `wchar` shim

**Depends on:** ICU4C (or `wwlib/wchar` fallback on Windows)

---

### wwtranslatedb (`Code/wwtranslatedb/`)

Caches INI parsing results into binary format for fast startup. Translates `.tbl` table files to a binary representation.

---

### wwsaveload (`Code/wwsaveload/`)

Game state serialization — save games, replays.

---

### wwui (`Code/wwui/`)

Immediate-mode GUI framework. Provides dialog boxes, controls, and input.

- **Dialog system:** `DialogManagerClass`, `DialogClass`
- **Controls:** Button, checkbox, list, tree, edit, slider, scrollbar, dropdown, tooltip, progress bar
- **Input:** keyboard, mouse capture, IME support
- **Custom controls:** map display, 3D viewport (via ww3d2)

Platform-specific code guarded with `#if defined(_WIN32)`. Depends on `ww3d2` for viewport rendering.

---

### ww3d2 (`Code/ww3d2/`)

The core 3D engine and W3D file format parser. The largest and most complex library.

#### W3D File Format

`W3D` is a custom binary format based on RenderWare R3 binary stream format. It encodes:
- **Hierarchy:** nodes, meshes, materials, animations
- **Animations:** skeletal (`HAnim`), vertex (`Moph`), keyframe
- **Collision geometry:** AABB trees, sorted meshes
- **Textures:** embedded or referenced DDS/DXT

#### Subsystems

| File | Purpose |
|------|---------|
| `assetmgr.cpp` | Resource management — loads and caches W3D assets |
| `shader.cpp` | Shader state machine — encodes depth, blend, fog, alpha test settings as a bitmask (`ShaderBits`) |
| `aabtree.cpp` | Hierarchical bounding volume tree for frustum culling |
| `camera.h/cpp` | Camera transforms, projection matrices |
| `surfaceclass.cpp` | Low-level texture surface — pixel formats, locking, direct pixel access |
| `textureloader.cpp` | Texture loading pipeline — DXT compression, mipmap generation |
| `lightenvironment.cpp` | Groups lights affecting a mesh and uploads to GPU |
| `material.cpp` | Material properties — diffuse, specular, emissive, shininess |
| `meshmdl.cpp` | Mesh rendering — VB/IB management, FVF layout |
| `terrain.cpp` | Terrain grid rendering |
| `animobj.cpp` | Animated object (skeletal + vertex animation) |
| `particle.cpp` | Particle system |
| `decal.cpp` | Decal projection onto surfaces |

#### Renderer Backends

`Code/ww3d2/backends/`

| Backend | File | Platform |
|---------|------|----------|
| BGFX | `bgfxbackend.cpp` | All |
| Null | `nullrobj.cpp` | All (headless) |
| DX8 | `dx8renderer.cpp` | Windows only |

The `WW3DBackend` interface defines ~50 pure virtual methods that each backend implements. The active backend is selected at build time via CMake.

**Shader pipeline (BGFX):** `ShaderKey` encodes `ShaderBits` as a struct of bitfields. `ShaderVariantCache::GetOrCreate()` maps `ShaderKey` to a compiled BGFX shader program. Shader sources (`.sc` files) are compiled via `shaderc` at build time.

---

### WWAudio (`Code/WWAudio/`)

Cross-platform audio subsystem with pluggable backends.

| Backend | Location | Notes |
|---------|----------|-------|
| OpenAL | `openal/` | HRTF + EFX reverb; default on Linux |
| Miles | `miles/` | EA's proprietary audio SDK; Windows only |
| Null | `null/` | No-op; headless builds |

Audio is spatialized via `Listener` and `Sound3D`/`SoundPseudo3D` objects. FFmpeg is used for video/audio decoding via `WWAudio`.

---

### wwnet (`Code/wwnet/`)

Networking and multiplayer. Original implementation uses `select()`-based I/O — functional but limited to ~50 concurrent connections per server.

- `SocketWrapper` — platform abstraction (Win32 winsock / POSIX BSD sockets)
- `PacketMgr` — packet send/receive queue
- `Connect` — connection establishment and state machine
- `LAN` — UDP broadcast server discovery (original WOL replacement)
- `MsgStat` — per-message-type traffic statistics

**Not currently maintained** — `select()` limits are a known issue for scaling.

---

### wwphys (`Code/wwphys/`)

Custom physics engine (no external physics library). Large and complex.

Key systems:
- **Collision detection:** AABB, OBB, heightfield, terrain grid, pathfinding nodes
- **Rigid body dynamics:** `RigidBodyClass` — linear/angular velocity, forces, torque
- **Vehicle physics:** `VehiclePhys` base class; `TrackedVehicle`, `WheeledVehicle`, `Motorcycle` subclasses
- **Pathfinding:** `Pathfind` — A* grid search on terrain
- **Projectiles:** `Projectile` — ballistic simulation
- **Human physics:** `HumanPhys` — player movement, collision capsule

**No external dependencies.** Self-contained physics implementation.

---

### Combat (`Code/Combat/`)

Game logic layer. Depends on all major libraries.

- Weapon system, damage, health
- Game rules (deathmatch, team, objective)
- Player input handling, command queue
- Game state machine (lobby → playing → end)

---

## Data Flow

### Rendering Pipeline

```
Game Logic (Combat)
  → ww3d2::SceneClass (add render objects)
  → ww3d2::LightEnvironmentClass (group lights)
  → ww3d2::CameraClass (set view/proj)
  → BGFXBackend / DX8Backend / NullBackend
    → ShaderVariantCache (select shader)
    → bgfx::submit (draw call)
```

### Audio Pipeline

```
Game event (Combat)
  → WWAudio::LogicalSound (play sound)
  → OpenAL backend (spatialize)
  → OpenAL soft (HRTF rendering)
  → speakers
```

### Save Load Pipeline

```
Game state
  → wwsaveload::SaveFileClass
  → BufferFileClass (memory buffer)
  → wwbitpack::BitStream (serialize)
  → MIX file / disk
```

---

## Platform Ports

| Platform | Status | Notes |
|----------|--------|-------|
| Windows (DX8) | Historical | Original EA target; DX8 backend unmaintained |
| Windows (BGFX/DX9) | Active | Primary dev platform |
| Linux (BGFX) | Active | Vulkan renderer; main CI target |
| Linux (Null) | Active | Headless server; no GPU |
| macOS | Untested | cmake files present; no CI |

Platform code is gated with `#if defined(_WIN32)` / `#if defined(__APPLE__)` / `#if defined(__linux__)` throughout.

## Threading Model

- **Main thread:** renderer, input, game logic tick
- **Audio thread:** audio buffer fill (separate from main)
- **Network thread:** socket I/O, packet processing (select-based loop)
- **Asset loading:** synchronous on main thread (no async loading layer)

The original `ThreadClass` abstraction is thin — it wraps platform primitives (Win32 threads / pthreads) but provides no work queue or thread pool.
