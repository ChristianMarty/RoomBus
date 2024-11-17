#ifndef SERIALBRIDGEPROTOCOL_H
#define SERIALBRIDGEPROTOCOL_H


#include <QObject>
#include <QMap>
#include "busProtocol.h"

class serialBridgeProtocol: public BusProtocol
{
    Q_OBJECT
public:
    enum commands_t{
        data  = 0,
        portInfoReport = 4,
        portInfoRequest = 5
    };

    enum sbp_status_t{
        ok,
        crcError
    };

    serialBridgeProtocol(busDevice *device);

    void sendData(uint8_t port, QByteArray data);

    void pushData(BusMessage msg);
    QList<Protocol> protocol(void);

signals:
    void receiveData(uint8_t port, sbp_status_t status, QByteArray data);


private:

    void _parseData(QByteArray data);
};

#endif // SERIALBRIDGEPROTOCOL_H
