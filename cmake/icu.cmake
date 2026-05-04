find_package(ICU COMPONENTS data i18n io uc)

if(NOT ICU_FOUND)
    message(WARNING "ICU not found — disabling ICU support. Install libicu-dev for Unicode string handling.")
    set(W3D_BUILD_OPTION_ICU OFF CACHE BOOL "Build with ICU." FORCE)
    return()
endif()

add_library(icu INTERFACE)
target_link_libraries(icu INTERFACE ICU::data ICU::i18n ICU::io ICU::uc)
