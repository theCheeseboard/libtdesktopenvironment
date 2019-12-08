QT += widgets dbus

TEMPLATE = lib
DEFINES += LIBTDESKTOPENVIRONMENT_LIBRARY
TARGET = tdesktopenvironment

CONFIG += c++11

unix {
    CONFIG += link_pkgconfig

    packagesExist(x11) {
        message("Building with X11 support");
        PKGCONFIG += x11
        DEFINES += HAVE_X11
        QT += x11extras

        SOURCES += Wm/x11/x11backend.cpp \
                   Wm/x11/x11window.cpp
        HEADERS += Wm/x11/x11backend.h \
                   Wm/x11/x11window.h \
                   Wm/x11/x11functions.h
    } else {
        message("X11 not found on this system.");
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
    UPower/desktopupower.cpp \
    UPower/desktopupowerdevice.cpp \
    UPower/desktopupowerdevicesmodel.cpp \
    Wm/desktopwm.cpp \
    Wm/desktopwmwindow.cpp \
    Wm/private/wmbackend.cpp \
    Wm/x11/x11functions.cpp

HEADERS += \
    UPower/desktopupower.h \
    UPower/desktopupowerdevice.h \
    UPower/desktopupowerdevicesmodel.h \
    Wm/desktopwm.h \
    Wm/desktopwmwindow.h \
    Wm/private/wmbackend.h \
    libtdesktopenvironment_global.h

unix {
    upowerheader.files = UPower/*.h
    upowerheader.path = /usr/include/libtdesktopenvironment/UPower
    wmheader.files = Wm/*.h
    wmheader.path = /usr/include/libtdesktopenvironment/Wm
    header.files = *.h
    header.path = /usr/include/libtdesktopenvironment

    module.files = qt_tdesktopenvironment.pri

    target.path = /usr/lib
    module.path = $$[QMAKE_MKSPECS]/modules

    INSTALLS += target upowerheader wmheader header module
}

DISTFILES += \
    qt_tdesktopenvironment.pri
