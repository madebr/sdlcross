if(WIN32 OR OS2)
    message(FATAL_ERROR "This module does not (yet) support Windows")
endif()

find_path(SDL2_image_INCLUDE_DIR
    NAMES SDL_image.h
    SUFFIXES SDL2
)

find_library(SDL2_image_SHARED_LIBRARY
    NAMES ${CMAKE_SHARED_LIBRARY_PREFIX}SDL2_image${CMAKE_SHARED_LIBRARY_SUFFIX}
)

find_library(SDL2_image_STATIC_LIBRARY
    NAMES ${CMAKE_STATIC_LIBRARY_PREFIX}SDL2_image${CMAKE_STATIC_LIBRARY_SUFFIX}
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(SDL2_image
    REQUIRED_VARS SDL2_image_SHARED_LIBRARY SDL2_image_INCLUDE_DIR
)
if(SDL2_image_FOUND)
    if(NOT TARGET SDL2_image::SDL2_image)
        add_library(SDL2_image::SDL2_image SHARED IMPORTED)
        set_target_properties(SDL2_image::SDL2_image
            PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${SDL2_image_INCLUDE_DIR}"
                IMPORTED_LOCATION "${SDL2_image_SHARED_LIBRARY}"
        )
    endif()
    if(SDL2_image_STATIC_LIBRARY AND NOT TARGET SDL2_image::SDL2_image-static)
        add_library(SDL2_image::SDL2_image-static STATIC IMPORTED)
        set_target_properties(SDL2_image::SDL2_image-static
            PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${SDL2_image_INCLUDE_DIR}"
                IMPORTED_LOCATION "${SDL2_image_STATIC_LIBRARY}"
        )
    endif()
endif()
