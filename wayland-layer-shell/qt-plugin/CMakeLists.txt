cmake_minimum_required(VERSION 3.20.0)

project(theme VERSION 1.0.0 LANGUAGES C CXX)

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS WaylandClient)
find_package(libcontemporary REQUIRED)

find_program(WAYLAND_SCANNER wayland-scanner)
if(NOT WAYLAND_SCANNER)
    message(FATAL_ERROR "wayland-scanner not found!")
endif()
message(STATUS "Found wayland-scanner: ${WAYLAND_SCANNER}")

find_program(QT_WAYLAND_SCANNER qtwaylandscanner)
if(NOT QT_WAYLAND_SCANNER)
    message(FATAL_ERROR "qtwaylandscanner not found!")
endif()
message(STATUS "Found qtwaylandscanner: ${QT_WAYLAND_SCANNER}")

cntp_define_build_dirs()

cntp_sourcelist(
    OUTPUT SOURCES
    OUTPUT_ALL ALL_SOURCES
    FILES
        waylandlayershellintegration.cpp
        waylandlayershellplugin.cpp
    )

cntp_sourcelist(
    OUTPUT HEADERS
    OUTPUT_ALL ALL_HEADERS
    FILES
        waylandlayershellintegration.h
        waylandlayershellplugin.h
    )

add_library(tdesktopenvironment-wayland-shell-qt-plugin SHARED ${SOURCES} ${HEADERS})
cntp_init(tdesktopenvironment-wayland-shell-qt-plugin 20)
tdesktopenvironment_register_wayland_protocol_extension(tdesktopenvironment-wayland-shell-qt-plugin FILES
    ../../lib/wayland-protocols/tdesktopenvironment-protocols/wlr-layer-shell-unstable-v1.xml
    ../../lib/wayland-protocols/wayland-protocols/stable/xdg-shell/xdg-shell.xml
)
set_target_properties(tdesktopenvironment-wayland-shell-qt-plugin PROPERTIES
        PREFIX ""
        OUTPUT_NAME "tdesktopenvironment-layer-shell")

target_compile_definitions(tdesktopenvironment-wayland-shell-qt-plugin PRIVATE CONTEMPORARY_LIBRARY)

target_link_libraries(tdesktopenvironment-wayland-shell-qt-plugin Qt::WaylandClient Qt::WaylandClientPrivate libcontemporary tdesktopenvironment-wayland-layer-shell-client-lib)
target_include_directories(tdesktopenvironment-wayland-shell-qt-plugin PUBLIC ../client-lib/)

install(TARGETS tdesktopenvironment-wayland-shell-qt-plugin
        LIBRARY DESTINATION ${CNTP_INSTALL_PLUGINS}/wayland-shell-integration/
    )
