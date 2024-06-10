#-------------------------------------------------
#
# Project created by QtCreator 2019-12-08T21:25:17
#
#-------------------------------------------------

QT       -= gui
QT       += network
TARGET = busProtocol
TEMPLATE = lib
CONFIG += staticlib

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        ../../QuCLib/source/cobs.cpp \
        ../../QuCLib/source/crc.cpp \
        ../../QuCLib/source/hexFileParser.cpp \
        busDevice.cpp \
        busMessage.cpp \
        busProtocol.cpp \
        hexFileParser_old.cpp \
        protocol/messageLogProtocol.cpp \
        protocol/protocolDecoder.cpp \
        tinyLoader.cpp \
        protocol/eventProtocol.cpp \
        protocol/fileTransferProtocol.cpp \
        protocol/serialBridgeProtocol.cpp \
        protocol/stateReportProtocol.cpp \
        protocol/triggerProtocol.cpp \
        protocol/valueReportProtocol.cpp

HEADERS += \
        ../../QuCLib/source/cobs.h \
        ../../QuCLib/source/crc.h \
        ../../QuCLib/source/hexFileParser.h \
        busDevice.h \
        busMessage.h \
        busProtocol.h \
        protocol/messageLogProtocol.h \
        protocol/protocol.h \
        protocol/protocolDecoder.h \
        tinyLoader.h \
        hexFileParser_old.h \
        protocol/eventProtocol.h \
        protocol/fileTransferProtocol.h \
        protocol/serialBridgeProtocol.h \
        protocol/stateReportProtocol.h \
        protocol/triggerProtocol.h \
        protocol/valueReportProtocol.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}
