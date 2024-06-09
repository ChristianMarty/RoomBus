#ifndef COM_H
#define COM_H

#include <QString>
#include <stdint.h>

#include <QObject>
#include <QByteArray>

class busCom : public QObject
{
    Q_OBJECT

public:

    busCom(void);
    ~busCom();

    virtual void write(QByteArray data) {};

    virtual bool isConnected(void) {return false;};

    virtual QString getConnectionName(void){return "Port Error";};
    virtual QString getConnectionPath(void){return "";};

signals:
    void receive(QByteArray data);

    void rxError(uint32_t errorCounter);
    void connectionChanged(bool connected);
};

#endif // COM_H
