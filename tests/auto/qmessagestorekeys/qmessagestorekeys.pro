TEMPLATE = app
TARGET = tst_qmessagestorekeys

CONFIG += testcase
QT += testlib

include(../../../common.pri)
include(../support/support.pri)

qtAddLibrary(QtMessaging)
INCLUDEPATH += ../../../src/messaging

symbian|win32 {
} else {
# Temporarily link against local qtopiamail lib (should be part of the platform)
LIBS += -L $$(QMF_LIBDIR) -lqtopiamail
}

wince*|symbian*: {
    addFiles.sources = testdata/*
    addFiles.path = testdata
    DEPLOYMENT += addFiles
}

wince* {
    DEFINES += TEADATA_DIR=\\\".\\\"
} !symbian {
    DEFINES += TESTDATA_DIR=\\\"$$PWD/\\\"
}

SOURCES += \
    tst_qmessagestorekeys.cpp
