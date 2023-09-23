project(lib VERSION 1.0.0 LANGUAGES C CXX)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS WaylandClient)
find_package(libcontemporary REQUIRED)

cntp_define_build_dirs()

cntp_sourcelist(
    OUTPUT HEADERS
    OUTPUT_ALL ALL_HEADERS
    FILES
        layershellwindow.cpp
        private/layershellshell.cpp
        private/layershellsurface.cpp
    )

cntp_sourcelist(
    OUTPUT HEADERS
    OUTPUT_ALL ALL_HEADERS
    FILES
        client-lib_global.h
        layershellwindow.h
        private/layershellshell.h
        private/layershellsurface.h
    )

set(PRIVATE_HEADERS

)

add_library(tdesktopenvironment-wayland-layer-shell-client-lib SHARED ${SOURCES} ${HEADERS} ${PRIVATE_HEADERS})
cntp_init(tdesktopenvironment-wayland-layer-shell-client-lib 20)
tdesktopenvironment_register_wayland_protocol_extension(tdesktopenvironment-wayland-layer-shell-client-lib FILES
    ../../lib/wayland-protocols/tdesktopenvironment-protocols/wlr-layer-shell-unstable-v1.xml
    ../../lib/wayland-protocols/wayland-protocols/stable/xdg-shell/xdg-shell.xml
)
set_target_properties(tdesktopenvironment-wayland-layer-shell-client-lib PROPERTIES
        OUTPUT_NAME tdesktopenvironment-wayland-layer-shell-client
        VERSION 1.0.0
        PUBLIC_HEADER "${HEADERS}"
        TRANSLATION_SOURCES "${ALL_SOURCES};${ALL_HEADERS}"
        CNTP_DATA_SUBDIR libtdesktopenvironment/layer-shell-lib)
cntp_translate(tdesktopenvironment-wayland-layer-shell-client-lib)

target_link_libraries(tdesktopenvironment-wayland-layer-shell-client-lib Qt::Widgets Qt::WaylandClient Qt::WaylandClientPrivate libcontemporary)
target_compile_definitions(tdesktopenvironment-wayland-layer-shell-client-lib PRIVATE CLIENTLIB_LIBRARY)

set(CMAKECONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/tdesktopenvironment-wayland-layer-shell-client-lib)
set(HEADER_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR}/libtdesktopenvironment/wayland-layer-shell-client-lib)
set(LIBRARY_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR})

configure_package_config_file(tdesktopenvironment-wayland-layer-shell-client-libConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/tdesktopenvironment-wayland-layer-shell-client-libConfig.cmake
        INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
        PATH_VARS HEADER_INSTALL_DIR LIBRARY_INSTALL_DIR)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tdesktopenvironment-wayland-layer-shell-client-libConfig.cmake
        DESTINATION ${CMAKECONFIG_INSTALL_DIR})
install(EXPORT tdesktopenvironment-wayland-layer-shell-client-libTargets
        FILE tdesktopenvironment-wayland-layer-shell-client-libTargets.cmake
        DESTINATION ${CMAKECONFIG_INSTALL_DIR})

install(TARGETS tdesktopenvironment-wayland-layer-shell-client-lib
        EXPORT tdesktopenvironment-wayland-layer-shell-client-libTargets
        PUBLIC_HEADER DESTINATION ${HEADER_INSTALL_DIR}
        INCLUDES DESTINATION ${INCLUDE_INSTALL_DIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR})
