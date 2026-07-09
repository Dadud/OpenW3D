add_executable(openw3d_platform_shell
    platform_shell.cpp
)

target_link_libraries(openw3d_platform_shell PRIVATE
    wwcommon
    wwlib
    wwmath
    wwdebug
    wwutil
)

if(TARGET SDL3::SDL3-shared)
    target_link_libraries(openw3d_platform_shell PRIVATE SDL3::SDL3-shared)
elseif(TARGET SDL3::SDL3)
    target_link_libraries(openw3d_platform_shell PRIVATE SDL3::SDL3)
else()
    message(FATAL_ERROR "W3D_PLATFORM_SHELL requires SDL3::SDL3 or SDL3::SDL3-shared")
endif()
