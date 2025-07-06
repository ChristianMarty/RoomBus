QT = core
QT += network
QT += websockets

CONFIG += c++20 cmdline

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060800    # disables all the APIs deprecated before Qt 6.8.0

SOURCES += \
        main.cpp \
        roomBus/roomBus.cpp \
        unfoldedCircle/driver.cpp \
        unfoldedCircle/entity/entity.cpp \
        unfoldedCircle/entity/light.cpp \
        unfoldedCircle/entity/switch.cpp \
        unfoldedCircle/remote.cpp \
        unfoldedCircle/server.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    roomBus/roomBus.h \
    unfoldedCircle/driver.h \
    unfoldedCircle/entity/entity.h \
    unfoldedCircle/entity/light.h \
    unfoldedCircle/entity/switch.h \
    unfoldedCircle/remote.h \
    unfoldedCircle/server.h


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../BusConfigurator/build/busAccess/release/ -lbusAccess
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../BusConfigurator/build/busAccess/debug/ -lbusAccess
else:unix: LIBS += -L$$PWD/../BusConfigurator/build/busAccess/ -lbusAccess

INCLUDEPATH += $$PWD/../BusConfigurator/busAccess
DEPENDPATH += $$PWD/../BusConfigurator/busAccess
