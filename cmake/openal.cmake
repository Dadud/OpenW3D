find_package(OpenAL REQUIRED)

add_library(openal INTERFACE)
target_link_libraries(openal INTERFACE OpenAL_Lib)
