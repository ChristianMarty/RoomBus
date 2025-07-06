#ifndef ROOM_BUS_INTERFACE_H
#define ROOM_BUS_INTERFACE_H

#include <QObject>
#include "busAccess.h"

class RoomBusInterface : public QObject
{
    Q_OBJECT
public:

    enum class Type{
        Undefined,
        Serial,
        Can
    };

    explicit RoomBusInterface(Type type, QString port, QObject *parent = nullptr);
    ~RoomBusInterface(void);

    void sendTrigger(uint16_t triggerChannel);
    void sendValue(uint16_t valueChannel, uint32_t value);

    RoomBusAccess *busConnection(void);

private:
    RoomBusAccess _busConnection;
    uint8_t _sourceAddress = 0x7E;
    Type _type = Type::Undefined;
};

#endif // ROOM_BUS_INTERFACE_H
