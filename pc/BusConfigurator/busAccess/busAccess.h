#ifndef BUS_ACCESS_H
#define BUS_ACCESS_H

#include <QObject>

#include "busaccess_global.h"
#include "miniBusMessage.h"

class MiniBusConnection;

class SerialConnection;
class SocketCanConnection;
class TcpConnection;
class UdpConnection;

class BUSACCESSSHARED_EXPORT MiniBusAccess : public QObject
{
    Q_OBJECT
public:
    MiniBusAccess(void);
    ~MiniBusAccess(void);

    enum class Type:uint8_t{
        Undefined,
        Serial,
        Tcp,
        Udp,
        SocketCan
    };

    bool setSocketCanConnection(QString port);
    bool setSerialConnection(QString port);
    bool setTcpConnection(QString ip, uint16_t port);
    bool setUdpConnection(QString ip, uint16_t port);

    void restConnection(void);

    void openConnection(void);
    void closeConnection(void);

    bool isConnected(void);
    QString lastError(void);

    QString getConnectionName(void);
    QString getConnectionPath(void);

    void setSourceAddress(uint8_t newSourceAddress);
    bool write(MiniBus::Message message, bool sourceOverride = true);

    QList<MiniBus::Message> rxMessageBuffer;

    friend SerialConnection;
    friend SocketCanConnection;
    friend TcpConnection;
    friend UdpConnection;

signals:
    void messageReceived(void);
    void connectionChanged(void);

private:
    Type _type = Type::Undefined;
    QString _url;
    uint16_t _port;

    void _openConnection(void);

    void _handleMessageReceived(const MiniBus::Message &message);

    uint8_t _sourceAddress = 0x7E; // DEC 126
    MiniBusConnection *_connection = nullptr;
};

#endif // BUS_ACCESS_H

