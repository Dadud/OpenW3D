# dx9.cmake — Direct3D 9 support
#
# Windows: Provides min-dx9-sdk (DX8/9 headers/lib) for native DX backend.
#          When ENABLE_BGFX_BACKEND=ON on Windows, the DX backend is optional
#          and the min-dx9-sdk is not required (BGFX handles Vulkan/D3D11+).
#
# Non-Windows: No DX9/DX8 support. Only BGFX is available.
#              BGFX uses Vulkan on Linux. No dxvk headers needed.

if(WIN32)
    if(NOT MINGW)
        FetchContent_Declare(
            dx9
            GIT_REPOSITORY https://github.com/madebr/min-dx9-sdk.git
            GIT_TAG        55973049eaaab204bb35a2b4e33a129a75a16244
        )

        FetchContent_MakeAvailable(dx9)
    else()
        # MinGW on Windows: link against system d3d9/d3dx9
        add_library(d3d9lib INTERFACE)
        target_link_libraries(d3d9lib INTERFACE d3d9 d3dx9)
    endif()
else()
    # Non-Windows: no DirectX. BGFX-only.
    # ww3d_platform.h provides minimal D3D type stubs so shared code compiles.
    # Code/ww3d2/d3d9.h redirects to ww3d_platform.h for files that #include <d3d9.h>.
    add_library(d3d9lib INTERFACE)
    target_include_directories(d3d9lib INTERFACE "${PROJECT_SOURCE_DIR}/Code/ww3d2")
endif()
