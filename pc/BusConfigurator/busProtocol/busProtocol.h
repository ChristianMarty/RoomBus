#ifndef BUSPROTOCOL_H
#define BUSPROTOCOL_H

#include "roomBusMessage.h"

class RoomBusDevice;

class BusProtocol: public QObject
{
    Q_OBJECT
public:
    BusProtocol(RoomBusDevice *device);
    ~BusProtocol(void);

    virtual void pushData(RoomBus::Message msg)=0;

    virtual QList<RoomBus::Protocol> protocol(void)=0;

signals:
    void dataReady(RoomBus::Message msg);

protected:
    RoomBusDevice *_device;

    void sendMessage(RoomBus::Message msg);


};
#endif // BUSPROTOCOL_H
