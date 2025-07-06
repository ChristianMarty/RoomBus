#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include <QTcpServer>
#include <QTcpSocket>
#include "../../QuCLib/source/CANbeSerial.h"
#include "connection.h"

class TcpConnection : public RoomBusConnection
{
    Q_OBJECT
public:
    TcpConnection(QString ip, uint16_t port);
    ~TcpConnection();

    void open(void) override;
    void close(void) override;

    bool write(RoomBus::Message message) override;

    QString getConnectionName(void) override;
    QString getConnectionPath(void) override;

private slots:
    void on_canBeSerial_writeReady(QByteArray data);
    void on_canBeSerial_readReady(CanBusFrame frame);

    void on_readyRead(void);
    void on_tcpConnect(void);
    void on_tcpDisconnect(void);
    void on_stateChanged(QAbstractSocket::SocketState socketState);

private:
    QString _ip;
    uint16_t _port;

    QTcpSocket _tcpClient;

    CANbeSerial _canSerial;
};

#endif // TCP_CONNECTION_H
