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

    virtual void pushData(RoomBus::Message msg)=0;

    virtual QList<RoomBus::Protocol> protocol(void)=0;

signals:
    void dataReady(RoomBus::Message msg);

protected:
    RoomBusDevice *_device;

    void sendMessage(RoomBus::Message msg);


};
#endif // BUS_PROTOCOL_H
