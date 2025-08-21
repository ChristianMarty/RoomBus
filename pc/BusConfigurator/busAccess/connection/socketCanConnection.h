#ifndef SOCKETCANCONNECTION_H
#define SOCKETCANCONNECTION_H

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QCanBusDevice>

#include "connection.h"


class SocketCanConnection : public MiniBusConnection
{
    Q_OBJECT
public:
    SocketCanConnection(MiniBusAccess *parrent, QString port);
    ~SocketCanConnection();

    void open(void) override;
    void close(void) override;

    bool write(MiniBus::Message message) override;

    QString getConnectionName(void) override;
    QString getConnectionPath(void) override;

private slots:
    void on_framesReceived(void);
    void on_errorOccurred(QCanBusDevice::CanBusError);

private:
    QString _port;
    QCanBusDevice *_device = nullptr;
};

#endif // SOCKETCANCONNECTION_H
