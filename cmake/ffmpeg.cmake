find_package(FFmpeg COMPONENTS AVCODEC AVFORMAT AVUTIL SWSCALE)

if(NOT FFmpeg_FOUND)
    message(WARNING "FFmpeg not found — disabling FFmpeg support. Install libavcodec-dev libavformat-dev libavutil-dev libswscale-dev for video/audio support.")
    set(W3D_BUILD_OPTION_FFMPEG OFF CACHE BOOL "Build with ffmpeg." FORCE)
    return()
endif()

add_library(ffmpeg INTERFACE)
target_link_libraries(ffmpeg INTERFACE FFmpeg::AVCODEC FFmpeg::AVFORMAT FFmpeg::AVUTIL FFmpeg::SWSCALE)
