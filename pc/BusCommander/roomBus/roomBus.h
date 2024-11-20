#ifndef ROOM_BUS_INTERFACE_H
#define ROOM_BUS_INTERFACE_H

#include <QObject>
#include "busAccess.h"

class RoomBusInterface : public QObject
{
    Q_OBJECT
public:
    explicit RoomBusInterface(QObject *parent = nullptr);
    ~RoomBusInterface(void);

    void sendTrigger(uint16_t triggerChannel);

    RoomBusAccess *busConnection(void);

private:
    RoomBusAccess _busConnection;
    uint8_t _sourceAddress = 0x7E;
};

#endif // ROOM_BUS_INTERFACE_H
