#ifndef SERIALBRIDGE_H
#define SERIALBRIDGE_H

#include <QObject>

#include <QTimer>
#include <QTcpSocket>
#include <QTcpServer>

#include "busDevice.h"

#include "protocol/serialBridgeProtocol.h"

#include "../../QuCLib/source/cobs.h"

class SerialBridge: public QObject
{
    Q_OBJECT
public:
    SerialBridge(busDevice *busDevice);

    void tcpBridgeOpen(uint16_t port);
    void tcpBridgeClose(void);

signals:
    void tcpBridgeStatus(QString message, bool open, bool error);
    void busMessage(QString message, bool error);

public slots:
    void on_receiveData(uint8_t port, serialBridgeProtocol::sbp_status_t status, QByteArray data);

    void on_tcpBridgeNewConnection(void);
    void on_tcpBridgeDataReceived(void);

private:

    QTcpServer _tcpBridgeServer;

    QList<QTcpSocket*> _tcpBridgeSocket;
    QuCLib::Cobs _tcpBridgeCobsDecoder;

    void _sendFrame(uint8_t address, uint8_t command, QByteArray data);

    serialBridgeProtocol _serialBridgeProtocol;
};

#endif // SERIALBRIDGE_H
