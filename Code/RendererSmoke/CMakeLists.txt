if(OPENW3D_ANDROID)
    add_library(openw3d_renderer_smoke SHARED
        renderer_smoke.cpp
        smoke_stubs.cpp
    )
    set_target_properties(openw3d_renderer_smoke PROPERTIES
        OUTPUT_NAME main
    )
else()
    add_executable(openw3d_renderer_smoke
        renderer_smoke.cpp
        smoke_stubs.cpp
    )
endif()

target_link_libraries(openw3d_renderer_smoke PRIVATE
    wwcommon
    ww3d2
    wwlib
    wwmath
    wwdebug
    wwutil
    wwsaveload
    wwtranslatedb
)

if(TARGET SDL3::SDL3-shared)
    target_link_libraries(openw3d_renderer_smoke PRIVATE SDL3::SDL3-shared)
elseif(TARGET SDL3::SDL3)
    target_link_libraries(openw3d_renderer_smoke PRIVATE SDL3::SDL3)
else()
    message(FATAL_ERROR "W3D_RENDERER_SMOKE requires SDL3::SDL3 or SDL3::SDL3-shared")
endif()
