QT += gui waylandclient waylandclient_private

TEMPLATE = lib
CONFIG += plugin
TARGET = tdesktopenvironment-layer-shell

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    waylandlayershellintegration.cpp \
    waylandlayershellplugin.cpp

HEADERS += \
    waylandlayershellintegration.h \
    waylandlayershellplugin.h

DISTFILES += qt-plugin.json


WAYLAND_PROTOCOL_EXTENSIONS = ../../lib/tdesktopenvironment-protocols/wlr-layer-shell-unstable-v1.xml ../../lib/wayland-protocols/wayland-protocols/stable/xdg-shell/xdg-shell.xml

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

# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_PLUGINS]/wayland-shell-integration
}
!isEmpty(target.path): INSTALLS += target

unix:!macx: LIBS += -L$$OUT_PWD/../client-lib/ -ltdesktopenvironment-wayland-layer-shell-client

INCLUDEPATH += $$PWD/../client-lib
DEPENDPATH += $$PWD/../client-lib
