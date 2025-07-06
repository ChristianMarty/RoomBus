#ifndef BUSACCESS_H
#define BUSACCESS_H

#include <QObject>

#include "busaccess_global.h"

#include "connection/connection.h"
#include "roomBusMessage.h"

class BUSACCESSSHARED_EXPORT RoomBusAccess : public QObject
{
    Q_OBJECT
public:
    RoomBusAccess(void);
    ~RoomBusAccess(void);

    bool write(RoomBus::Message message);

    void openSocketCanConnection(QString port);
    void openSerialConnection(QString port);
    void openTcpConnection(QString ip, uint16_t port);
    void openUdpConnection(QString ip, uint16_t port);

    void closeConnection(void);

    bool isConnected(void);
    QString lastError(void);

    QString getConnectionName(void);
    QString getConnectionPath(void);

    QList<RoomBus::Message> rxMsgBuffer;

signals:
    void newData(void);
    void connectionChanged(void);

private slots:
    void on_received(RoomBus::Message message);
    void on_connectionChanged(void);

private:
    void _openConnection(void);

    uint8_t _sourceAddress;
    RoomBusConnection *_connection = nullptr;
};

#endif // BUSACCESS_H

