#ifndef CONNECTION_H
#define CONNECTION_H

#include <QObject>
#include "roomBusMessage.h"

class RoomBusConnection : public QObject
{
    Q_OBJECT
public:
    virtual void open(void) {};
    virtual void close(void) {};

    virtual bool connected(void) {return _isConnected;};

    virtual bool write(RoomBus::Message message) {Q_UNUSED(message); return false;};

    virtual QString lastError(void) {return _lastErrorMessage;};

    virtual QString getConnectionName(void){return "Port Error";};
    virtual QString getConnectionPath(void){return "";};

signals:
    void received(RoomBus::Message message);
    void connectionChanged(void);

protected:
    bool _isConnected = false;
    QString _lastErrorMessage;
};

#endif // CONNECTION_H
