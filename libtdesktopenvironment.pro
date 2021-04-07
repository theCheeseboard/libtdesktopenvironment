TEMPLATE = subdirs

libproj.subdir = lib

testproj.subdir = test
testproj.depends = libproj

SUBDIRS += \
    libproj
#    testproj

CONFIG += qt

QT += widgets
