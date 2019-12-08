QT += testlib gui widgets

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

SOURCES +=  tst_testx11backend.cpp

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../lib/release/ -ltdesktopenvironment
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../lib/debug/ -ltdesktopenvironment
else:unix: LIBS += -L$$OUT_PWD/../lib/ -ltdesktopenvironment

INCLUDEPATH += $$PWD/../lib
DEPENDPATH += $$PWD/../lib
