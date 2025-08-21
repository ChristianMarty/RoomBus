#ifndef SERIAL_CONNECTION_H
#define SERIAL_CONNECTION_H

#include <QSerialPort>
#include <QSerialPortInfo>
#include "../../QuCLib/source/CANbeSerial.h"
#include "connection.h"

class SerialConnection : public MiniBusConnection
{
    Q_OBJECT
public:
    SerialConnection(MiniBusAccess *parrent, QString port, uint32_t baud);
    ~SerialConnection();

    void open(void) override;
    void close(void) override;

    bool write(MiniBus::Message message) override;

    QString getConnectionName(void) override;
    QString getConnectionPath(void) override;

private slots:
    void on_canBeSerial_writeReady(QByteArray data);
    void on_canBeSerial_readReady(CanBusFrame frame);

    void on_serialPort_readyRead(void);
    void on_serialPort_errorOccurred(QSerialPort::SerialPortError error);

private:
    QString _port;
    uint32_t _baud;

    QSerialPort _serialPort;

    CANbeSerial _canSerial;
};

#endif // SERIAL_CONNECTION_H
