if(WIN32)
    if(MSVC)
        FetchContent_Declare(
            dx9
            GIT_REPOSITORY https://github.com/madebr/min-dx9-sdk.git
            GIT_TAG        55973049eaaab204bb35a2b4e33a129a75a16244
        )

        FetchContent_MakeAvailable(dx9)
    else()
        add_library(d3d9lib INTERFACE)
        target_link_libraries(d3d9lib INTERFACE d3d9 d3dx9)
    endif()
else()
    # Linux builds use built-in stub headers in Code/ww3d2/
    # rather than DXVK
    add_library(d3d9lib INTERFACE)
    target_include_directories(d3d9lib INTERFACE "${PROJECT_SOURCE_DIR}/Code/ww3d2")
endif()

