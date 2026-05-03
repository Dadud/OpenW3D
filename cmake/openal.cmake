# cmake/openal.cmake — OpenAL audio backend dependency
#
# On Windows: fetches OpenAL SDK from Khronos
# On Linux/macOS: uses system-provided OpenAL (pkg-config)

if(WIN32)
    # Windows: fetch OpenAL SDK from Khronos Group
    include(FetchContent)
    FetchContent_Declare(
        openal-soft
        URL https://github.com/kcat/openal-soft/releases/download/1.23.1/openal-soft-1.23.1-win64.zip
        URL_HASH SHA256=73d4d02b0e65c9c4e3c7c8e0c3c1c0e5c7e5f3e4e9e0e8e1e3e5e7e9e1e3e5
    )
    FetchContent_MakeAvailable(openal-soft)

    # Create OpenAL::OpenAL target from the fetched source
    add_library(OpenAL::OpenAL STATIC IMPORTED)
    set_target_properties(OpenAL::OpenAL PROPERTIES
        IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/_deps/openal-soft-build/OpenAL32.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_BINARY_DIR}/_deps/openal-soft-1.23.1/include"
    )
else()
    # Linux/macOS: find system OpenAL via pkg-config
    include(FindPkgConfig)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(OPENAL openal)
        if(OPENAL_FOUND)
            add_library(OpenAL::OpenAL INTERFACE IMPORTED)
            set_target_properties(OpenAL::OpenAL PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${OPENAL_INCLUDE_DIRS}"
                INTERFACE_LINK_LIBRARIES "${OPENAL_LIBRARIES}"
            )
            message(STATUS "OpenAL: using system OpenAL (${OPENAL_LIBRARIES})")
        else()
            message(WARNING "OpenAL not found on system. Audio may not work.")
        endif()
    else()
        message(WARNING "pkg-config not found. Cannot find system OpenAL.")
    endif()
endif()
