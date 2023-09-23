function(tdesktopenvironment_register_wayland_protocol_extension target)
    set(multiValueArgs FILES)
    cmake_parse_arguments(REGISTER_WAYLAND_PROTOCOL_EXTENSIONS "" "" "${oneValueArgs}" ${ARGN})

    foreach(_file ${REGISTER_WAYLAND_PROTOCOL_EXTENSIONS_FILES})
        get_filename_component(_basename ${_file} NAME_WE)

        # Wayland scanner
        add_custom_command(OUTPUT "wayland-${_basename}-client-protocol.h"
                           COMMAND ${WAYLAND_SCANNER} client-header ${CMAKE_CURRENT_SOURCE_DIR}/${_file} "${CMAKE_CURRENT_BINARY_DIR}/wayland-${_basename}-client-protocol.h"
                           DEPENDS ${_file}
                           VERBATIM)

        add_custom_command(OUTPUT "wayland-${_basename}-client-protocol.c"
                           COMMAND ${WAYLAND_SCANNER} private-code ${CMAKE_CURRENT_SOURCE_DIR}/${_file} "${CMAKE_CURRENT_BINARY_DIR}/wayland-${_basename}-client-protocol.c"
                           DEPENDS ${_file}
                           VERBATIM)

        # Qt Wayland scanner
        add_custom_command(OUTPUT "qwayland-${_basename}.h"
                           COMMAND ${QT_WAYLAND_SCANNER} client-header ${CMAKE_CURRENT_SOURCE_DIR}/${_file} > "${CMAKE_CURRENT_BINARY_DIR}/qwayland-${_basename}.h"
                           DEPENDS ${_file}
                           VERBATIM)

        add_custom_command(OUTPUT "qwayland-${_basename}.cpp"
                           COMMAND ${QT_WAYLAND_SCANNER} client-code ${CMAKE_CURRENT_SOURCE_DIR}/${_file} > "${CMAKE_CURRENT_BINARY_DIR}/qwayland-${_basename}.cpp"
                           DEPENDS ${_file}
                           VERBATIM)

        # Include in sources
        list(APPEND GENERATED_SOURCES "wayland-${_basename}-client-protocol.c" "qwayland-${_basename}.cpp")
        list(APPEND GENERATED_HEADERS "wayland-${_basename}-client-protocol.h" "qwayland-${_basename}.h")
    endforeach()

    target_sources(${target} PRIVATE ${GENERATED_SOURCES} ${GENERATED_HEADERS})
endfunction()

add_subdirectory(client-lib)
add_subdirectory(qt-plugin)
