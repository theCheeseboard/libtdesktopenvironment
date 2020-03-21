QT += widgets dbus thelib network svg

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
    Background/backgroundselectionmodel.cpp \
    SystemSlide/private/slidehud.cpp \
    SystemSlide/private/slidempriscontroller.cpp \
    SystemSlide/systemslide.cpp \
    TimeDate/desktoptimedate.cpp \
    UPower/desktopupower.cpp \
    UPower/desktopupowerdevice.cpp \
    UPower/desktopupowerdevicesmodel.cpp \
    Wm/desktopwm.cpp \
    Wm/desktopwmwindow.cpp \
    Wm/private/wmbackend.cpp \
    Wm/x11/x11functions.cpp \
    Background/backgroundcontroller.cpp \
    mpris/mprisengine.cpp \
    mpris/mprisplayer.cpp

HEADERS += \
    Background/backgroundselectionmodel.h \
    SystemSlide/private/slidehud.h \
    SystemSlide/private/slidempriscontroller.h \
    SystemSlide/systemslide.h \
    TimeDate/desktoptimedate.h \
    UPower/desktopupower.h \
    UPower/desktopupowerdevice.h \
    UPower/desktopupowerdevicesmodel.h \
    Wm/desktopwm.h \
    Wm/desktopwmwindow.h \
    Wm/private/wmbackend.h \
    Background/backgroundcontroller.h \
    libtdesktopenvironment_global.h \
    mpris/mprisengine.h \
    mpris/mprisplayer.h

unix {
    upowerheader.files = UPower/*.h
    upowerheader.path = /usr/include/libtdesktopenvironment/UPower
    wmheader.files = Wm/*.h
    wmheader.path = /usr/include/libtdesktopenvironment/Wm
    timedateheaders.files = TimeDate/*.h
    timedateheaders.path = /usr/include/libtdesktopenvironment/TimeDate
    backgroundheaders.files = Background/*.h
    backgroundheaders.path = /usr/include/libtdesktopenvironment/Background
    header.files = *.h
    header.path = /usr/include/libtdesktopenvironment

    module.files = qt_tdesktopenvironment.pri

    target.path = /usr/lib
    module.path = $$[QMAKE_MKSPECS]/modules

    INSTALLS += target upowerheader wmheader timedateheaders backgroundheaders header module
}

DISTFILES += \
    qt_tdesktopenvironment.pri

RESOURCES += \
    libtdesktopenvironment_resources.qrc

FORMS += \
    SystemSlide/private/slidehud.ui \
    SystemSlide/private/slidempriscontroller.ui \
    SystemSlide/systemslide.ui
