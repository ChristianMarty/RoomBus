#include "comSerial.h"

using namespace std;

serialCom::serialCom(void)
{  

}

serialCom::~serialCom()
{
    _serialPort.close();
}

bool serialCom::openConnection(QString comport)
{
    _port = "COM13";//comport;

    _serialPort.setPortName(_port);
    //_serialPort.setBaudRate(614400);
    //_serialPort.setParity(QSerialPort::Parity::NoParity);
    _serialPort.open(QIODevice::ReadWrite);

    connect(&_serialPort, &QSerialPort::readyRead, this, &serialCom::on_rx);
    connect(&_serialPort, &QSerialPort::errorOccurred, this, &serialCom::on_errorOccurred);

    if(_serialPort.isOpen()){
        emit connectionChanged(true);
    }

    return true;
}

QString serialCom::getConnectionName()
{
    return "Serial";
}

QString serialCom::getConnectionPath()
{
    return _port;
}

bool serialCom::isConnected(void)
{
    return _serialPort.isOpen();
}

void serialCom::write(QByteArray data)
{
    _serialPort.write(data,data.size());
}

void serialCom::on_rx(void)
{
    emit receive(_serialPort.readAll());
}

void serialCom::on_errorOccurred(QSerialPort::SerialPortError error)
{
    emit connectionChanged(isConnected());
}
