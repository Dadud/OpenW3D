add_library(openw3d_modern_foundation STATIC
    foundation/runtime.cpp
)

target_compile_features(openw3d_modern_foundation PUBLIC cxx_std_20)
target_include_directories(openw3d_modern_foundation PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_definitions(openw3d_modern_foundation PUBLIC
    OPENW3D_MODERN_RUNTIME=1
)

add_executable(openw3d_modern
    runtime/main.cpp
)
target_link_libraries(openw3d_modern PRIVATE
    openw3d_modern_foundation
)

if(TARGET SDL3::SDL3-shared)
    target_link_libraries(openw3d_modern PRIVATE SDL3::SDL3-shared)
elseif(TARGET SDL3::SDL3)
    target_link_libraries(openw3d_modern PRIVATE SDL3::SDL3)
else()
    message(FATAL_ERROR "W3D_MODERN_RUNTIME requires SDL3")
endif()

if(WIN32)
    target_compile_definitions(openw3d_modern PRIVATE OPENW3D_PLATFORM_WINDOWS=1)
elseif(APPLE)
    target_compile_definitions(openw3d_modern PRIVATE OPENW3D_PLATFORM_APPLE=1)
elseif(ANDROID)
    target_compile_definitions(openw3d_modern PRIVATE OPENW3D_PLATFORM_ANDROID=1)
elseif(EMSCRIPTEN)
    target_compile_definitions(openw3d_modern PRIVATE OPENW3D_PLATFORM_WEB=1)
elseif(UNIX)
    target_compile_definitions(openw3d_modern PRIVATE OPENW3D_PLATFORM_LINUX=1)
endif()

set_target_properties(openw3d_modern PROPERTIES OUTPUT_NAME "openw3d-modern")
