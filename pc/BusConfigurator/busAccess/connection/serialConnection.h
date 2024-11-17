#ifndef SERIAL_CONNECTION_H
#define SERIAL_CONNECTION_H

#include <QSerialPort>
#include <QSerialPortInfo>

#include "connection.h"

class SerialConnection : public RoomBusConnection
{
    Q_OBJECT
public:
    SerialConnection(QString port, uint32_t baud);
    ~SerialConnection();

    void open(void) override;
    void close(void) override;

    bool write(QByteArray data) override;

    QString getConnectionName(void) override;
    QString getConnectionPath(void) override;

private slots:
    void on_readyRead(void);
    void on_errorOccurred(QSerialPort::SerialPortError error);

private:
    QString _port;
    QSerialPort _serialPort;
    uint32_t _baud;
};

#endif // SERIAL_CONNECTION_H
