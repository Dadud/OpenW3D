add_executable(openw3d_renderer_smoke
    renderer_smoke.cpp
)

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
