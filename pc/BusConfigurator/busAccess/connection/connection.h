#ifndef CONNECTION_H
#define CONNECTION_H

#include <QObject>
#include "miniBusMessage.h"

class MiniBusAccess;
class MiniBusConnection : public QObject
{
    Q_OBJECT
public:
    virtual void open(void) {};
    virtual void close(void) {};

    virtual bool connected(void) {return _isConnected;};

    virtual bool write(MiniBus::Message message) {Q_UNUSED(message); return false;};

    virtual QString lastError(void) {return _lastErrorMessage;};

    virtual QString getConnectionName(void){return "Port Error";};
    virtual QString getConnectionPath(void){return "";};

protected:
    bool _isConnected = false;
    QString _lastErrorMessage;

    MiniBusAccess *_parrent = nullptr;
};

#endif // CONNECTION_H
