#!/usr/bin/env bash
#
# OpenW3D Interactive Build Tool
# Cross-platform build script for Linux, macOS, and Windows (via MSYS2/WSL)
#
# Usage:
#   ./build.sh                # Interactive mode
#   ./build.sh --quick        # Non-interactive, sensible defaults
#   ./build.sh --help         # Show usage
#

set -euo pipefail

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# ---------------------------------------------------------------------------
# Platform Detection
# ---------------------------------------------------------------------------
 detect_platform() {
    case "$(uname -s)" in
        Linux*)     PLATFORM="linux";;
        Darwin*)    PLATFORM="macos";;
        CYGWIN*|MINGW*|MSYS*) PLATFORM="windows";;
        *)          PLATFORM="unknown";;
    esac
    echo "$PLATFORM"
}

PLATFORM=$(detect_platform)

# ---------------------------------------------------------------------------
# Logging
# ---------------------------------------------------------------------------
info() { echo -e "${BLUE}[INFO]${NC} $*"; }
warn() { echo -e "${YELLOW}[WARN]${NC} $*"; }
error() { echo -e "${RED}[ERROR]${NC} $*"; }
success() { echo -e "${GREEN}[OK]${NC} $*"; }
step() { echo -e "${CYAN}==>${NC} $*"; }

# ---------------------------------------------------------------------------
# Dependency Checks
# ---------------------------------------------------------------------------
check_command() {
    command -v "$1" >/dev/null 2>&1
}

check_deps_linux() {
    local missing=()
    
    if ! check_command cmake; then
        missing+=("cmake")
    fi
    
    if ! check_command git; then
        missing+=("git")
    fi
    
    if ! check_command g++; then
        missing+=("build-essential / g++")
    fi
    
    # Check for optional but recommended packages
    if ! pkg-config --exists libavcodec 2>/dev/null; then
        warn "FFmpeg development libraries not found (optional, needed for video/audio)"
        warn "  Install: sudo apt-get install libavcodec-dev libavformat-dev libavutil-dev libswscale-dev"
    fi
    
    if ! pkg-config --exists openal 2>/dev/null; then
        warn "OpenAL development libraries not found (optional, needed for 3D audio)"
        warn "  Install: sudo apt-get install libopenal-dev"
    fi
    
    if ! pkg-config --exists sdl3 2>/dev/null; then
        warn "SDL3 not found — client build will use null/headless backend"
        warn "  Install: sudo apt-get install libsdl3-dev  (or build from source)"
    fi
    
    if [ ${#missing[@]} -gt 0 ]; then
        error "Missing required packages: ${missing[*]}"
        error "Install with: sudo apt-get install -y ${missing[*]}"
        exit 1
    fi
}

check_deps_windows() {
    if ! check_command cmake; then
        error "CMake not found. Install from https://cmake.org/download/"
        exit 1
    fi
    
    if ! check_command git; then
        error "Git not found. Install from https://git-scm.com/download/win"
        exit 1
    fi
    
    # Detect compiler
    if check_command cl; then
        success "MSVC (Visual Studio) detected"
        COMPILER="msvc"
    elif check_command g++; then
        success "MinGW/GCC detected"
        COMPILER="mingw"
    else
        error "No C++ compiler found. Install Visual Studio or MinGW."
        exit 1
    fi
}

check_deps_macos() {
    if ! check_command cmake; then
        error "CMake not found. Install with: brew install cmake"
        exit 1
    fi
    
    if ! check_command git; then
        error "Git not found. Install Xcode Command Line Tools."
        exit 1
    fi
    
    if ! check_command clang++; then
        error "Clang not found. Install Xcode Command Line Tools."
        exit 1
    fi
}

# ---------------------------------------------------------------------------
# Submodule Setup
# ---------------------------------------------------------------------------
setup_submodules() {
    step "Setting up external dependencies..."
    
    # The repo's .gitmodules file exists but submodules aren't registered in git.
    # We manually clone them into external/.
    
    mkdir -p external
    
    if [ ! -f "external/bgfx/.git" ]; then
        info "Cloning bgfx..."
        rm -rf external/bgfx
        git clone --depth 1 https://github.com/bkaradzic/bgfx.git external/bgfx
    else
        info "bgfx already present"
    fi
    
    if [ ! -f "external/bx/.git" ]; then
        info "Cloning bx..."
        rm -rf external/bx
        git clone --depth 1 https://github.com/bkaradzic/bx.git external/bx
    else
        info "bx already present"
    fi
    
    if [ ! -f "external/bimg/.git" ]; then
        info "Cloning bimg..."
        rm -rf external/bimg
        git clone --depth 1 https://github.com/bkaradzic/bimg.git external/bimg
    else
        info "bimg already present"
    fi
    
    success "External dependencies ready"
}

# ---------------------------------------------------------------------------
# BGFX Build
# ---------------------------------------------------------------------------
build_bgfx() {
    step "Building BGFX..."
    
    cd external/bgfx
    
    case "$PLATFORM" in
        linux)
            if [ ! -f ".build/linux64_gcc/bin/libbgfxRelease.a" ]; then
                make linux-gcc-release64
            else
                info "BGFX already built"
            fi
            ;;
        macos)
            if [ ! -f ".build/osx64_clang/bin/libbgfxRelease.a" ]; then
                make osx-clang-release64
            else
                info "BGFX already built"
            fi
            ;;
        windows)
            if [ ! -f ".build/win64_vs2022/bin/bgfxRelease.lib" ]; then
                ../../bx/tools/bin/windows/genie.exe vs2022
                info "BGFX VS2022 project generated. Build it manually or use MSBuild."
                # For MinGW:
                # make mingw-gcc-release64
            else
                info "BGFX already built"
            fi
            ;;
    esac
    
    cd "$SCRIPT_DIR"
    success "BGFX build complete"
}

# ---------------------------------------------------------------------------
# Interactive Configuration
# ---------------------------------------------------------------------------
interactive_config() {
    step "Build Configuration"
    echo ""
    
    # Backend selection
    echo "Select renderer backend:"
    echo "  1) Null / Headless (no GPU required, server-only)"
    echo "  2) BGFX / Vulkan (modern renderer, requires GPU + shader compilation)"
    echo ""
    
    if [ "$PLATFORM" = "windows" ]; then
        echo "  3) DirectX 9 (native Windows, legacy)"
        echo ""
    fi
    
    read -rp "Choice [1]: " backend_choice
    backend_choice=${backend_choice:-1}
    
    case "$backend_choice" in
        1)
            ENABLE_BGFX=OFF
            BUILD_TYPE="headless"
            ;;
        2)
            ENABLE_BGFX=ON
            BUILD_TYPE="bgfx"
            ;;
        3)
            if [ "$PLATFORM" = "windows" ]; then
                ENABLE_BGFX=OFF
                BUILD_TYPE="dx9"
            else
                warn "DX9 is Windows-only. Using headless."
                ENABLE_BGFX=OFF
                BUILD_TYPE="headless"
            fi
            ;;
        *)
            ENABLE_BGFX=OFF
            BUILD_TYPE="headless"
            ;;
    esac
    
    # Build type
    echo ""
    echo "Select build type:"
    echo "  1) Release (optimized, recommended)"
    echo "  2) Debug (with symbols, slower)"
    echo "  3) RelWithDebInfo (optimized + symbols)"
    echo ""
    read -rp "Choice [1]: " build_type_choice
    build_type_choice=${build_type_choice:-1}
    
    case "$build_type_choice" in
        1) CMAKE_BUILD_TYPE="Release";;
        2) CMAKE_BUILD_TYPE="Debug";;
        3) CMAKE_BUILD_TYPE="RelWithDebInfo";;
        *) CMAKE_BUILD_TYPE="Release";;
    esac
    
    # Optional features
    echo ""
    echo "Optional features:"
    
    if pkg-config --exists libavcodec 2>/dev/null; then
        read -rp "Enable FFmpeg (video/audio decoding)? [Y/n]: " enable_ffmpeg
        enable_ffmpeg=${enable_ffmpeg:-Y}
    else
        warn "FFmpeg not detected — disabling"
        enable_ffmpeg="N"
    fi
    
    if pkg-config --exists openal 2>/dev/null; then
        read -rp "Enable OpenAL (3D spatial audio)? [Y/n]: " enable_openal
        enable_openal=${enable_openal:-Y}
    else
        warn "OpenAL not detected — disabling"
        enable_openal="N"
    fi
    
    if pkg-config --exists sdl3 2>/dev/null; then
        read -rp "Enable SDL3 (windowing/input)? [Y/n]: " enable_sdl3
        enable_sdl3=${enable_sdl3:-Y}
    else
        warn "SDL3 not detected — client will be headless"
        enable_sdl3="N"
    fi
    
    # Targets
    echo ""
    echo "Select build targets:"
    echo "  1) Core libraries only (wwmath, wwlib, ww3d2, etc.)"
    echo "  2) Game client + server (renegade, renegadeserver, combat, combate)"
    echo "  3) Everything including tools (LevelEdit, W3DView, etc.)"
    echo ""
    read -rp "Choice [2]: " target_choice
    target_choice=${target_choice:-2}
    
    case "$target_choice" in
        1) BUILD_TARGETS="wwmath wwutil wwdebug wwbitpack wwlib ww3d2";;
        2) BUILD_TARGETS="";;  # empty = default targets
        3) BUILD_TARGETS="";;
    esac
}

# ---------------------------------------------------------------------------
# Quick/Default Configuration
# ---------------------------------------------------------------------------
quick_config() {
    ENABLE_BGFX=OFF
    BUILD_TYPE="headless"
    CMAKE_BUILD_TYPE="Release"
    
    if pkg-config --exists libavcodec 2>/dev/null; then
        enable_ffmpeg="Y"
    else
        enable_ffmpeg="N"
    fi
    
    if pkg-config --exists openal 2>/dev/null; then
        enable_openal="Y"
    else
        enable_ffmpeg="N"
    fi
    
    if pkg-config --exists sdl3 2>/dev/null; then
        enable_sdl3="Y"
    else
        enable_sdl3="N"
    fi
    
    BUILD_TARGETS=""
}

# ---------------------------------------------------------------------------
# CMake Configure
# ---------------------------------------------------------------------------
configure_cmake() {
    step "Configuring with CMake..."
    
    local cmake_args=()
    cmake_args+=("-S" ".")
    cmake_args+=("-B" "build")
    cmake_args+=("-DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE")
    
    if [ "$ENABLE_BGFX" = "ON" ]; then
        cmake_args+=("-DENABLE_BGFX_BACKEND=ON")
    else
        cmake_args+=("-DENABLE_BGFX_BACKEND=OFF")
    fi
    
    if [ "${enable_ffmpeg:-N}" = "Y" ] || [ "${enable_ffmpeg:-N}" = "y" ]; then
        cmake_args+=("-DW3D_BUILD_OPTION_FFMPEG=ON")
    else
        cmake_args+=("-DW3D_BUILD_OPTION_FFMPEG=OFF")
    fi
    
    if [ "${enable_openal:-N}" = "Y" ] || [ "${enable_openal:-N}" = "y" ]; then
        cmake_args+=("-DW3D_BUILD_OPTION_OPENAL=ON")
    else
        cmake_args+=("-DW3D_BUILD_OPTION_OPENAL=OFF")
    fi
    
    if [ "${enable_sdl3:-N}" = "Y" ] || [ "${enable_sdl3:-N}" = "y" ]; then
        cmake_args+=("-DW3D_BUILD_OPTION_SDL3=ON")
    else
        cmake_args+=("-DW3D_BUILD_OPTION_SDL3=OFF")
    fi
    
    # Windows-specific
    if [ "$PLATFORM" = "windows" ] && [ "$COMPILER" = "msvc" ]; then
        cmake_args+=("-G" "Visual Studio 17 2022" "-Ax64")
    fi
    
    info "Running: cmake ${cmake_args[*]}"
    cmake "${cmake_args[@]}"
    
    success "CMake configuration complete"
}

# ---------------------------------------------------------------------------
# Build
# ---------------------------------------------------------------------------
run_build() {
    step "Building OpenW3D..."
    
    local build_args=()
    build_args+=("--build" "build")
    build_args+=("--parallel")
    
    if [ -n "${BUILD_TARGETS:-}" ]; then
        build_args+=("--target" $BUILD_TARGETS)
    fi
    
    info "Running: cmake ${build_args[*]}"
    cmake "${build_args[@]}"
    
    success "Build complete!"
}

# ---------------------------------------------------------------------------
# Asset Detection
# ---------------------------------------------------------------------------
find_renegade_assets() {
    step "Looking for C&C Renegade game assets..."
    
    local found_paths=()
    
    case "$PLATFORM" in
        linux)
            # Common Linux/Wine paths
            local search_paths=(
                "$HOME/.wine/drive_c/Program Files/EA Games/Command & Conquer Renegade"
                "$HOME/.wine/drive_c/Program Files (x86)/EA Games/Command & Conquer Renegade"
                "$HOME/Games/renegade"
                "/media"/*/renegade
                "/mnt"/*/renegade
            )
            
            # Steam Proton / Linux native paths
            if [ -d "$HOME/.local/share/Steam/steamapps" ]; then
                while IFS= read -r -d '' manifest; do
                    local install_dir
                    install_dir=$(grep -oP '"installdir"\s*"\K[^"]+' "$manifest" 2>/dev/null | head -1)
                    if [ -n "$install_dir" ]; then
                        local app_path="$(dirname "$manifest")/common/$install_dir"
                        if [ -f "$app_path/conquer.mix" ] || [ -f "$app_path/data/conquer.mix" ]; then
                            search_paths+=("$app_path")
                        fi
                    fi
                done < <(find "$HOME/.local/share/Steam/steamapps" -name "appmanifest_*.acf" -print0 2>/dev/null)
            fi
            
            # Lutris
            if [ -d "$HOME/Games" ]; then
                while IFS= read -r -d '' lutris_path; do
                    search_paths+=("$lutris_path")
                done < <(find "$HOME/Games" -maxdepth 2 -name "*.mix" -printf '%h\0' 2>/dev/null | sort -z -u)
            fi
            ;;
            
        macos)
            search_paths=(
                "$HOME/Library/Application Support/CrossOver/Bottles/*/drive_c/Program Files/EA Games/Command & Conquer Renegade"
                "/Volumes"/*/renegade
            )
            ;;
            
        windows)
            # Check registry for install paths
            local reg_paths=()
            if check_command reg; then
                reg_paths+=(
                    "$(reg query 'HKLM\SOFTWARE\EA Games\Command and Conquer Renegade' /v InstallPath 2>/dev/null | grep -oP 'REG_SZ\s+\K.*' || true)"
                    "$(reg query 'HKLM\SOFTWARE\WOW6432Node\EA Games\Command and Conquer Renegade' /v InstallPath 2>/dev/null | grep -oP 'REG_SZ\s+\K.*' || true)"
                )
            fi
            
            search_paths=(
                "C:\\Program Files\\EA Games\\Command & Conquer Renegade"
                "C:\\Program Files (x86)\\EA Games\\Command & Conquer Renegade"
                "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Renegade"
                "C:\\Program Files\\Steam\\steamapps\\common\\Renegade"
                "${reg_paths[@]}"
            )
            ;;
    esac
    
    for path in "${search_paths[@]}"; do
        if [ -f "$path/conquer.mix" ] || [ -f "$path/data/conquer.mix" ]; then
            found_paths+=("$path")
        fi
    done
    
    if [ ${#found_paths[@]} -eq 0 ]; then
        warn "Could not find C&C Renegade installation automatically."
        warn "You will need to manually copy .mix files to the Run/ directory."
        return 1
    fi
    
    echo ""
    echo "Found potential installs:"
    local i=1
    for path in "${found_paths[@]}"; do
        echo "  $i) $path"
        ((i++))
    done
    echo ""
    
    read -rp "Select install to copy assets from [1]: " choice
    choice=${choice:-1}
    
    local selected="${found_paths[$((choice-1))]}"
    
    if [ -n "$selected" ]; then
        info "Copying .mix files from: $selected"
        mkdir -p Run
        cp -v "$selected"/*.mix Run/ 2>/dev/null || true
        if [ -d "$selected/data" ]; then
            cp -v "$selected/data"/*.mix Run/ 2>/dev/null || true
        fi
        success "Assets copied to Run/"
    fi
}

# ---------------------------------------------------------------------------
# Post-Build Setup
# ---------------------------------------------------------------------------
post_build() {
    step "Post-build setup..."
    
    mkdir -p Run
    
    # Symlink or copy binaries to Run/
    if [ -d "build" ]; then
        find build -maxdepth 1 -type f -executable -exec cp -v {} Run/ \; 2>/dev/null || true
    fi
    
    # Check for assets
    if [ ! -f "Run/conquer.mix" ] && [ ! -f "Run/data/conquer.mix" ]; then
        warn "Game assets (.mix files) not found in Run/"
        
        read -rp "Try to auto-detect C&C Renegade install? [Y/n]: " detect
        detect=${detect:-Y}
        
        if [ "$detect" = "Y" ] || [ "$detect" = "y" ]; then
            find_renegade_assets || true
        fi
    else
        success "Game assets already present in Run/"
    fi
    
    echo ""
    echo "=========================================="
    echo "  OpenW3D Build Complete!"
    echo "=========================================="
    echo ""
    echo "Build type:    $BUILD_TYPE"
    echo "CMake config:  $CMAKE_BUILD_TYPE"
    echo "Backend:       ${ENABLE_BGFX:-OFF}"
    echo ""
    echo "Binaries:      build/"
    echo "Game data:     Run/"
    echo ""
    
    if [ ! -f "Run/conquer.mix" ]; then
        echo "⚠️  Place your C&C Renegade .mix files in Run/ before running"
    fi
    
    echo ""
    echo "To run:"
    echo "  ./build/renegade    (client)"
    echo "  ./build/renegadeserver  (dedicated server)"
    echo ""
}

# ---------------------------------------------------------------------------
# Help
# ---------------------------------------------------------------------------
show_help() {
    cat << 'EOF'
OpenW3D Interactive Build Tool

Usage:
  ./build.sh              Run interactive build wizard
  ./build.sh --quick      Build with sensible defaults (no prompts)
  ./build.sh --help       Show this help message

Platforms:
  Linux    - Native build with GCC/Clang
  macOS    - Native build with Clang
  Windows  - MSYS2/MinGW or Visual Studio (via CMake)

Dependencies (Linux):
  sudo apt-get install -y \
    build-essential cmake ninja-build pkg-config git \
    libavcodec-dev libavformat-dev libavutil-dev libswscale-dev \
    libasound2-dev libpulse-dev \
    libvulkan-dev \
    libopenal-dev \
    libsdl3-dev \
    libx11-dev libxrandr-dev libxcursor-dev

Dependencies (macOS):
  brew install cmake ninja pkg-config git \
    ffmpeg alsa-lib portaudio vulkan-loader

Dependencies (Windows MSYS2):
  pacman -S mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja \
    mingw-w64-x86_64-gcc mingw-w64-x86_64-ffmpeg \
    mingw-w64-x86_64-pkg-config

Game Assets:
  You need a legal copy of C&C Renegade. This script can attempt to
  auto-detect your installation and copy the .mix data files.

EOF
}

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------
main() {
    echo ""
    echo "=========================================="
    echo "  OpenW3D Interactive Build Tool"
    echo "  Platform: ${PLATFORM}"
    echo "=========================================="
    echo ""
    
    case "${1:-}" in
        --help|-h)
            show_help
            exit 0
            ;;
        --quick|-q)
            QUICK_MODE=1
            ;;
        "")
            QUICK_MODE=0
            ;;
        *)
            error "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
    
    # Check platform
    case "$PLATFORM" in
        linux)
            check_deps_linux
            ;;
        macos)
            check_deps_macos
            ;;
        windows)
            check_deps_windows
            ;;
        *)
            error "Unsupported platform: $(uname -s)"
            exit 1
            ;;
    esac
    
    # Setup submodules
    setup_submodules
    
    # Configure
    if [ "$QUICK_MODE" -eq 1 ]; then
        info "Quick mode: using defaults"
        quick_config
    else
        interactive_config
    fi
    
    # Build BGFX if needed
    if [ "$ENABLE_BGFX" = "ON" ]; then
        build_bgfx
    fi
    
    # Configure and build
    configure_cmake
    run_build
    
    # Post-build
    post_build
}

main "$@"
