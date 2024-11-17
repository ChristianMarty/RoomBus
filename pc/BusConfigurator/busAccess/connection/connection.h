#ifndef CONNECTION_H
#define CONNECTION_H

#include <QObject>

class RoomBusConnection : public QObject
{
    Q_OBJECT
public:
    virtual void open(void) {};
    virtual void close(void) {};

    virtual bool connected(void) {return _isConnected;};

    virtual bool write(QByteArray data) {Q_UNUSED(data); return false;};

    virtual QString lastError(void) {return _lastErrorMessage;};

    virtual QString getConnectionName(void){return "Port Error";};
    virtual QString getConnectionPath(void){return "";};

signals:
    void receive(QByteArray data);
    void connectionChanged(void);

protected:
    bool _isConnected = false;
    QString _lastErrorMessage;
};

#endif // CONNECTION_H
