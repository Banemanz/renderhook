include(FindPackageHandleStandardArgs)

find_path(imgui_INCLUDE_DIR
    NAMES imgui.h
    PATHS /usr/include/imgui /usr/local/include/imgui
    PATH_SUFFIXES imgui
)

find_library(imgui_LIBRARY
    NAMES imgui
)

if (imgui_INCLUDE_DIR AND imgui_LIBRARY)
    set(_imgui_required_vars imgui_INCLUDE_DIR imgui_LIBRARY)
else ()
    set(_imgui_required_vars imgui_INCLUDE_DIR)
endif ()

find_package_handle_standard_args(imgui
    REQUIRED_VARS ${_imgui_required_vars}
)

if(imgui_FOUND AND NOT TARGET imgui::imgui)
    if (imgui_LIBRARY)
        add_library(imgui::imgui UNKNOWN IMPORTED)
        set_target_properties(imgui::imgui PROPERTIES
            IMPORTED_LOCATION "${imgui_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${imgui_INCLUDE_DIR}"
        )
    else ()
        add_library(imgui::imgui INTERFACE IMPORTED)
        set_target_properties(imgui::imgui PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${imgui_INCLUDE_DIR}"
        )
    endif ()
endif()
