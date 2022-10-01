cmake_minimum_required(VERSION 3.0)
project(sdlcross C)

find_package(SDL2 REQUIRED CONFIG)
find_package(SDL2_image CONFIG)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
if(NOT SDL2_image_FOUND)
    find_package(SDL2_image REQUIRED)
endif()

if(ANDROID)
    enable_language(CXX)
    add_library(sdlcross SHARED main.c)
    set_target_properties(sdlcross PROPERTIES LINKER_LANGUAGE CXX)
else()
    add_executable(sdlcross main.c)
endif()
target_link_libraries(sdlcross PRIVATE SDL2::SDL2 SDL2::SDL2main SDL2_image::SDL2_image)
set_target_properties(sdlcross PROPERTIES
    CXX_STANDARD 11
)
