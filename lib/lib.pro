QT += widgets dbus thelib network svg

TEMPLATE = lib
DEFINES += LIBTDESKTOPENVIRONMENT_LIBRARY
TARGET = tdesktopenvironment
SHARE_APP_NAME=libtdesktopenvironment

CONFIG += c++11
LIBS += -lXrandr

unix {
    CONFIG += link_pkgconfig

    # Include the-libs build tools
    equals(THELIBS_BUILDTOOLS_PATH, "") {
        THELIBS_BUILDTOOLS_PATH = $$[QT_INSTALL_PREFIX]/share/the-libs/pri
    }
    include($$THELIBS_BUILDTOOLS_PATH/buildmaster.pri)

    packagesExist(x11) {
        message("Building with X11 support");
        PKGCONFIG += x11
        DEFINES += HAVE_X11
        QT += x11extras

        packagesExist(xscrnsaver) {
            PKGCONFIG += xscrnsaver
            DEFINES += HAVE_XSCRNSAVER
        } else {
            message("xscrnsaver not found on this system.");
        }

        packagesExist(xext) {
            PKGCONFIG += xext
            DEFINES += HAVE_XEXT
        } else {
            message("xext not found on this system.");
        }

        packagesExist(xrandr) {
            PKGCONFIG += xrandr
            DEFINES += HAVE_XRANDR
        } else {
            message("xrandr not found on this system.");
        }

        SOURCES += $$files(Wm/x11/*.cpp) \
                   $$files(Screens/x11/*.cpp) \
                   $$files(Gestures/x11/*.cpp)
        HEADERS += $$files(Wm/x11/*.h) \
                   $$files(Screens/x11/*.h) \
                   $$files(Gestures/x11/*.h)
    } else {
        message("X11 not found on this system.");
    }


    qtHaveModule(NetworkManagerQt) {
        QT += NetworkManagerQt
        DEFINES += HAVE_NETWORKMANAGERQT
        message("Building with NetworkManagerQt support");
    } else {
        exists($$THELIBS_INSTALL_LIB/libKF5NetworkManagerQt.so) {
            INCLUDEPATH += $$THELIBS_INSTALL_HEADERS/KF5/NetworkManagerQt/
            LIBS += -lKF5NetworkManagerQt

            DEFINES += HAVE_NETWORKMANAGERQT
            message("Building with NetworkManagerQt support");
        } else {
            message("NetworkManagerQt not found on this system.");
        }
    }

    packagesExist(wayland-client) {
        message("Building with Wayland support.");
        DEFINES += HAVE_WAYLAND

        QT += gui-private

        PKGCONFIG += wayland-client

        SOURCES += $$files(Wm/wayland/*.cpp) \
                   $$files(Screens/wayland/*.cpp)
        HEADERS += $$files(Wm/wayland/*.h) \
                   $$files(Screens/wayland/*.h)


        WAYLAND_PROTOCOL_EXTENSIONS = wayland-protocols/wlr-protocols/unstable/wlr-foreign-toplevel-management-unstable-v1.xml

        wayland_scanner_headers.output = wayland-${QMAKE_FILE_BASE}-client-protocol.h
        wayland_scanner_headers.commands = wayland-scanner client-header ${QMAKE_FILE_NAME} ${QMAKE_FILE_OUT}
        wayland_scanner_headers.input = WAYLAND_PROTOCOL_EXTENSIONS
        wayland_scanner_headers.CONFIG += target_predeps no_link

        wayland_scanner_sources.output = wayland-${QMAKE_FILE_BASE}-client-protocol.c
        wayland_scanner_sources.commands = wayland-scanner private-code ${QMAKE_FILE_NAME} ${QMAKE_FILE_OUT}
        wayland_scanner_sources.input = WAYLAND_PROTOCOL_EXTENSIONS
        wayland_scanner_sources.variable_out = SOURCES
        wayland_scanner_headers.CONFIG += target_predeps no_link

        qwayland_scanner_headers.output = qwayland-${QMAKE_FILE_BASE}.h
        qwayland_scanner_headers.commands = qtwaylandscanner client-header ${QMAKE_FILE_NAME} > ${QMAKE_FILE_OUT}
        qwayland_scanner_headers.input = WAYLAND_PROTOCOL_EXTENSIONS
        qwayland_scanner_headers.CONFIG += target_predeps no_link

        qwayland_scanner_sources.output = qwayland-${QMAKE_FILE_BASE}.cpp
        qwayland_scanner_sources.commands = qtwaylandscanner client-code ${QMAKE_FILE_NAME} > ${QMAKE_FILE_OUT}
        qwayland_scanner_sources.input = WAYLAND_PROTOCOL_EXTENSIONS
        qwayland_scanner_sources.variable_out = SOURCES
        qwayland_scanner_headers.CONFIG += target_predeps no_link

        QMAKE_EXTRA_COMPILERS += wayland_scanner_headers wayland_scanner_sources qwayland_scanner_headers qwayland_scanner_sources

        unix:!macx: LIBS += -L$$OUT_PWD/../wayland-layer-shell/client-lib/ -ltdesktopenvironment-wayland-layer-shell-client

        INCLUDEPATH += $$PWD/../wayland-layer-shell/client-lib
        DEPENDPATH += $$PWD/../wayland-layer-shell/client-lib
    } else {
        message("layer-shell-qt or wayland-client not found on this system.");
    }

    exists($$THELIBS_INSTALL_LIB/libKF5PulseAudioQt.so) : packagesExist(libpulse) : packagesExist(libpulse-mainloop-glib) {
        PKGCONFIG += libpulse libpulse-mainloop-glib
        LIBS += -lKF5PulseAudioQt
        INCLUDEPATH += $$THELIBS_INSTALL_HEADERS/../KF5/KF5PulseAudioQt/PulseAudioQt

        DEFINES += HAVE_PULSE
        message("Building with pulseaudio support");
    } else {
        message("PulseAudio or PulseAudioQt not found on this system.");
    }
}


# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Applications/application.cpp \
    Applications/qsettingsformats.cpp \
    Background/backgroundselectionmodel.cpp \
    Gestures/gesturedaemon.cpp \
    Gestures/gestureinteraction.cpp \
    Gestures/private/gesturebackend.cpp \
    MimeAssociations/mimeassociationmanager.cpp \
    Screens/private/screenbackend.cpp \
    Screens/screendaemon.cpp \
    Screens/systemscreen.cpp \
    SystemSlide/private/slidehud.cpp \
    SystemSlide/private/slidempriscontroller.cpp \
    SystemSlide/private/slidepulseaudiomonitor.cpp \
    SystemSlide/private/slidequicksettings.cpp \
    SystemSlide/systemslide.cpp \
    TimeDate/desktoptimedate.cpp \
    UPower/desktopupower.cpp \
    UPower/desktopupowerdevice.cpp \
    UPower/desktopupowerdevicesmodel.cpp \
    Wm/desktopaccessibility.cpp \
    Wm/desktopwm.cpp \
    Wm/desktopwmwindow.cpp \
    Wm/private/wmbackend.cpp \
    Background/backgroundcontroller.cpp \
    mpris/mprisengine.cpp \
    mpris/mprisplayer.cpp \
    theShellIntegration/quietmodemanager.cpp

HEADERS += \
    Applications/application.h \
    Applications/qsettingsformats.h \
    Background/backgroundselectionmodel.h \
    Gestures/gesturedaemon.h \
    Gestures/gestureinteraction.h \
    Gestures/gesturetypes.h \
    Gestures/private/gesturebackend.h \
    MimeAssociations/mimeassociationmanager.h \
    Screens/private/screenbackend.h \
    Screens/screendaemon.h \
    Screens/systemscreen.h \
    SystemSlide/private/slidehud.h \
    SystemSlide/private/slidempriscontroller.h \
    SystemSlide/private/slidepulseaudiomonitor.h \
    SystemSlide/private/slidequicksettings.h \
    SystemSlide/systemslide.h \
    TimeDate/desktoptimedate.h \
    UPower/desktopupower.h \
    UPower/desktopupowerdevice.h \
    UPower/desktopupowerdevicesmodel.h \
    Wm/desktopaccessibility.h \
    Wm/desktopwm.h \
    Wm/desktopwmwindow.h \
    Wm/private/wmbackend.h \
    Background/backgroundcontroller.h \
    libtdesktopenvironment_global.h \
    mpris/mprisengine.h \
    mpris/mprisplayer.h \
    theShellIntegration/quietmodemanager.h

unix {
    upowerheader.files = UPower/*.h
    upowerheader.path = $$THELIBS_INSTALL_HEADERS/libtdesktopenvironment/UPower
    wmheader.files = Wm/*.h
    wmheader.path = $$THELIBS_INSTALL_HEADERS/libtdesktopenvironment/Wm
    timedateheaders.files = TimeDate/*.h
    timedateheaders.path = $$THELIBS_INSTALL_HEADERS/libtdesktopenvironment/TimeDate
    backgroundheaders.files = Background/*.h
    backgroundheaders.path = $$THELIBS_INSTALL_HEADERS/libtdesktopenvironment/Background
    slideheaders.files = SystemSlide/*.h
    slideheaders.path = $$THELIBS_INSTALL_HEADERS/libtdesktopenvironment/SystemSlide
    tsiheaders.files = theShellIntegration/*.h
    tsiheaders.path = $$THELIBS_INSTALL_HEADERS/libtdesktopenvironment/theShellIntegration
    screenheaders.files = Screens/*.h
    screenheaders.path = $$THELIBS_INSTALL_HEADERS/libtdesktopenvironment/Screens
    applicationsheaders.files = Applications/*.h
    applicationsheaders.path = $$THELIBS_INSTALL_HEADERS/libtdesktopenvironment/Applications
    mprisheaders.files = mpris/*.h
    mprisheaders.path = $$THELIBS_INSTALL_HEADERS/libtdesktopenvironment/mpris
    mimemanagerheaders.files = MimeAssociations/*.h
    mimemanagerheaders.path = $$THELIBS_INSTALL_HEADERS/libtdesktopenvironment/MimeAssociations
    gestureheaders.files = Gestures/*.h
    gestureheaders.path = $$THELIBS_INSTALL_HEADERS/libtdesktopenvironment/Gestures
    header.files = *.h
    header.path = $$THELIBS_INSTALL_HEADERS/libtdesktopenvironment

    module.files = qt_tdesktopenvironment.pri

    target.path = $$THELIBS_INSTALL_LIB
    module.path = $$THELIBS_INSTALL_MODULES

    INSTALLS += target upowerheader wmheader timedateheaders backgroundheaders slideheaders tsiheaders screenheaders applicationsheaders header module mprisheaders mimemanagerheaders gestureheaders
}

DISTFILES += \
    qt_tdesktopenvironment.pri

RESOURCES += \
    libtdesktopenvironment_resources.qrc

FORMS += \
    SystemSlide/private/slidehud.ui \
    SystemSlide/private/slidempriscontroller.ui \
    SystemSlide/private/slidequicksettings.ui \
    SystemSlide/systemslide.ui
