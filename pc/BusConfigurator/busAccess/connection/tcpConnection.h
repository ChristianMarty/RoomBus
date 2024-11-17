#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include <QTcpServer>
#include <QTcpSocket>

#include "connection.h"

class TcpConnection : public RoomBusConnection
{
    Q_OBJECT
public:
    TcpConnection(QString ip, uint16_t port);
    ~TcpConnection();

    void open(void) override;
    void close(void) override;

    bool write(QByteArray data) override;

    QString getConnectionName(void) override;
    QString getConnectionPath(void) override;

private slots:
    void on_readyRead(void);
    void on_tcpConnect(void);
    void on_tcpDisconnect(void);
    void on_stateChanged(QAbstractSocket::SocketState socketState);

private:
    QTcpSocket _tcpClient;

    QString _ip;
    uint16_t _port;
};

#endif // TCP_CONNECTION_H
