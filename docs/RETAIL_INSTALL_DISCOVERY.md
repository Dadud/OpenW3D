# Retail install discovery policy

## Principle

The importer must discover **content roots**, not assume an executable path. Installers, launchers, patches, portable copies, and mod distributions all move the files around. Every candidate path is validated by signatures before it is accepted.

A valid candidate normally contains one or more of:

- `Data/Always.dat` or `data/always.dat`
- localized resources such as `Data/00000409.016` / `00000409.256`
- MIX archives (`*.mix`) in the root or `Data/`
- W3D/map/definition resources in known resource trees
- `Renegade.exe` is useful evidence but is **not required** for modern import

The importer must never execute the retail binary or load its DLLs.

## Edition/path matrix

| Edition/distribution | Candidate locations and discovery method | Confidence |
|---|---|---|
| Original CD/DVD | User-selected directory first. Also inspect `C:/Westwood/Renegade`, `C:/Program Files/Westwood/Renegade`, and `C:/Program Files (x86)/Westwood/Renegade`. | Medium: historical defaults vary by installer/version |
| The First Decade physical/digital bundle | User-selected directory first. Inspect `C:/Program Files (x86)/EA Games/Command & Conquer The First Decade/Command & Conquer Renegade`, its `Renegade` child, and equivalent `Program Files/EA Games` roots. | Medium: bundle/OS/installer choices vary |
| Steam | Parse every Steam `libraryfolders.vdf` found from Steam install roots; inspect each library's `steamapps/common/Command & Conquer Renegade`. Also accept a user-selected Steam library path. | High for Steam layout; local install verified |
| GOG or other DRM-free repackaging | Do not hard-code one path. Inspect launcher manifests if available, then common `C:/GOG Games/`, `C:/Games/`, and user-selected roots. Validate by content signatures. | Low/medium: product packaging and naming vary |
| EA App/Origin-era digital distribution | Prefer launcher metadata. Also inspect EA Games roots and TFD-style nested paths under both Program Files variants. | Medium: launcher metadata is authoritative when present |
| Portable/modded/community copy | User-selected root and explicit recursive scan. Never reject solely because the executable or registry key is absent. | High when signature validation passes |

## Windows discovery sources

Use all of these, in this order:

1. Explicit command-line paths (`--root`, repeatable).
2. Launcher metadata:
   - Steam `libraryfolders.vdf` and `appmanifest_222640.acf` when present.
   - EA App/Origin manifests when present; do not depend on a fixed registry key.
   - GOG Galaxy manifests if present.
3. Registry hints, read-only and optional:
   - `HKLM/HKCU\SOFTWARE\Westwood\Renegade`
   - `HKLM/HKCU\SOFTWARE\WOW6432Node\Westwood\Renegade`
   - equivalent `Electronic Arts\Renegade` keys
   - TFD parent keys containing an install path
4. Known historical defaults.
5. User-selected search roots, with bounded recursion.

Registry and launcher paths are hints only. They must be normalized, deduplicated, and signature-validated.

## Cross-platform discovery

The core importer accepts paths and does not depend on Windows registry APIs. Platform frontends provide optional hints:

- Linux: Steam libraries, `$HOME/.steam/steam`, `$HOME/.local/share/Steam`, Proton prefixes, and explicit paths.
- macOS: Steam libraries under `~/Library/Application Support/Steam`, plus explicit paths.
- Windows: Steam libraries, optional registry/launcher adapters, known defaults, and explicit paths.
- Android/iOS/Web: packaged/imported cooked content only; no retail install scanning at runtime.

The build/import tool should also accept a manifest generated on another machine so discovery and cooking are not tied to the target runtime platform.

## Validation and ranking

Each candidate gets a report:

```text
root
edition_hints
matched_signatures
mix_count
always_dat
localized_resources
w3d_count
map_count
confidence
warnings
```

Suggested ranking:

- +100 `Data/Always.dat`
- +80 localized `00000409.*`
- +5 per MIX archive, capped at +50
- +20 W3D resources
- +10 known launcher manifest match
- +5 executable present
- reject if no recognized content signature

If multiple roots pass, report all of them and select the highest score only when the user did not specify `--root`. Never silently merge two installs.

## Evidence captured during Phase 1 research

- The local Steam install is at `C:/Program Files (x86)/Steam/steamapps/common/Command & Conquer Renegade`.
- It contains `Data/Always.dat`, localized `00000409.*` resources, and 27 MIX archives.
- A registry probe on the current machine found no usable Westwood/Electronic Arts Renegade install key, confirming registry-only discovery is insufficient.
- Search-engine results for this old title are noisy and frequently confuse the game with unrelated content; importer behavior is therefore based on install signatures and launcher metadata rather than search snippets.
