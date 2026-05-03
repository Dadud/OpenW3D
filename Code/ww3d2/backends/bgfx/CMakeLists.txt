# BGFX Shader Build Rules
#
# Compiles .sc shader sources to bgfx binary format (.bin) using the shaderc tool
# from the bgfx submodule. Output goes to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders/
# (i.e. build/shaders/ in an out-of-source build).
#
# Runtime search paths (ShaderVariantCache.cpp):
#   ./shaders/, ../shaders/, shaders/, ../../shaders/
# so the compiled .bin files must be placed next to the executable.
#
# Shader naming convention (must match GetRendererSuffix() in ShaderVariantCache.cpp):
#   vs_mesh_<profile>.bin    — vertex shader for mesh rendering
#   fs_mesh_<profile>.bin    — fragment shader for mesh rendering
#   vs_uber_<profile>.bin    — vertex shader for uber shader (shader variants)
#   fs_uber_<profile>.bin    — fragment shader for uber shader (shader variants)
#
# Profile suffixes:
#   s_5_0  — Direct3D 11 / 12  (Windows)
#   s_3_0  — Direct3D 9        (Windows legacy)
#   120    — OpenGL / GLES      (Linux, macOS)
#   spirv  — Vulkan             (all platforms)
#   metal  — Metal              (macOS)
#
# shaderc CLI (confirmed from bgfx/tools/shaderc/shaderc.cpp):
#   shaderc -f <in> -o <out> --type <vertex/fragment/compute>
#             --platform <windows/linux/osx> -p <profile>
#             --varyingdef <varying.def.sc> -i <include paths>

set(BGFX_SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(BGFX_SHADER_SOURCE_DIR "${BGFX_SHADER_DIR}/source")
set(BGFX_SHADER_EMBED_DIR "${BGFX_SHADER_DIR}/embed")
set(BGFX_SHADER_VARYING_DEF "${BGFX_SHADER_DIR}/varying.def.sc")

# Output to the same directory as the runtime executable (build/shaders/)
set(BGFX_SHADER_OUTPUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders")

# Find shaderc compiler (prefer prebuilt binaries in bgfx submodule)
find_program(SHADERC_EXECUTABLE
    NAMES shaderc shadercRelease
    PATHS
        "${BGFX_DIR}/.build/linux64_gcc/bin"
        "${BGFX_DIR}/.build/win64_vs2022/bin"
        "${BGFX_DIR}/.build/osx64_clang/bin"
    NO_DEFAULT_PATH
)

# Fallback: search PATH
if(NOT SHADERC_EXECUTABLE)
    find_program(SHADERC_EXECUTABLE NAMES shaderc shadercRelease)
endif()

if(NOT SHADERC_EXECUTABLE)
    message(WARNING "shaderc not found. Skipping shader compilation. "
        "Pre-compiled .bin files must be placed in ${BGFX_SHADER_OUTPUT_DIR}/ manually.")
    file(MAKE_DIRECTORY "${BGFX_SHADER_OUTPUT_DIR}")
    return()
endif()

message(STATUS "Found shaderc: ${SHADERC_EXECUTABLE}")

# ─── Shader profiles per platform ─────────────────────────────────────────────

if(WIN32)
    # Windows: D3D11 primary, D3D9 legacy
    set(ALL_PROFILES s_5_0 s_3_0)
    set(PROFILE_PLATFORM_s_5_0 "windows")
    set(PROFILE_PLATFORM_s_3_0 "windows")
elseif(APPLE)
    # macOS: Metal + OpenGL fallback
    set(ALL_PROFILES metal 120)
    set(PROFILE_PLATFORM_metal "osx")
    set(PROFILE_PLATFORM_120 "osx")
else()
    # Linux: OpenGL + Vulkan
    set(ALL_PROFILES 120 spirv)
    set(PROFILE_PLATFORM_120 "linux")
    set(PROFILE_PLATFORM_spirv "linux")
endif()

# ─── Source shaders ────────────────────────────────────────────────────────────

# Mesh shaders: vs_mesh.sc, fs_mesh.sc
set(MESH_SHADER_SOURCES
    "${BGFX_SHADER_SOURCE_DIR}/vs_mesh.sc"
    "${BGFX_SHADER_SOURCE_DIR}/fs_mesh.sc"
)

# Uber shaders: vs_uber.sc, fs_uber.sc
set(UBER_SHADER_SOURCES
    "${BGFX_SHADER_SOURCE_DIR}/vs_uber.sc"
    "${BGFX_SHADER_SOURCE_DIR}/fs_uber.sc"
)

# ─── Compile a single shader ──────────────────────────────────────────────────

function(compile_bgfx_shader INPUT OUTPUT TYPE PROFILE PLATFORM)
    add_custom_command(
        OUTPUT "${OUTPUT}"
        COMMAND "${SHADERC_EXECUTABLE}"
            -f "${INPUT}"
            -o "${OUTPUT}"
            --type ${TYPE}
            --platform ${PLATFORM}
            -p ${PROFILE}
            --varyingdef "${BGFX_SHADER_VARYING_DEF}"
            -i "${BGFX_SHADER_SOURCE_DIR}"
            -i "${BGFX_DIR}/src"
            -i "${BGFX_DIR}/include"
            -i "${BGFX_DIR}/examples/common"
        DEPENDS "${INPUT}" "${BGFX_SHADER_VARYING_DEF}"
        COMMENT "Compiling ${PROFILE} ${OUTPUT}"
        VERBATIM
    )
endfunction()

# ─── Determine shader type from filename prefix ────────────────────────────────

function(shader_type_from_name NAME RESULT)
    if(${NAME} MATCHES "^vs_")
        set(${RESULT} "vertex" PARENT_SCOPE)
    elseif(${NAME} MATCHES "^fs_")
        set(${RESULT} "fragment" PARENT_SCOPE)
    elseif(${NAME} MATCHES "^cs_")
        set(${RESULT} "compute" PARENT_SCOPE)
    else()
        set(${RESULT} "vertex" PARENT_SCOPE)
    endif()
endfunction()

# ─── Compile all source shaders for all profiles ──────────────────────────────

set(ALL_COMPILED)

foreach(SHADER_SRC ${MESH_SHADER_SOURCES} ${UBER_SHADER_SOURCES})
    get_filename_component(SHADER_NAME ${SHADER_SRC} NAME_WE)
    shader_type_from_name(${SHADER_NAME} SHADER_TYPE)

    foreach(PROFILE ${ALL_PROFILES})
        set(PLATFORM "${PROFILE_PLATFORM_${PROFILE}}")
        set(OUTPUT "${BGFX_SHADER_OUTPUT_DIR}/${SHADER_NAME}_${PROFILE}.bin")
        compile_bgfx_shader("${SHADER_SRC}" "${OUTPUT}" "${SHADER_TYPE}" "${PROFILE}" "${PLATFORM}")
        list(APPEND ALL_COMPILED "${OUTPUT}")
    endforeach()
endforeach()

# ─── Output directory ─────────────────────────────────────────────────────────

file(MAKE_DIRECTORY "${BGFX_SHADER_OUTPUT_DIR}")

# ─── Copy embedded / pre-built shaders if present ─────────────────────────────

set(EMBEDDED_COMPILED)
if(EXISTS "${BGFX_SHADER_EMBED_DIR}")
    file(GLOB EMBEDDED_SHADERS "${BGFX_SHADER_EMBED_DIR}/*.bin")
    foreach(EMBED ${EMBEDDED_SHADERS})
        get_filename_component(EMBED_NAME ${EMBED} NAME)
        set(OUTPUT "${BGFX_SHADER_OUTPUT_DIR}/${EMBED_NAME}")
        add_custom_command(
            OUTPUT "${OUTPUT}"
            COMMAND "${CMAKE_COMMAND}" -E copy "${EMBED}" "${OUTPUT}"
            DEPENDS "${EMBED}"
            COMMENT "Copying embedded shader: ${EMBED_NAME}"
        )
        list(APPEND EMBEDDED_COMPILED "${OUTPUT}")
    endforeach()
endif()

# ─── Custom target: bgfx-shaders (always built) ───────────────────────────────

set(ALL_SHADER_OUTPUTS ${ALL_COMPILED} ${EMBEDDED_COMPILED})
list(REMOVE_DUPLICATES ALL_SHADER_OUTPUTS)

add_custom_target(bgfx-shaders ALL DEPENDS ${ALL_SHADER_OUTPUTS})

# Ensure shaders are ready before the main executable
add_dependencies(ww3d2 bgfx-shaders)

message(STATUS "BGFX shader output: ${BGFX_SHADER_OUTPUT_DIR}")
