#!/usr/bin/env bash
set -euo pipefail

OPENW3D_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OPENW3D_ROOT_WIN="$(cygpath -m "$OPENW3D_ROOT")"
DXVK_DIR="${DXVK_DIR:-C:/Users/Dadud/projects/engine-reference/fbraz3-dxvk}"
DXVK_DIR_WIN="$(cygpath -m "$DXVK_DIR")"
ANDROID_NDK_ROOT="${ANDROID_NDK_ROOT:-C:/Users/Dadud/android-dev/sdk/ndk/27.2.12479018}"
SDL3_SOURCE_DIR="${SDL3_SOURCE_DIR:-C:/Users/Dadud/android-dev/SDL3}"
SDL3_PREFIX="${SDL3_PREFIX:-$OPENW3D_ROOT_WIN/build/android-sdl3-shell}"
DXVK_BUILD_DIR="${DXVK_BUILD_DIR:-$DXVK_DIR_WIN/build-android-arm64}"
DXVK_PKGCONFIG_DIR="$DXVK_DIR_WIN/build-android-arm64-pkgconfig"
DXVK_CROSS_FILE="$DXVK_DIR_WIN/build-android-arm64.txt"
GLSLANG_BIN="${GLSLANG_BIN:-C:/VulkanSDK/1.4.350.0/Bin}"
GLSLANG_BIN_MSYS="$(cygpath -u "$GLSLANG_BIN")"
PKGCONF="${PKGCONF:-C:/Users/Dadud/AppData/Local/Microsoft/WinGet/Packages/BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe/mingw64/bin/pkgconf.exe}"

if [[ ! -d "$DXVK_DIR_WIN" ]]; then
  echo "DXVK_DIR not found: $DXVK_DIR_WIN" >&2
  exit 1
fi

if [[ ! -f "$SDL3_PREFIX/libSDL3.so" ]]; then
  echo "SDL3 Android artifact missing: $SDL3_PREFIX/libSDL3.so" >&2
  echo "Run: cmake --preset android-sdl3-shell && cmake --build --preset android-sdl3-shell" >&2
  exit 1
fi

if [[ ! -x "$GLSLANG_BIN_MSYS/glslangValidator.exe" ]]; then
  echo "Modern glslangValidator not found at: $GLSLANG_BIN/glslangValidator.exe" >&2
  echo "Install Vulkan SDK or set GLSLANG_BIN." >&2
  exit 1
fi

mkdir -p "$DXVK_PKGCONFIG_DIR"
cat > "$DXVK_PKGCONFIG_DIR/SDL3.pc" <<EOF_PC
prefix=$SDL3_PREFIX
exec_prefix=\${prefix}
libdir=\${prefix}
includedir=$SDL3_SOURCE_DIR/include

Name: SDL3
Description: Simple DirectMedia Layer is a cross-platform multimedia library
Version: 3.2.10
Libs: -L\${libdir} -lSDL3
Cflags: -I\${includedir}
EOF_PC

cat > "$DXVK_CROSS_FILE" <<EOF_CROSS
[binaries]
c = '$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/windows-x86_64/bin/clang.exe'
cpp = '$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/windows-x86_64/bin/clang++.exe'
ar = '$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/windows-x86_64/bin/llvm-ar.exe'
strip = '$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/windows-x86_64/bin/llvm-strip.exe'
pkgconfig = '$PKGCONF'

[host_machine]
system = 'android'
cpu_family = 'aarch64'
cpu = 'aarch64'
endian = 'little'

[properties]
needs_exe_wrapper = true
pkg_config_libdir = '$DXVK_PKGCONFIG_DIR'

[built-in options]
c_args = ['--target=aarch64-linux-android21', '--sysroot=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/windows-x86_64/sysroot', '-DANDROID', '-DVK_ENABLE_BETA_EXTENSIONS', '-fPIC']
cpp_args = ['--target=aarch64-linux-android21', '--sysroot=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/windows-x86_64/sysroot', '-DANDROID', '-DVK_ENABLE_BETA_EXTENSIONS', '-fPIC']
c_link_args = ['--target=aarch64-linux-android21', '--sysroot=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/windows-x86_64/sysroot']
cpp_link_args = ['--target=aarch64-linux-android21', '--sysroot=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/windows-x86_64/sysroot']
EOF_CROSS

python - "$DXVK_DIR_WIN/subprojects/libdisplay-info/memory-stream.c" <<'PY'
from pathlib import Path
import sys
p = Path(sys.argv[1])
s = p.read_text()
if "defined(__ANDROID__)" not in s:
    s = s.replace("#ifndef _WIN32\n", "#if !defined(_WIN32) && !defined(__ANDROID__)\n", 1)
    marker = "#else\n\n#include <windows.h>"
    android_impl = r'''#elif defined(__ANDROID__)

bool
memory_stream_open(struct memory_stream *m)
{
	*m = (struct memory_stream){ 0 };
	m->fp = tmpfile();

	return m->fp != NULL;
}

char *
memory_stream_close(struct memory_stream *m)
{
	size_t size;
	char *str;
	long end;

	if (m->fp == NULL) {
		*m = (struct memory_stream){ 0 };
		return NULL;
	}

	if (fseek(m->fp, 0, SEEK_END) != 0) {
		fclose(m->fp);
		*m = (struct memory_stream){ 0 };
		return NULL;
	}

	end = ftell(m->fp);
	if (end < 0 || fseek(m->fp, 0, SEEK_SET) != 0) {
		fclose(m->fp);
		*m = (struct memory_stream){ 0 };
		return NULL;
	}
	size = (size_t)end;

	str = malloc(size + 1);
	if (str == NULL) {
		fclose(m->fp);
		*m = (struct memory_stream){ 0 };
		return NULL;
	}

	if (fread(str, 1, size, m->fp) != size) {
		free(str);
		fclose(m->fp);
		*m = (struct memory_stream){ 0 };
		return NULL;
	}
	str[size] = '\0';
	fclose(m->fp);
	*m = (struct memory_stream){ 0 };

	return str;
}

#else

#include <windows.h>'''
    if marker not in s:
        raise SystemExit("Could not patch libdisplay-info memory-stream.c")
    s = s.replace(marker, android_impl, 1)
    p.write_text(s)
PY

export PATH="$GLSLANG_BIN_MSYS:$PATH"
export PKG_CONFIG_PATH="$DXVK_PKGCONFIG_DIR"
cd "$DXVK_DIR_WIN"
if [[ ! -d "$DXVK_BUILD_DIR" ]]; then
  meson setup "$DXVK_BUILD_DIR" --cross-file "$DXVK_CROSS_FILE" --buildtype=release \
    -Denable_dxgi=false -Denable_d3d8=false -Denable_d3d9=true -Denable_d3d10=false -Denable_d3d11=false
fi
meson compile -C "$DXVK_BUILD_DIR" dxvk_d3d9

ARTIFACT="$DXVK_BUILD_DIR/src/d3d9/libdxvk_d3d9.so"
if [[ ! -f "$ARTIFACT" ]]; then
  echo "Expected artifact missing: $ARTIFACT" >&2
  exit 1
fi

echo "$ARTIFACT"
