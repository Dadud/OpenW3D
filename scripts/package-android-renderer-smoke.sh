#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ROOT_WIN="$(cygpath -m "$ROOT")"
PROJECT="$ROOT/packaging/android-renderer-smoke"
PROJECT_WIN="$(cygpath -m "$PROJECT")"
ANDROID_SDK_ROOT="${ANDROID_SDK_ROOT:-C:/Users/Dadud/android-dev/sdk}"
JAVA_HOME="${JAVA_HOME:-C:/Users/Dadud/android-dev/jdk17}"
JAVA_HOME_WIN="$(cygpath -w "$JAVA_HOME")"
DXVK_DIR="${DXVK_DIR:-C:/Users/Dadud/projects/engine-reference/fbraz3-dxvk}"
ABI_DIR="$PROJECT/app/src/main/jniLibs/arm64-v8a"

mkdir -p "$ABI_DIR"

bash "$ROOT/scripts/build-dxvk-android-arm64.sh"
cmake --preset android-dxvk-renderer-smoke
cmake --build "$ROOT_WIN/build/android-dxvk-renderer-smoke" --target openw3d_renderer_smoke -j8

copy_lib() {
  local src="$1"
  local dst="$ABI_DIR/$(basename "$src")"
  if [[ ! -f "$src" ]]; then
    echo "Missing native library: $src" >&2
    exit 1
  fi
  cp -f "$src" "$dst"
}

copy_lib "$ROOT/build/android-dxvk-renderer-smoke/libmain.so"
copy_lib "$ROOT/build/android-dxvk-renderer-smoke/libSDL3.so"
copy_lib "$DXVK_DIR/build-android-arm64/src/d3d9/libdxvk_d3d9.so"
copy_lib "$ANDROID_SDK_ROOT/ndk/27.2.12479018/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/lib/aarch64-linux-android/libc++_shared.so"

cat > "$PROJECT/local.properties" <<EOF_PROPS
sdk.dir=C:/Users/Dadud/android-dev/sdk
EOF_PROPS

cd "$PROJECT"
cmd.exe /c "set JAVA_HOME=$JAVA_HOME_WIN&& set PATH=$JAVA_HOME_WIN\\bin;%PATH%&& gradlew.bat assembleDebug"

echo "$PROJECT_WIN/app/build/outputs/apk/debug/app-debug.apk"
