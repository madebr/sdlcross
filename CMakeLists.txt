cmake_minimum_required(VERSION 3.0)
project(sdlcross C)

option(WITH_IMAGE "Enable SDL3_image" OFF)
option(WITH_MIXER "Enable SDL3_mixer" OFF)

find_package(SDL3 REQUIRED CONFIG)

if(WITH_IMAGE)
    find_package(SDL3_image CONFIG REQUIRED)
endif()

if(WITH_MIXER)
    find_package(SDL3_mixer CONFIG REQUIRED)
endif()

if(ANDROID)
    enable_language(CXX)
    add_library(sdlcross SHARED main.c)
    set_target_properties(sdlcross PROPERTIES LINKER_LANGUAGE CXX)
else()
    add_executable(sdlcross main.c)
endif()

if(WITH_IMAGE)
    target_compile_definitions(sdlcross PRIVATE WITH_IMAGE)
    target_link_libraries(sdlcross PRIVATE SDL3_image::SDL3_image)
endif()

if(WITH_MIXER)
    target_compile_definitions(sdlcross PRIVATE WITH_MIXER)
    target_link_libraries(sdlcross PRIVATE SDL3_mixer::SDL3_mixer)
endif()

target_link_libraries(sdlcross PRIVATE SDL3::SDL3)

set_target_properties(sdlcross PROPERTIES
    CXX_STANDARD 11
)
if(NOT MSVC)
    target_compile_options(sdlcross PRIVATE -Wall -Wextra -Werror)
endif()

if(ANDROID)
    if(WITH_IMAGE)
        # Explicitly link to libpng to force libpng16.so to be added to the .apk
        # FIXME: how can this be avoided?
        if(TARGET SDL2_image::external_libpng)
            target_link_libraries(sdlcross PRIVATE SDL2_image::external_libpng)
        endif()
    endif()
    if(WITH_MIXER)
        if(TARGET SDL2_mixer::external_libflac)
            target_link_libraries(sdlcross PRIVATE SDL2_mixer::external_libflac)
        endif()
        if(TARGET SDL2_mixer::external_libopusfile)
            target_link_libraries(sdlcross PRIVATE SDL2_mixer::external_libopusfile)
        endif()
        if(TARGET SDL2_mixer::external_libmpg123)
            target_link_libraries(sdlcross PRIVATE SDL2_mixer::external_libmpg123)
        endif()
    endif()
endif()
