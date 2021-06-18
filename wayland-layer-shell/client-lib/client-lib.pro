QT -= gui
QT += waylandclient waylandclient_private
SHARE_APP_NAME=libtdesktopenvironment/layer-shell-lib

TEMPLATE = lib
DEFINES += CLIENTLIB_LIBRARY
TARGET = tdesktopenvironment-wayland-layer-shell-client

CONFIG += c++11 link_pkgconfig
PKGCONFIG += wayland-client

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    layershellwindow.cpp \
    private/layershellshell.cpp \
    private/layershellsurface.cpp

HEADERS += \
    client-lib_global.h \
    layershellwindow.h \
    private/layershellshell.h \
    private/layershellsurface.h

# Default rules for deployment.
unix {
    equals(THELIBS_BUILDTOOLS_PATH, "") {
        THELIBS_BUILDTOOLS_PATH = $$[QT_INSTALL_PREFIX]/share/the-libs/pri
    }
    include($$THELIBS_BUILDTOOLS_PATH/buildmaster.pri)

    target.path = $$THELIBS_INSTALL_LIB
}
!isEmpty(target.path): INSTALLS += target

qtPrepareTool(QTWAYLANDSCANNER, qtwaylandscanner)

WAYLAND_PROTOCOL_EXTENSIONS = ../../lib/wayland-protocols/tdesktopenvironment-protocols/wlr-layer-shell-unstable-v1.xml ../../lib/wayland-protocols/wayland-protocols/stable/xdg-shell/xdg-shell.xml

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
qwayland_scanner_headers.commands = $$QTWAYLANDSCANNER client-header ${QMAKE_FILE_NAME} > ${QMAKE_FILE_OUT}
qwayland_scanner_headers.input = WAYLAND_PROTOCOL_EXTENSIONS
qwayland_scanner_headers.CONFIG += target_predeps no_link

qwayland_scanner_sources.output = qwayland-${QMAKE_FILE_BASE}.cpp
qwayland_scanner_sources.commands = $$QTWAYLANDSCANNER client-code ${QMAKE_FILE_NAME} > ${QMAKE_FILE_OUT}
qwayland_scanner_sources.input = WAYLAND_PROTOCOL_EXTENSIONS
qwayland_scanner_sources.variable_out = SOURCES
qwayland_scanner_headers.CONFIG += target_predeps no_link

QMAKE_EXTRA_COMPILERS += wayland_scanner_headers wayland_scanner_sources qwayland_scanner_headers qwayland_scanner_sources
