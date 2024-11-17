#ifndef BUSACCESS_H
#define BUSACCESS_H

#include "busaccess_global.h"
#include <QObject>

#include "connection/connection.h"
#include "connection/serialConnection.h"
#include "connection/tcpConnection.h"
#include "connection/udpConnection.h"

#include "busMessage.h"

#include "../../QuCLib/source/CANbeSerial.h"

class BUSACCESSSHARED_EXPORT RoomBusAccess : public QObject
{
    Q_OBJECT

public:
    typedef enum {
        serial,
        tcp,
        udp
    }comType_t;

    RoomBusAccess(void);
    ~RoomBusAccess(void);

    enum class Priority {
        Reserved = 0,
        High = 1,
        Normal = 2,
        Low = 3
    };

    bool write(uint8_t destinationAddress, Protocol protocol, uint8_t command, QByteArray data);
    bool write(uint8_t destinationAddress, Protocol protocol, uint8_t command, QByteArray data, Priority priority);

    bool write(BusMessage message);
    bool write(BusMessage message, Priority priority);

    QList<BusMessage> rxMsgBuffer;

    void setPriority(uint8_t priority);
    void setPriority(Priority priority);

    void openTcpConnection(QString ip, uint16_t port);
    void openSerialConnection(QString port);
    void openUdpConnection(QString ip, uint16_t port);

    void closeConnection(void);

    bool isConnected(void);
    QString lastError(void);

    QString getConnectionName(void);
    QString getConnectionPath(void);

signals:
    void newData(void);
    void connectionChanged(void);

private slots:
    void on_writeReady(QByteArray data);
    void on_readReady(CanBusFrame frame);

    void on_receive(QByteArray data);
    void on_connectionChanged(void);

private:
    void openConnection(void);

    RoomBusConnection *_connection = nullptr;

    Priority _priority;
    uint8_t _sourceAddress;

    uint8_t _dlsCode[16] = {0,1,2,3,4,5,6,7,8,12,16,20,24,32,48,64};

    CANbeSerial _canSerial;
};

#endif // BUSACCESS_H

