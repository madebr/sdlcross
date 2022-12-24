if(WIN32 OR OS2)
    message(FATAL_ERROR "This module does not (yet) support Windows")
endif()

find_path(SDL2_mixer_INCLUDE_DIR
    NAMES SDL_mixer.h
    SUFFIXES SDL2
)

find_library(SDL2_mixer_SHARED_LIBRARY
    NAMES ${CMAKE_SHARED_LIBRARY_PREFIX}SDL2_mixer${CMAKE_SHARED_LIBRARY_SUFFIX}
)

find_library(SDL2_mixer_STATIC_LIBRARY
    NAMES ${CMAKE_STATIC_LIBRARY_PREFIX}SDL2_mixer${CMAKE_STATIC_LIBRARY_SUFFIX}
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(SDL2_mixer
    REQUIRED_VARS SDL2_mixer_SHARED_LIBRARY SDL2_mixer_INCLUDE_DIR
)
if(SDL2_mixer_FOUND)
    if(NOT TARGET SDL2_mixer::SDL2_mixer)
        add_library(SDL2_mixer::SDL2_mixer SHARED IMPORTED)
        set_target_properties(SDL2_mixer::SDL2_mixer
            PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${SDL2_mixer_INCLUDE_DIR}"
                IMPORTED_LOCATION "${SDL2_mixer_SHARED_LIBRARY}"
        )
    endif()
    if(SDL2_mixer_STATIC_LIBRARY AND NOT TARGET SDL2_mixer::SDL2_mixer-static)
        add_library(SDL2_mixer::SDL2_mixer-static STATIC IMPORTED)
        set_target_properties(SDL2_mixer::SDL2_mixer-static
            PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${SDL2_mixer_INCLUDE_DIR}"
                IMPORTED_LOCATION "${SDL2_mixer_STATIC_LIBRARY}"
        )
    endif()
endif()
