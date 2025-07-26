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

    enum class Type:uint8_t{
        Undefined,
        Serial,
        Tcp,
        Udp,
        SocketCan
    };

    void setSocketCanConnection(QString port);
    void setSerialConnection(QString port);
    void setTcpConnection(QString ip, uint16_t port);
    void setUdpConnection(QString ip, uint16_t port);

    void openConnection(void);
    void closeConnection(void);

    bool isConnected(void);
    QString lastError(void);

    QString getConnectionName(void);
    QString getConnectionPath(void);

    bool write(RoomBus::Message message);

    QList<RoomBus::Message> rxMsgBuffer;

signals:
    void newData(void);
    void connectionChanged(void);

private slots:
    void on_received(RoomBus::Message message);
    void on_connectionChanged(void);

private:
    Type _type = Type::Undefined;
    QString _url;
    uint16_t _port;

    void _openConnection(void);

    uint8_t _sourceAddress = 0x7E; // DEC 126
    RoomBusConnection *_connection = nullptr;
};

#endif // BUSACCESS_H

