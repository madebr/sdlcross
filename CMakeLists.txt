cmake_minimum_required(VERSION 3.0)
project(sdlcross C)

find_package(SDL2 REQUIRED CONFIG)

if(ANDROID)
    enable_language(CXX)
    add_library(sdlcross SHARED main.c)
    set_target_properties(sdlcross PROPERTIES LINKER_LANGUAGE CXX)
else()
    add_executable(sdlcross main.c)
endif()
target_link_libraries(sdlcross PRIVATE SDL2::SDL2 SDL2::SDL2main)
set_target_properties(sdlcross PROPERTIES
    CXX_STANDARD 11
)
