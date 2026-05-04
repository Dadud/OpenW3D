# DXVK needs a shared SDL3 library
if(NOT W3D_BUILD_OPTION_SDL3)
    return()
endif()
find_package(SDL3 REQUIRED COMPONENTS SDL3-shared Headers)
