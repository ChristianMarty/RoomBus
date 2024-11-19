#-------------------------------------------------
#
# Project created by QtCreator 2019-06-24T00:38:14
#
#-------------------------------------------------

QT += network
QT += serialport
QT -= gui

TARGET = busAccess
TEMPLATE = lib

DEFINES += BUSACCESS_LIBRARY

CONFIG += c++20

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060800    # disables all the APIs deprecated before Qt 6.8.0

SOURCES += \
        ../../QuCLib/source/CANbeSerial.cpp \
        ../../QuCLib/source/cobs.cpp \
        ../../QuCLib/source/crc.cpp \
        busAccess.cpp \
        connection/serialConnection.cpp \
        connection/tcpConnection.cpp \
        connection/udpConnection.cpp

HEADERS += \
        ../../QuCLib/source/CANbeSerial.h \
        ../../QuCLib/source/cobs.h \
        ../../QuCLib/source/crc.h \
        busAccess.h \
        busaccess_global.h  \
        connection/connection.h \
        connection/serialConnection.h \
        connection/tcpConnection.h \
        connection/udpConnection.h \
        roomBusMessage.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
