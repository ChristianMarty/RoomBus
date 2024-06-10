#ifndef BUSPROTOCOL_H
#define BUSPROTOCOL_H

#include "busMessage.h"
#include "protocol/protocol.h"

class busDevice;

class BusProtocol: public QObject
{
    Q_OBJECT
public:
    BusProtocol(busDevice *device);
    ~BusProtocol(void);

    virtual void pushData(busMessage msg)=0;

    virtual QList<Protocol> protocol(void)=0;

    static uint8_t getUint8(QByteArray data, uint32_t index);
    static uint16_t getUint16(QByteArray data, uint32_t index);
    static uint32_t getUint32(QByteArray data, uint32_t index);
    static float getFloat32(QByteArray data, uint32_t index);

signals:
    void dataReady(busMessage msg);

protected:
    busDevice *_device;

    void sendMessage(busMessage msg);

    QByteArray packUint32(uint32_t value);
    QByteArray packFloat32(float value);
};
#endif // BUSPROTOCOL_H
