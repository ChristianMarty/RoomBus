#ifndef SERIALBRIDGEPROTOCOL_H
#define SERIALBRIDGEPROTOCOL_H


#include <QObject>
#include <QMap>
#include "busProtocol.h"

class SerialBridgeProtocol: public BusProtocol
{
    Q_OBJECT
public:
    enum sbp_status_t{
        ok,
        crcError
    };

    enum SerialBridgeType {
        SBC_Type_UART = 0x00,
        SBC_Type_I2C = 0x01
    };


    SerialBridgeProtocol(RoomBusDevice *device);

    void sendData(uint8_t port, QByteArray data);

    void pushData(RoomBus::Message msg);
    QList<RoomBus::Protocol> protocol(void);

signals:
    void receiveData(uint8_t port, sbp_status_t status, QByteArray data);


private:

    void _parseData(QByteArray data);
};

#endif // SERIALBRIDGEPROTOCOL_H
