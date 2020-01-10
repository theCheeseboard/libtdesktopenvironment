TEMPLATE = subdirs

libproj.subdir = lib

testproj.subdir = test
testproj.depends = lib

SUBDIRS += \
    libproj \
    testproj
