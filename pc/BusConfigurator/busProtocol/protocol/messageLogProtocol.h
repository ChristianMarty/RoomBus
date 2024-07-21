#ifndef MESSAGE_LOG_PROTOCOL_H
#define MESSAGE_LOG_PROTOCOL_H

#include <QObject>
#include <QMap>
#include <QDateTime>

#include "busProtocol.h"

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

class MessageLogProtocol : public BusProtocol
{
    Q_OBJECT
public:
    MessageLogProtocol(busDevice *device);

    void pushData(busMessage msg);
    QList<Protocol> protocol(void);

    QList<LogMessage> messages() const;
    void clearLog(void);

signals:
    void newMessage(LogMessage msg);

private:
    QList<LogMessage> _messages;
};

#endif // MESSAGE_LOG_PROTOCOL_H
