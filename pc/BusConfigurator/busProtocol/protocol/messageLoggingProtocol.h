#ifndef MESSAGE_LOG_PROTOCOL_H
#define MESSAGE_LOG_PROTOCOL_H

#include <QObject>
#include <QMap>
#include <QDateTime>

#include "busProtocol.h"

class logMessage
{
public:
    logMessage(void){time = QDateTime::currentDateTime();}
    enum messageType_t{
        sysMessage,
        sysWarning,
        sysError,
        sysReserved,
        appMessage,
        appWarning,
        appError,
        appReserved
    }messageType;

    uint8_t messageCode;
    QString massage;
    QDateTime time;

};

class messageLoggingProtocol : public busProtocol
{
    Q_OBJECT
public:
    messageLoggingProtocol(busDevice *device);

    void pushData(busMessage msg);
    QList<Protocol> protocol(void);

    QList<logMessage> messages() const;
    void clearLog(void);

signals:
    void newMessage(logMessage msg);

private:


    QList<logMessage> _messages;
};

#endif // MESSAGE_LOG_PROTOCOL_H
