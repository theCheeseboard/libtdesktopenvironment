TEMPLATE = subdirs

libproj.subdir = lib

testproj.subdir = test
testproj.depends = libproj

SUBDIRS += \
    libproj \
    testproj

HEADERS += \
    ../theShell/theshell-lib/server/serverdaemon.h

SOURCES += \
    ../theShell/theshell-lib/server/serverdaemon.cpp
