#-------------------------------------------------
#
# Project created by QtCreator 2019-12-08T18:55:33
#
#-------------------------------------------------

QT       += core gui network
QT       += serialport
QT       += xml
QT       += svg
QT       += svgwidgets


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = busManager
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        busConnectionWidget.cpp \
        busDeviceWidget.cpp \
        busDeviceWindow.cpp \
        busMonitor/busMonitor.cpp \
        busMonitor/busMonitorLine.cpp \
        echoTestWidget.cpp \
        eventSignalLine.cpp \
        eventWidget.cpp \
        fileTransferWidget.cpp \
        main.cpp \
        mainwindow.cpp \
        messageLogWidget.cpp \
        qualityOfServiceWindow.cpp \
        settings.cpp \
        settingsWidget.cpp \
        stateReportLine.cpp \
        stateReportWidget.cpp \
        tcpConnectionWidget.cpp \
        tinyLoaderWidget.cpp \
        triggerLine.cpp \
        triggerWidget.cpp \
        valueReportLine.cpp \
        valueReportWidget.cpp

HEADERS += \
        appPluginInterface.h \
        busConnectionWidget.h \
        busDeviceWidget.h \
        busDeviceWindow.h \
        busMonitor/busMonitor.h \
        busMonitor/busMonitorLine.h \
        echoTestWidget.h \
        eventSignalLine.h \
        eventWidget.h \
        fileTransferWidget.h \
        mainwindow.h \
        messageLogWidget.h \
        qualityOfServiceWindow.h \
        settings.h \
        settingsWidget.h \
        stateReportLine.h \
        stateReportWidget.h \
        tcpConnectionWidget.h \
        tinyLoaderWidget.h \
        triggerLine.h \
        triggerWidget.h \
        valueReportLine.h \
        valueReportWidget.h

FORMS += \
        busConnectionWidget.ui \
        busDeviceWidget.ui \
        busDeviceWindow.ui \
        busMonitor/busMonitor.ui \
        busMonitor/busMonitorLine.ui \
        echoTestWidget.ui \
        eventSignalLine.ui \
        eventWidget.ui \
        fileTransferWidget.ui \
        mainwindow.ui \
        messageLogWidget.ui \
        qualityOfServiceWindow.ui \
        settingsWidget.ui \
        stateReportLine.ui \
        stateReportWidget.ui \
        tcpConnectionWidget.ui \
        tinyLoaderWidget.ui \
        triggerLine.ui \
        triggerWidget.ui \
        valueReportLine.ui \
        valueReportWidget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build/busAccess/release/ -lbusAccess
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build/busAccess/debug/ -lbusAccess
else:unix: LIBS += -L$$PWD/../busAccess/build/ -lbusAccess

INCLUDEPATH += $$PWD/../busAccess
DEPENDPATH += $$PWD/../busAccess

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build/busProtocol/release/ -lbusProtocol
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build/busProtocol/debug/ -lbusProtocol
else:unix: LIBS += -L$$PWD/../../busProtocol/build/ -lbusProtocol

INCLUDEPATH += $$PWD/../busProtocol
DEPENDPATH += $$PWD/../busProtocol
