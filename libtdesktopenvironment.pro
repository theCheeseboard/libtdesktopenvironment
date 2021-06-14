TEMPLATE = subdirs

libproj.subdir = lib

testproj.subdir = test
testproj.depends = libproj

SUBDIRS += \
    libproj# \
#    wayland-layer-shell
#    testproj

CONFIG += qt

QT += widgets
