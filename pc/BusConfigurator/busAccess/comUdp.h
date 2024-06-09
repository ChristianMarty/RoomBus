#ifndef COMUDP_H
#define COMUDP_H

#include <QUdpSocket>
#include <QString>
#include <stdint.h>
#include <com.h>
#include <QObject>
#include <QByteArray>

#include "busMessage.h"

class udpCom : public busCom
{
    Q_OBJECT

public:

    udpCom(void);
    ~udpCom();

    bool openConnection(QString ip, uint16_t port);
    void write(QByteArray data) override;

    bool isConnected(void) override;
    QString getConnectionName(void) override;
    QString getConnectionPath(void) override;

private slots:
    void on_udpRx(void);
    void on_udpConnect(void);
    void on_udpDisconnect(void);
    void on_stateChanged(QAbstractSocket::SocketState socketState);

private:
    QUdpSocket _udpClient;
    QString _udpIp;
    uint16_t _udpPort;
    QHostAddress _udpHost;
};

#endif // COMUDP_H
