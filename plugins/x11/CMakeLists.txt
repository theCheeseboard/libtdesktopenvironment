cmake_minimum_required(VERSION 3.24.0)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(libcontemporary)
cntp_find_pkgconfig(X11 x11)
cntp_find_pkgconfig(X11_SCRNSAVER xscrnsaver)
cntp_find_pkgconfig(X11_EXT xext)
cntp_find_pkgconfig(X11_RANDR xrandr)

if(NOT ${X11_SCRNSAVER_FOUND})
    message(WARNING "X11 not found. Building without X11 support")
else()
    set(SOURCES
            x11plugin.cpp

            Wm/x11accessibility.cpp
            Wm/x11backend.cpp
            Wm/x11functions.cpp
            Wm/x11keyboardtables.cpp
            Wm/x11window.cpp
            Wm/x11xsettingsprovider.cpp

            Screens/x11screen.cpp
            Screens/x11screenbackend.cpp
            Gestures/x11gesturedaemon.cpp
    )

    set(HEADERS
            x11plugin.h

            Wm/x11accessibility.h
            Wm/x11backend.h
            Wm/x11functions.h
            Wm/x11keyboardtables.h
            Wm/x11window.h
            Wm/x11xsettingsprovider.h

            Screens/x11screen.h
            Screens/x11screenbackend.h
            Gestures/x11gesturedaemon.h
    )

    set(PLUGIN_NAME x11)
    add_plugin_tde(${PLUGIN_NAME})
    target_sources(plugin-${PLUGIN_NAME} PRIVATE ${SOURCES} ${HEADERS})

    target_link_libraries(plugin-${PLUGIN_NAME} Qt::Widgets libcontemporary libtdesktopenvironment PkgConfig::X11)

    IF(${X11_SCRNSAVER_FOUND})
        target_link_libraries(plugin-${PLUGIN_NAME} PkgConfig::X11_SCRNSAVER)
        target_compile_definitions(plugin-${PLUGIN_NAME} PRIVATE HAVE_XSCRNSAVER)
    ENDIF()

    IF(${X11_EXT_FOUND})
        target_link_libraries(plugin-${PLUGIN_NAME} PkgConfig::X11_EXT)
        target_compile_definitions(plugin-${PLUGIN_NAME} PRIVATE HAVE_XEXT)
    ENDIF()

    IF(${X11_RANDR_FOUND})
        target_link_libraries(plugin-${PLUGIN_NAME} PkgConfig::X11_RANDR)
        target_compile_definitions(plugin-${PLUGIN_NAME} PRIVATE HAVE_XRANDR)
    ENDIF()
endif()
