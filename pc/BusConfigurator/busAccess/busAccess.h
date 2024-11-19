#ifndef BUSACCESS_H
#define BUSACCESS_H

#include <QObject>

#include "busaccess_global.h"

#include "../../QuCLib/source/CANbeSerial.h"
#include "connection/connection.h"
#include "roomBusMessage.h"

class BUSACCESSSHARED_EXPORT RoomBusAccess : public QObject
{
    Q_OBJECT
public:
    RoomBusAccess(void);
    ~RoomBusAccess(void);

    enum class Priority {
        Reserved = 0,
        High = 1,
        Normal = 2,
        Low = 3
    };

    bool write(RoomBus::Message message);
    bool write(RoomBus::Message message, Priority priority);

    void setDefaultPriority(Priority priority);

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
    void on_writeReady(QByteArray data);
    void on_readReady(CanBusFrame frame);

    void on_receive(QByteArray data);
    void on_connectionChanged(void);

private:
    void _openConnection(void);

    Priority _priority = Priority::Normal;
    uint8_t _sourceAddress;

    RoomBusConnection *_connection = nullptr;
    CANbeSerial _canSerial;

    uint8_t _dlsCode[16] = {0,1,2,3,4,5,6,7,8,12,16,20,24,32,48,64};
};

#endif // BUSACCESS_H

