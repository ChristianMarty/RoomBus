#-------------------------------------------------
#
# Project created by QtCreator 2019-12-08T18:55:33
#
#-------------------------------------------------

QT += core gui network
QT += serialport
QT += xml
QT += svg
QT += svgwidgets

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

CONFIG += c++20

SOURCES += \
        busDeviceWidget.cpp \
        busDeviceWindow.cpp \
        busMonitor/busMonitor.cpp \
        busMonitor/busMonitorLine.cpp \
        connection/connectionWidget.cpp \
        echoTestWidget.cpp \
        fileTransferWidget.cpp \
        main.cpp \
        mainwindow.cpp \
        messageLogWidget.cpp \
        qualityOfServiceWindow.cpp \
        serialBridge/serialBridgeWidget.cpp \
        settings.cpp \
        settingsWidget.cpp \
        connection/tcpConnectionWidget.cpp \
        trigger/triggerSignalLine.cpp \
        trigger/triggerSlotLine.cpp \
        trigger/triggerWidget.cpp \
        event/eventWidget.cpp \
        event/eventSignalLine.cpp \
        event/eventSlotLine.cpp \
        state/stateReportSignalLine.cpp \
        state/stateReportSlotLine.cpp \
        state/stateReportWidget.cpp \
        value/valueSignalLine.cpp \
        value/valueSlotLine.cpp \
        value/valueWidget.cpp

HEADERS += \
        busDeviceWidget.h \
        busDeviceWindow.h \
        busMonitor/busMonitor.h \
        busMonitor/busMonitorLine.h \
        connection/connectionWidget.h \
        echoTestWidget.h \
        fileTransferWidget.h \
        mainwindow.h \
        messageLogWidget.h \
        qualityOfServiceWindow.h \
        serialBridge/serialBridgeWidget.h \
        settings.h \
        settingsWidget.h \
        connection/tcpConnectionWidget.h \
        trigger/triggerSignalLine.h \
        trigger/triggerSlotLine.h \
        trigger/triggerWidget.h \
        event/eventWidget.h \
        event/eventSignalLine.h \
        event/eventSlotLine.h \
        state/stateReportSignalLine.h \
        state/stateReportSlotLine.h \
        state/stateReportWidget.h \
        value/valueSignalLine.h \
        value/valueSlotLine.h \
        value/valueWidget.h

FORMS += \
        busDeviceWidget.ui \
        busDeviceWindow.ui \
        busMonitor/busMonitor.ui \
        busMonitor/busMonitorLine.ui \
        connection/connectionWidget.ui \
        echoTestWidget.ui \
        fileTransferWidget.ui \
        mainwindow.ui \
        messageLogWidget.ui \
        qualityOfServiceWindow.ui \
        serialBridge/serialBridgeWidget.ui \
        settingsWidget.ui \
        connection/tcpConnectionWidget.ui \
        trigger/triggerSignalLine.ui \
        trigger/triggerSlotLine.ui \
        trigger/triggerWidget.ui \
        event/eventWidget.ui \
        event/eventSignalLine.ui \
        event/eventSlotLine.ui \
        state/stateReportSignalLine.ui \
        state/stateReportSlotLine.ui \
        state/stateReportWidget.ui \
        value/valueSignalLine.ui \
        value/valueSlotLine.ui \
        value/valueWidget.ui

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
