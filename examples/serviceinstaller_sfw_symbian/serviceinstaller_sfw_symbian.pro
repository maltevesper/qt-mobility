TEMPLATE = app
TARGET = exampleinstaller 
include(../examples.pri)
INCLUDEPATH += ../../src/serviceframework \
               ../../src/global

QT = core

SOURCES += exampleinstaller.cpp

qtAddLibrary(QtServiceFramework)

CONFIG += no_icon

symbian {
    addFiles.sources = ../filemanagerplugin/filemanagerservice.xml \ 
                       ../bluetoothtransferplugin/bluetoothtransferservice.xml 
    addFiles.path = xmldata
    DEPLOYMENT += addFiles

    load(data_caging_paths)
    pluginDep.sources = serviceframework_filemanagerplugin.dll \ 
                                            serviceframework_bluetoothtransferplugin.dll
    pluginDep.path = $$QT_PLUGINS_BASE_DIR    
    DEPLOYMENT += pluginDep

    TARGET.UID3 = 0xE00b7e42

    TARGET.CAPABILITY = ALL -TCB
}