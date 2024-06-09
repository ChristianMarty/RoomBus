#ifndef COMTCP_H
#define COMTCP_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QString>
#include <stdint.h>
#include <com.h>
#include <QObject>
#include <QByteArray>

#include "busMessage.h"

class tcpCom : public busCom
{
    Q_OBJECT

public:

    tcpCom(void);
    ~tcpCom();

    bool openConnection(QString ip, uint16_t port);
    void write(QByteArray data) override;

    bool isConnected(void) override;
    QString getConnectionName(void) override;
    QString getConnectionPath(void) override;

private slots:
    void on_tcpRx(void);
    void on_tcpConnect(void);
    void on_tcpDisconnect(void);
    void on_stateChanged(QAbstractSocket::SocketState socketState);

private:
    QTcpSocket _tcpClient;
    QString _tcpIp;
    uint16_t _tcpPort;
};

#endif // COMTCP_H
