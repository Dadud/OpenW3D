# FindOpenAL.cmake — explicit path version
set(OPENAL_ROOT "C:/Users/Dadud/openal-soft/openal-soft-1.24.3-bin/include")
set(OPENAL_LIB_ROOT "C:/Users/Dadud/openal-soft/openal-soft-1.24.3-bin/libs/Win64")
find_path(OpenAL_INCLUDE_DIR
    NAMES AL/al.h
    PATHS "${OPENAL_ROOT}" NO_DEFAULT_PATH)
find_library(OpenAL_LIBRARY
    NAMES OpenAL32 libOpenAL32.dll.a
    PATHS "${OPENAL_LIB_ROOT}" NO_DEFAULT_PATH)
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenAL
    REQUIRED_VARS OpenAL_INCLUDE_DIR OpenAL_LIBRARY)
if(OpenAL_FOUND AND NOT TARGET OpenAL_Lib)
    add_library(OpenAL_Lib UNKNOWN IMPORTED)
    set_target_properties(OpenAL_Lib PROPERTIES
        IMPORTED_LOCATION "${OpenAL_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${OpenAL_INCLUDE_DIR}")
endif()
