# FindFFmpeg.cmake — explicit path version
set(FFMPEG_ROOT "C:/Users/Dadud/ffmpeg-dev/ffmpeg-n7.1-latest-win64-gpl-shared-7.1/include")
set(FFMPEG_LIB_ROOT "C:/Users/Dadud/ffmpeg-dev/ffmpeg-n7.1-latest-win64-gpl-shared-7.1/lib")
find_path(FFMPEG_INCLUDE_DIRS
    NAMES libavcodec/avcodec.h libavformat/avformat.h libavutil/avutil.h
    PATHS "${FFMPEG_ROOT}" NO_DEFAULT_PATH)
find_library(FFMPEG_avcodec_LIBRARY
    NAMES avcodec libavcodec.dll.a
    PATHS "${FFMPEG_LIB_ROOT}" NO_DEFAULT_PATH)
find_library(FFMPEG_avformat_LIBRARY
    NAMES avformat libavformat.dll.a
    PATHS "${FFMPEG_LIB_ROOT}" NO_DEFAULT_PATH)
find_library(FFMPEG_avutil_LIBRARY
    NAMES avutil libavutil.dll.a
    PATHS "${FFMPEG_LIB_ROOT}" NO_DEFAULT_PATH)
find_library(FFMPEG_swscale_LIBRARY
    NAMES swscale libswscale.dll.a
    PATHS "${FFMPEG_LIB_ROOT}" NO_DEFAULT_PATH)
find_library(FFMPEG_swsresample_LIBRARY
    NAMES swresample libswresample.dll.a
    PATHS "${FFMPEG_LIB_ROOT}" NO_DEFAULT_PATH)
set(FFMPEG_LIBRARIES
    ${FFMPEG_avcodec_LIBRARY}
    ${FFMPEG_avformat_LIBRARY}
    ${FFMPEG_avutil_LIBRARY}
    ${FFMPEG_swscale_LIBRARY}
    ${FFMPEG_swsresample_LIBRARY})
set(FFMPEG_INCLUDE_DIRS ${FFMPEG_INCLUDE_DIRS})
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FFmpeg
    REQUIRED_VARS FFMPEG_INCLUDE_DIRS FFMPEG_avcodec_LIBRARY FFMPEG_avformat_LIBRARY
                  FFMPEG_avutil_LIBRARY FFMPEG_swscale_LIBRARY FFMPEG_swsresample_LIBRARY)
if(FFmpeg_FOUND)
    add_library(FFmpeg_AVCODEC UNKNOWN IMPORTED)
    set_target_properties(FFmpeg_AVCODEC PROPERTIES
        IMPORTED_LOCATION "${FFMPEG_avcodec_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${FFMPEG_INCLUDE_DIRS}")
    add_library(FFmpeg_AVFORMAT UNKNOWN IMPORTED)
    set_target_properties(FFmpeg_AVFORMAT PROPERTIES
        IMPORTED_LOCATION "${FFMPEG_avformat_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${FFMPEG_INCLUDE_DIRS}")
    add_library(FFmpeg_AVUTIL UNKNOWN IMPORTED)
    set_target_properties(FFmpeg_AVUTIL PROPERTIES
        IMPORTED_LOCATION "${FFMPEG_avutil_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${FFMPEG_INCLUDE_DIRS}")
    add_library(FFmpeg_SWSCALE UNKNOWN IMPORTED)
    set_target_properties(FFmpeg_SWSCALE PROPERTIES
        IMPORTED_LOCATION "${FFMPEG_swscale_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${FFMPEG_INCLUDE_DIRS}")
    add_library(FFmpeg_SWRESAMPLE UNKNOWN IMPORTED)
    set_target_properties(FFmpeg_SWRESAMPLE PROPERTIES
        IMPORTED_LOCATION "${FFMPEG_swsresample_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${FFMPEG_INCLUDE_DIRS}")
endif()
