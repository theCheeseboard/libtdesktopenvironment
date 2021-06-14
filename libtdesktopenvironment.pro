TEMPLATE = subdirs


testproj.subdir = test
testproj.depends = libproj

waylandlayershellproj.subdir = wayland-layer-shell

libproj.subdir = lib
libproj.depends = waylandlayershellproj

SUBDIRS += \
    libproj \
    waylandlayershellproj
#    testproj

CONFIG += qt

QT += widgets
