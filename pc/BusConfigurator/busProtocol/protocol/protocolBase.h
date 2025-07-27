#ifndef BUS_PROTOCOL_H
#define BUS_PROTOCOL_H

#include "roomBusMessage.h"

class RoomBusDevice;

class ProtocolBase: public QObject
{
    Q_OBJECT
public:
    ProtocolBase(RoomBusDevice *device);
    ~ProtocolBase(void);

    virtual void handleMessage(RoomBus::Message msg)=0;

protected:
    RoomBusDevice *_device;

    void sendMessage(RoomBus::Message msg);
};
#endif // BUS_PROTOCOL_H
