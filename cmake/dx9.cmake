if(W3D_RENDERER STREQUAL "NULL")
    return()
endif()

if(W3D_RENDERER STREQUAL "NATIVE")
    if(WIN32)
        if(NOT MINGW)
            FetchContent_Declare(
                dx9
                GIT_REPOSITORY https://github.com/madebr/min-dx9-sdk.git
                GIT_TAG        efa003f57d5c6282c45559b50616b24f32c4f442
            )

            FetchContent_MakeAvailable(dx9)
        else()
            add_library(d3d9lib INTERFACE)
            target_link_libraries(d3d9lib INTERFACE d3d9 d3dx9)
        endif()
    else()
        message(FATAL_ERROR "W3D_RENDERER=NATIVE is only supported on Windows. Use W3D_RENDERER=DXVK or NULL on this platform.")
    endif()
elseif(W3D_RENDERER STREQUAL "DXVK")
    set(DXVK_INCLUDE_PATH "" CACHE PATH "Directory containing dxvk/d3d9.h for DXVK-backed D3D9 builds")
    set(DXVK_D3D9_LIBRARY "" CACHE FILEPATH "Path to libdxvk_d3d9 for DXVK-backed D3D9 builds")

    find_path(DXVK_INCLUDE_PATH
        NAMES dxvk/d3d9.h
        HINTS
            "${DXVK_INCLUDE_PATH}"
            "${PROJECT_SOURCE_DIR}/references/dxvk/include"
            "${PROJECT_SOURCE_DIR}/references/fbraz3-dxvk/include"
            "${PROJECT_SOURCE_DIR}/../engine-reference/fbraz3-dxvk/include"
    )

    find_library(DXVK_D3D9_LIBRARY
        NAMES dxvk_d3d9 libdxvk_d3d9
        HINTS
            "${DXVK_D3D9_LIBRARY}"
            "${PROJECT_SOURCE_DIR}/references/dxvk/lib"
            "${PROJECT_SOURCE_DIR}/references/fbraz3-dxvk/build/src/d3d9"
            "${PROJECT_SOURCE_DIR}/../engine-reference/fbraz3-dxvk/build/src/d3d9"
    )

    if(NOT DXVK_INCLUDE_PATH OR NOT DXVK_D3D9_LIBRARY)
        message(FATAL_ERROR
            "W3D_RENDERER=DXVK requires real DXVK/Wine D3D9 headers and libdxvk_d3d9.\n"
            "Expected headers: a directory containing dxvk/d3d9.h.\n"
            "Expected library: libdxvk_d3d9.\n"
            "Pass -DDXVK_INCLUDE_PATH=<include-root> -DDXVK_D3D9_LIBRARY=<path-to-lib>.\n"
            "Do not re-add Code/d3d9_stub; fake SDK stubs are intentionally removed.")
    endif()

    add_library(dxvk_d3d9 UNKNOWN IMPORTED)
    set_property(TARGET dxvk_d3d9 PROPERTY IMPORTED_LOCATION "${DXVK_D3D9_LIBRARY}")
    set_property(TARGET dxvk_d3d9 PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${DXVK_INCLUDE_PATH}")

    add_library(d3d9lib INTERFACE)
    target_link_libraries(d3d9lib INTERFACE dxvk_d3d9)
    # Keep Code/dxvk_wrapper first so <d3d9.h> resolves to the compatibility
    # wrapper, which then includes the real <dxvk/d3d9.h> supplied above.
    target_include_directories(d3d9lib INTERFACE
        "${PROJECT_SOURCE_DIR}/Code/dxvk_wrapper"
        "${DXVK_INCLUDE_PATH}"
    )
else()
    message(FATAL_ERROR "Unsupported W3D_RENDERER value: ${W3D_RENDERER}")
endif()
