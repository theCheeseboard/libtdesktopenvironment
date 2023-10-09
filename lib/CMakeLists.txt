project(lib VERSION 1.0.0 LANGUAGES C CXX)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Multimedia Concurrent)
find_package(libcontemporary REQUIRED)

# TODO: Port NetworkManager portions
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
    Screens/private/overlaywindow.cpp Screens/private/overlaywindow.ui
    Screens/screendaemon.cpp
    Screens/systemscreen.cpp
    Screens/systempopover.cpp
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
    Greeter/passwordpane.cpp Greeter/passwordpane.ui
    Greeter/readypane.cpp Greeter/readypane.ui
    Greeter/logingreeter.cpp Greeter/logingreeter.ui
    Greeter/messagepane.cpp Greeter/messagepane.ui
    
    libtdesktopenvironment_resources.qrc
    
    SystemSlide/private/slidehud.ui
    SystemSlide/private/slidempriscontroller.ui
    SystemSlide/private/slidequicksettings.ui
    SystemSlide/systemslide.ui

    SystemJob/systemjobcontroller.cpp
    SystemJob/systemjob.cpp

    TdePlugin/tdepluginmanager.cpp
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
    Screens/systempopover.h
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
    Greeter/passwordpane.h
    Greeter/readypane.h
    Greeter/logingreeter.h
    Greeter/messagepane.h
    SystemJob/systemjobcontroller.h
    SystemJob/systemjob.h

    TdePlugin/tdeplugininterface.h
)

set(PRIVATE_HEADERS
    Gestures/private/gesturebackend.h
    Screens/private/screenbackend.h
    Screens/private/overlaywindow.h
    SystemSlide/private/slidehud.h
    SystemSlide/private/slidempriscontroller.h
    SystemSlide/private/slidepulseaudiomonitor.h
    SystemSlide/private/slidequicksettings.h
    TdePlugin/tdepluginmanager.h
)

add_library(libtdesktopenvironment SHARED)
cntp_init(libtdesktopenvironment 20)
set_target_properties(libtdesktopenvironment PROPERTIES
        OUTPUT_NAME tdesktopenvironment
        FRAMEWORK TRUE
        MACOSX_FRAMEWORK_IDENTIFIER com.vicr123.libtdesktopenvironment
        VERSION 1.0.0
        CNTP_DATA_SUBDIR libtdesktopenvironment
)

target_link_libraries(libtdesktopenvironment Qt::Widgets Qt::DBus Qt::Multimedia Qt::Concurrent libcontemporary)
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
