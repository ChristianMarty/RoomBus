#ifndef BUSACCESS_H
#define BUSACCESS_H

#include "busaccess_global.h"
#include <QObject>

#include "com.h"

#include "comTcp.h"
#include "comUdp.h"
#include "comSerial.h"

#include "../../QuCLib/source/CANbeSerial.h"

class BUSACCESSSHARED_EXPORT busAccess : public QObject
{
    Q_OBJECT

public:
    typedef enum {serial, tcp, udp} comType_t;

    busAccess(comType_t type);
    ~busAccess(void);

    enum busPriority {
        reserved = 0,
        high = 1,
        normal = 2,
        low = 3
    };

    bool write(uint8_t destAddress, Protocol protocol, uint8_t command, QByteArray data);
    bool write(uint8_t destAddress, Protocol protocol, uint8_t command, QByteArray data, busPriority priority);

    bool write(busMessage msg);
    bool write(busMessage msg, busPriority priority);

    QList<busMessage> rxMsgBuffer;

    void setPriority(uint8_t priority);
    void setPriority(busPriority priority);

    void openTcpConnection(QString ip, uint16_t port);
    void openSerialConnection(QString port);
    void openUdpConnection(QString ip, uint16_t port);
    void closeConnection(void);

    bool getIsConnected(void);

    QString getConnectionName(void);
    QString getConnectionPath(void);

signals:
    void newData(void);
    void connectionChanged(bool isConnected);


private slots:
    void on_writeReady(QByteArray data);
    void on_readReady(CanBusFrame frame);

    void on_receive(QByteArray data);
    void on_connectionChanged(bool connected);

private:
    void openConnection(void);

    busCom *_com = nullptr;
    busPriority _priority;
    uint8_t _srcAddress;

    bool isConnected;

    uint8_t _dlsCode[16] = {0,1,2,3,4,5,6,7,8,12,16,20,24,32,48,64};

    CANbeSerial _canSerial;
};

#endif // BUSACCESS_H

