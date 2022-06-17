cmake_minimum_required(VERSION 3.4.0)

project(lib VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia Concurrent)
find_package(libcontemporary REQUIRED)
cntp_find_pkgconfig(X11 x11)
cntp_find_pkgconfig(X11_SCRNSAVER xscrnsaver)
cntp_find_pkgconfig(X11_EXT xext)
cntp_find_pkgconfig(X11_RANDR xrandr)

# TODO: Port NetworkManager portions
# TODO: Port Wayland portions
# TODO: Port pulseaudip portions

set(SOURCES
    Applications/application.cpp
    Applications/qsettingsformats.cpp
    Background/backgroundselectionmodel.cpp
    DesktopPowerProfiles/desktoppowerprofiles.cpp
    Gestures/gesturedaemon.cpp
    Gestures/gestureinteraction.cpp
    Gestures/private/gesturebackend.cpp
    MimeAssociations/mimeassociationmanager.cpp
    Screens/private/screenbackend.cpp
    Screens/screendaemon.cpp
    Screens/systemscreen.cpp
    SystemSlide/private/slidehud.cpp
    SystemSlide/private/slidempriscontroller.cpp
    SystemSlide/private/slidepulseaudiomonitor.cpp
    SystemSlide/private/slidequicksettings.cpp
    SystemSlide/systemslide.cpp
    TimeDate/desktoptimedate.cpp
    UPower/desktopupower.cpp
    UPower/desktopupowerdevice.cpp
    UPower/desktopupowerdevicesmodel.cpp
    Wm/desktopaccessibility.cpp
    Wm/desktopwm.cpp
    Wm/desktopwmwindow.cpp
    Wm/private/wmbackend.cpp
    Background/backgroundcontroller.cpp
    mpris/mprisengine.cpp
    mpris/mprisplayer.cpp
    theShellIntegration/quietmodemanager.cpp
    
    libtdesktopenvironment_resources.qrc
    
    SystemSlide/private/slidehud.ui
    SystemSlide/private/slidempriscontroller.ui
    SystemSlide/private/slidequicksettings.ui
    SystemSlide/systemslide.ui
)

set(HEADERS
    Applications/application.h
    Applications/qsettingsformats.h
    Background/backgroundselectionmodel.h
    DesktopPowerProfiles/desktoppowerprofiles.h
    Gestures/gesturedaemon.h
    Gestures/gestureinteraction.h
    Gestures/gesturetypes.h
    MimeAssociations/mimeassociationmanager.h
    Screens/screendaemon.h
    Screens/systemscreen.h
    SystemSlide/systemslide.h
    TimeDate/desktoptimedate.h
    UPower/desktopupower.h
    UPower/desktopupowerdevice.h
    UPower/desktopupowerdevicesmodel.h
    Wm/desktopaccessibility.h
    Wm/desktopwm.h
    Wm/desktopwmwindow.h
    Wm/private/wmbackend.h
    Background/backgroundcontroller.h
    libtdesktopenvironment_global.h
    mpris/mprisengine.h
    mpris/mprisplayer.h
    theShellIntegration/quietmodemanager.h
)

set(PRIVATE_HEADERS
    Gestures/private/gesturebackend.h
    Screens/private/screenbackend.h
    SystemSlide/private/slidehud.h
    SystemSlide/private/slidempriscontroller.h
    SystemSlide/private/slidepulseaudiomonitor.h
    SystemSlide/private/slidequicksettings.h
)

add_library(libtdesktopenvironment SHARED)
cntp_init(libtdesktopenvironment 17)
set_target_properties(libtdesktopenvironment PROPERTIES
        OUTPUT_NAME tdesktopenvironment
        FRAMEWORK TRUE
        MACOSX_FRAMEWORK_IDENTIFIER com.vicr123.libtdesktopenvironment
        VERSION 1.0.0
        PUBLIC_HEADER "${HEADERS}")

target_link_libraries(libtdesktopenvironment Qt6::Widgets Qt6::DBus Qt6::Multimedia Qt6::Concurrent libcontemporary)
target_compile_definitions(libtdesktopenvironment PRIVATE LIBTDESKTOPENVIRONMENT_LIBRARY)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKECONFIG_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/libtdesktopenvironment.framework/Resources/CMake)
    set(HEADER_INSTALL_DIR ${CMAKE_INSTALL_PREFIX})
    set(LIBRARY_INSTALL_DIR ../)
ELSE()
    set(CMAKECONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/libtdesktopenvironment)
    set(HEADER_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR}/libtdesktopenvironment)
    set(LIBRARY_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR})
ENDIF()

IF(${X11_FOUND})
    target_link_libraries(libtdesktopenvironment PkgConfig::X11)
    target_compile_definitions(libtdesktopenvironment PRIVATE HAVE_X11)

    set(SOURCES
        ${SOURCES}
        Wm/x11/x11accessibility.cpp
        Wm/x11/x11backend.cpp
        Wm/x11/x11functions.cpp
        Wm/x11/x11keyboardtables.cpp
        Wm/x11/x11window.cpp
        Wm/x11/x11xsettingsprovider.cpp
        Screens/x11/x11screen.cpp
        Screens/x11/x11screenbackend.cpp
        Gestures/x11/x11gesturedaemon.cpp
    )

    set(PRIVATE_HEADERS
        ${PRIVATE_HEADERS}
        Wm/x11/x11accessibility.h
        Wm/x11/x11backend.h
        Wm/x11/x11functions.h
        Wm/x11/x11keyboardtables.h
        Wm/x11/x11window.h
        Wm/x11/x11xsettingsprovider.h
        Screens/x11/x11screen.h
        Screens/x11/x11screenbackend.h
        Gestures/x11/x11gesturedaemon.h
    )
    
    IF(${X11_SCRNSAVER_FOUND})
        target_link_libraries(libtdesktopenvironment PkgConfig::X11_SCRNSAVER)
        target_compile_definitions(libtdesktopenvironment PRIVATE HAVE_XSCRNSAVER)
    ENDIF()
    
    IF(${X11_EXT_FOUND})
        target_link_libraries(libtdesktopenvironment PkgConfig::X11_EXT)
        target_compile_definitions(libtdesktopenvironment PRIVATE HAVE_XEXT)
    ENDIF()
    
    IF(${X11_RANDR_FOUND})
        target_link_libraries(libtdesktopenvironment PkgConfig::X11_RANDR)
        target_compile_definitions(libtdesktopenvironment PRIVATE HAVE_XRANDR)
    ENDIF()
ENDIF()

target_sources(libtdesktopenvironment PRIVATE ${SOURCES} ${HEADERS} ${PRIVATE_HEADERS})
cntp_target_public_headers(libtdesktopenvironment DESTINATION_DIRECTORY libtdesktopenvironment
    HEADERS ${HEADERS})

configure_package_config_file(libtdesktopenvironmentConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/libtdesktopenvironmentConfig.cmake
        INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
        PATH_VARS HEADER_INSTALL_DIR LIBRARY_INSTALL_DIR)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libtdesktopenvironmentConfig.cmake
        DESTINATION ${CMAKECONFIG_INSTALL_DIR})
install(EXPORT libtdesktopenvironmentTargets
        FILE libtdesktopenvironmentTargets.cmake
        DESTINATION ${CMAKECONFIG_INSTALL_DIR})

install(TARGETS libtdesktopenvironment
        EXPORT libtdesktopenvironmentTargets
        INCLUDES DESTINATION ${libtdesktopenvironment_INCLUDE_DIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR})

cntp_mark_support_library_directory()
