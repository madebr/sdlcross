cmake_minimum_required(VERSION 3.0...4.0)
project(sdlcross C)

option(WITH_IMAGE "Enable SDL3_image" OFF)
option(WITH_MIXER "Enable SDL3_mixer" OFF)
option(WITH_NET "Enable SDL3_net" OFF)
option(WITH_BOX2D "Enable Box2D" OFF)

find_package(SDL3 REQUIRED CONFIG)

if(ANDROID)
    function(add_sdl_executable TARGET)
        add_library(${TARGET} SHARED ${ARGN})
    endfunction()
else()
    function(add_sdl_executable TARGET)
        add_executable(${TARGET} ${ARGN})
    endfunction()
endif()

add_sdl_executable(sdlcross
    src/main.c
)

if(WITH_BOX2D)
    find_package(box2d CONFIG REQUIRED)

    target_sources(sdlcross PRIVATE
        src/box2d-debug-drawer.h
    )
    target_compile_definitions(sdlcross PRIVATE WITH_BOX2D)
    target_link_libraries(sdlcross PRIVATE box2d::box2d)
    if(NOT (ANDROID OR MSVC))
        target_link_libraries(sdlcross PRIVATE m)
    endif()
endif()

if(WITH_IMAGE)
    find_package(SDL3_image CONFIG REQUIRED)

    target_compile_definitions(sdlcross PRIVATE WITH_IMAGE)
    target_link_libraries(sdlcross PRIVATE SDL3_image::SDL3_image)
endif()

if(WITH_MIXER)
    find_package(SDL3_mixer CONFIG REQUIRED)

    target_compile_definitions(sdlcross PRIVATE WITH_MIXER)
    target_link_libraries(sdlcross PRIVATE SDL3_mixer::SDL3_mixer)
endif()

if(WITH_NET)
    find_package(SDL3_net CONFIG REQUIRED)

    target_compile_definitions(sdlcross PRIVATE WITH_NET)
    target_link_libraries(sdlcross PRIVATE SDL3_net::SDL3_net)
endif()

target_link_libraries(sdlcross PRIVATE SDL3::SDL3)

set_target_properties(sdlcross PROPERTIES
    CXX_STANDARD 11
)
if(NOT MSVC)
    target_compile_options(sdlcross PRIVATE -Wall -Wextra -Werror)
endif()
