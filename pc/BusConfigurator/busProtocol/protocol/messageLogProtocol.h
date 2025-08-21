#ifndef MESSAGE_LOG_PROTOCOL_H
#define MESSAGE_LOG_PROTOCOL_H

#include <QObject>
#include <QMap>
#include <QDateTime>

#include "protocolBase.h"

class LogMessage
{
public:

    LogMessage(void){receiveTime = QDateTime::currentDateTime();}

    enum MessageType:uint8_t {
        sysMessage,
        sysWarning,
        sysError,
        sysReserved,
        appMessage,
        appWarning,
        appError,
        appReserved
    } messageType;

    uint32_t messageCode;
    QString massage;
    uint32_t tickTime;
    QDateTime receiveTime;
};

class MessageLogProtocol : public ProtocolBase
{
    Q_OBJECT
public:

    enum class Command:MiniBus::Command {
        SystemMessage = 0x00,
        SystemWarning = 0x01,
        SystemError = 0x02,
        Reserved0,

        ApplicationMessage = 0x04,
        ApplicationWarning = 0x05,
        ApplicationError = 0x06,
        Reserved1
    };

    MessageLogProtocol(RoomBusDevice *device);

    void handleMessage(MiniBus::Message msg);

    QList<LogMessage> messages() const;
    void clearLog(void);

    static QString commandName(MiniBus::Command command);
    static QString dataDecoder(MiniBus::Command command, const QByteArray &data);

signals:
    void newMessage(LogMessage msg);

private:
    QList<LogMessage> _messages;
};

#endif // MESSAGE_LOG_PROTOCOL_H
