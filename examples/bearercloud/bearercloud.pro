HEADERS = bearercloud.h \
          cloud.h

SOURCES = main.cpp \
          bearercloud.cpp \
          cloud.cpp

RESOURCES = icons.qrc

TARGET = bearercloud

QT = core gui network svg

INCLUDEPATH += ../../src/bearer

include(../examples.pri)

qtAddLibrary(QtBearer)

CONFIG += console

include(../examples.pri)

symbian {
    BEARERLIB.sources = $$OUTPUT_DIR/build/$$SUBDIRPART/bin/QtBearer.dll
    BEARERLIB.path = .
    DEPLOYMENT += BEARERLIB
}

macx: {
    contains(QT_CONFIG,qt_framework):LIBS += -framework QtBearer
    INCLUDEPATH += ../../
    contains(CONFIG, debug) {
    }
}