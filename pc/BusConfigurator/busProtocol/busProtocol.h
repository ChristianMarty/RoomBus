#ifndef BUSPROTOCOL_H
#define BUSPROTOCOL_H

#include "roomBusMessage.h"

class busDevice;

class BusProtocol: public QObject
{
    Q_OBJECT
public:
    BusProtocol(busDevice *device);
    ~BusProtocol(void);

    virtual void pushData(RoomBus::Message msg)=0;

    virtual QList<RoomBus::Protocol> protocol(void)=0;

    static uint8_t getUint8(QByteArray data, uint32_t index);
    static uint16_t getUint16(QByteArray data, uint32_t index);
    static uint32_t getUint32(QByteArray data, uint32_t index);
    static float getFloat32(QByteArray data, uint32_t index);

signals:
    void dataReady(RoomBus::Message msg);

protected:
    busDevice *_device;

    void sendMessage(RoomBus::Message msg);

    QByteArray packUint16(uint16_t value);
    QByteArray packUint32(uint32_t value);
    QByteArray packFloat32(float value);
};
#endif // BUSPROTOCOL_H
