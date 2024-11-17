#ifndef UDP_CONNECTION_H
#define UDP_CONNECTION_H

#include <QUdpSocket>

#include "connection.h"

class UdpConnection : public RoomBusConnection
{
    Q_OBJECT
public:
    UdpConnection(QString ip, uint16_t port);
    ~UdpConnection();

    void open(void) override;
    void close(void) override;

    bool write(QByteArray data) override;

    QString getConnectionName(void) override;
    QString getConnectionPath(void) override;

private slots:
    void on_readyRead(void);
    void on_udpConnect(void);
    void on_udpDisconnect(void);
    void on_stateChanged(QAbstractSocket::SocketState socketState);

private:
    QUdpSocket _udpClient;
    QHostAddress _udpHost;

    QString _ip;
    uint16_t _port;
};

#endif // UDP_CONNECTION_H
