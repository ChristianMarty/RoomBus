#-------------------------------------------------
#
# Project created by QtCreator 2019-06-24T00:38:14
#
#-------------------------------------------------

QT       += network
QT       += serialport
QT       -= gui

TARGET = busAccess
TEMPLATE = lib

DEFINES += BUSACCESS_LIBRARY

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
        ../../QuCLib/source/CANbeSerial.cpp \
        ../../QuCLib/source/cobs.cpp \
        ../../QuCLib/source/crc.cpp \
        busAccess.cpp \
        com.cpp \
        comSerial.cpp \
        comTcp.cpp \
        comUdp.cpp

HEADERS += \
        ../../QuCLib/source/CANbeSerial.h \
        ../../QuCLib/source/cobs.h \
        ../../QuCLib/source/crc.h \
        busAccess.h \        busaccess_global.h  \
        com.h \
        comSerial.h \
        comTcp.h \
        comUdp.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build/busProtocol/release/ -lbusProtocol
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build/busProtocol/debug/ -lbusProtocol
else:unix: LIBS += -L$$PWD/build/busProtocol/ -lbusProtocol

INCLUDEPATH += $$PWD/../busProtocol
DEPENDPATH += $$PWD/../busProtocol
