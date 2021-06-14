TEMPLATE = subdirs

clientproj.subdir = client-lib

pluginproj.subdir = qt-plugin
pluginproj.depends = clientproj

SUBDIRS += \
    clientproj \
    pluginproj
