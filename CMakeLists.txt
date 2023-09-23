cmake_minimum_required(VERSION 3.22)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15)

project(libtdesktopenvironment VERSION 1.0.0 LANGUAGES C CXX)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    link_directories(/usr/local/lib)
ENDIF()

find_package(QT REQUIRED NAMES Qt6)

function(tdesktopenvironment_register_wayland_protocol_extension target)
    set(multiValueArgs FILES)
    cmake_parse_arguments(REGISTER_WAYLAND_PROTOCOL_EXTENSIONS "" "" "${multiValueArgs}" ${ARGN})

    foreach(_file ${REGISTER_WAYLAND_PROTOCOL_EXTENSIONS_FILES})
        get_filename_component(_basename ${_file} NAME_WE)

        # Wayland scanner
        add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/wayland-${_basename}-client-protocol.h"
                           COMMAND ${WAYLAND_SCANNER} client-header ${CMAKE_CURRENT_SOURCE_DIR}/${_file} "${CMAKE_CURRENT_BINARY_DIR}/wayland-${_basename}-client-protocol.h"
                           DEPENDS ${_file}
                           VERBATIM)

        add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/wayland-${_basename}-client-protocol.c"
                           COMMAND ${WAYLAND_SCANNER} public-code ${CMAKE_CURRENT_SOURCE_DIR}/${_file} "${CMAKE_CURRENT_BINARY_DIR}/wayland-${_basename}-client-protocol.c"
                           DEPENDS ${_file}
                           VERBATIM)

        # Qt Wayland scanner
        add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/qwayland-${_basename}.h"
                           COMMAND ${QT_WAYLAND_SCANNER} client-header ${CMAKE_CURRENT_SOURCE_DIR}/${_file} > "${CMAKE_CURRENT_BINARY_DIR}/qwayland-${_basename}.h"
                           DEPENDS ${_file}
                           VERBATIM)

        add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/qwayland-${_basename}.cpp"
                           COMMAND ${QT_WAYLAND_SCANNER} client-code ${CMAKE_CURRENT_SOURCE_DIR}/${_file} > "${CMAKE_CURRENT_BINARY_DIR}/qwayland-${_basename}.cpp"
                           DEPENDS ${_file}
                           VERBATIM)

        # Include in sources
        set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/qwayland-${_basename}.cpp" PROPERTY SKIP_AUTOGEN ON)
        set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/qwayland-${_basename}.h" PROPERTY SKIP_AUTOGEN ON)
        set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/wayland-${_basename}-client-protocol.c" PROPERTY SKIP_AUTOGEN ON)
        set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/wayland-${_basename}-client-protocol.h" PROPERTY SKIP_AUTOGEN ON)
        target_sources(${target} PRIVATE
            "${CMAKE_CURRENT_BINARY_DIR}/wayland-${_basename}-client-protocol.c"
            "${CMAKE_CURRENT_BINARY_DIR}/wayland-${_basename}-client-protocol.h"
            "${CMAKE_CURRENT_BINARY_DIR}/qwayland-${_basename}.h"
            "${CMAKE_CURRENT_BINARY_DIR}/qwayland-${_basename}.cpp")
    endforeach()
endfunction()

add_subdirectory(wayland-layer-shell)
add_subdirectory(lib)
