cmake_minimum_required(VERSION 3.24.0)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(libcontemporary)
cntp_find_pkgconfig(wayland-client wayland-client)

if(NOT ${wayland-client_FOUND})
    message(WARNING "Wayland not found. Building without Wayland support")
else()
    set(SOURCES
            waylandplugin.cpp

            Wm/waylandaccessibility.cpp
            Wm/waylandbackend.cpp
            Wm/waylandkeyboardtables.cpp
            Wm/waylandwindow.cpp
            Screens/waylandmode.cpp
            Screens/waylandscreen.cpp
            Screens/waylandscreenbackend.cpp
            Screens/waylandgammacontrol.cpp
            Screens/waylandoutput.cpp
    )

    set(HEADERS
            waylandplugin.h
            twaylandregistry.h

            Wm/waylandwmconstants.h
            Wm/waylandaccessibility.h
            Wm/waylandbackend.h
            Wm/waylandkeyboardtables.h
            Wm/waylandwindow.h
            Screens/waylandmode.h
            Screens/waylandscreen.h
            Screens/waylandscreenbackend.h
            Screens/waylandgammacontrol.h
            Screens/waylandoutput.h
    )

    set(PLUGIN_NAME wayland)
    add_plugin_tde(${PLUGIN_NAME})

target_sources(plugin-${PLUGIN_NAME} PRIVATE ${SOURCES} ${HEADERS})

    target_link_libraries(plugin-${PLUGIN_NAME} Qt::Widgets libcontemporary Qt::GuiPrivate tdesktopenvironment-wayland-layer-shell-client-lib libtdesktopenvironment PkgConfig::wayland-client)
    target_include_directories(plugin-${PLUGIN_NAME} PRIVATE ../../wayland-layer-shell/client-lib/)

    tdesktopenvironment_register_wayland_protocol_extension(plugin-${PLUGIN_NAME} FILES
            wayland-protocols/wlr-protocols/unstable/wlr-foreign-toplevel-management-unstable-v1.xml
            wayland-protocols/tdesktopenvironment-protocols/tdesktopenvironment-keygrab-v1.xml
            wayland-protocols/tdesktopenvironment-protocols/tdesktopenvironment-accessibility-v1.xml
            wayland-protocols/wlr-protocols/unstable/wlr-output-management-unstable-v1.xml
            wayland-protocols/wlr-protocols/unstable/wlr-gamma-control-unstable-v1.xml
            ${CMAKE_INSTALL_FULL_DATADIR}/wayland/wayland.xml
    )
    install(DIRECTORY wayland-protocols/tdesktopenvironment-protocols
            DESTINATION ${CMAKE_INSTALL_DATADIR}/libtdesktopenvironment/wayland-protocols
    )
endif()
