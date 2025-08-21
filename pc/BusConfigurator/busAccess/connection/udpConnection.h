#ifndef UDP_CONNECTION_H
#define UDP_CONNECTION_H

#include <QUdpSocket>
#include "../../QuCLib/source/CANbeSerial.h"
#include "connection.h"

class UdpConnection : public MiniBusConnection
{
    Q_OBJECT
public:
    UdpConnection(MiniBusAccess *parrent, QString ip, uint16_t port);
    ~UdpConnection();

    void open(void) override;
    void close(void) override;

    bool write(MiniBus::Message message) override;

    QString getConnectionName(void) override;
    QString getConnectionPath(void) override;

private slots:
    void on_canBeSerial_writeReady(QByteArray data);
    void on_canBeSerial_readReady(CanBusFrame frame);

    void on_readyRead(void);
    void on_udpConnect(void);
    void on_udpDisconnect(void);
    void on_stateChanged(QAbstractSocket::SocketState socketState);

private:
    QString _ip;
    uint16_t _port;

    QUdpSocket _udpClient;
    QHostAddress _udpHost;

    CANbeSerial _canSerial;
};

#endif // UDP_CONNECTION_H
