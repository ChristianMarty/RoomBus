#ifndef COMSERIAL_H
#define COMSERIAL_H


#include <QSerialPort>
#include <QSerialPortInfo>

#include <QString>
#include <stdint.h>

#include <com.h>

#include <QObject>
#include <QByteArray>

#include "busMessage.h"


class serialCom : public busCom
{
    Q_OBJECT

public:

    serialCom(void);
    ~serialCom();

    bool openConnection(QString comport);
    void write(QByteArray data) override;

    bool isConnected(void) override;
    QString getConnectionName(void) override;
    QString getConnectionPath(void) override;

private slots:
    void on_rx(void);
    void on_errorOccurred(QSerialPort::SerialPortError error);

private:
    QString _port;
    QSerialPort _serialPort;
};

#endif // COMSERIAL_H
