#include "serialConnection.h"

using namespace std;

SerialConnection::SerialConnection(QString port, uint32_t baud)
    :_port{port}
    ,_baud{baud}
{
    _canSerial.setBaudrate(CANbeSerial::Baud500k);
    _canSerial.setDataBaudrate(CANbeSerial::Baud500k);
    _canSerial.setTxPaddingEnable(true, 0x00);

    connect(&_canSerial, &CANbeSerial::writeReady, this, &SerialConnection::on_canBeSerial_writeReady);
    connect(&_canSerial, &CANbeSerial::readReady, this, &SerialConnection::on_canBeSerial_readReady);

    connect(&_serialPort, &QSerialPort::readyRead, this, &SerialConnection::on_serialPort_readyRead);
    connect(&_serialPort, &QSerialPort::errorOccurred, this, &SerialConnection::on_serialPort_errorOccurred);
}

SerialConnection::~SerialConnection()
{
    close();
}

void SerialConnection::open(void)
{
    _serialPort.setPortName(_port);
    _serialPort.setBaudRate(_baud) ;
    _serialPort.setParity(QSerialPort::Parity::NoParity);
    _serialPort.open(QIODeviceBase::ReadWrite);

    _canSerial.setEnabled(true);

    emit connectionChanged();
}

void SerialConnection::close()
{
    _serialPort.close();
    _isConnected = false;
    emit connectionChanged();
}

QString SerialConnection::getConnectionName()
{
    return "Serial";
}

QString SerialConnection::getConnectionPath()
{
    return _port;
}

bool SerialConnection::write(RoomBus::Message message)
{
    if(!_serialPort.isOpen()){
        return false;
    }

    CanBusFrame frame;
    frame.identifier = RoomBus::toCanIdentifier(message);
    frame.extended = true;
    frame.fd = true;
    frame.data = message.data;
    frame.rtr = false;

    _canSerial.write(frame);

    return true;
}

void SerialConnection::on_canBeSerial_writeReady(QByteArray data)
{
    _serialPort.write(data,data.size());
}

void SerialConnection::on_canBeSerial_readReady(CanBusFrame frame)
{
    emit received(RoomBus::toMessage(frame.identifier, frame.data));
}

void SerialConnection::on_serialPort_readyRead(void)
{
    _canSerial.receive(_serialPort.readAll());
}

void SerialConnection::on_serialPort_errorOccurred(QSerialPort::SerialPortError error)
{
    switch(error){
        case QSerialPort::NoError: _lastErrorMessage = "No error occurred."; break;
        case QSerialPort::DeviceNotFoundError: _lastErrorMessage = "An error occurred while attempting to open an non-existing device."; break;
        case QSerialPort::PermissionError: _lastErrorMessage = "An error occurred while attempting to open an already opened device by another process or a user not having enough permission and credentials to open."; break;
        case QSerialPort::OpenError: _lastErrorMessage = "An error occurred while attempting to open an already opened device in this object."; break;
        case QSerialPort::NotOpenError: _lastErrorMessage = "This error occurs when an operation is executed that can only be successfully performed if the device is open. "; break;
        case QSerialPort::WriteError: _lastErrorMessage = "An I/O error occurred while writing the data."; break;
        case QSerialPort::ReadError: _lastErrorMessage = "An I/O error occurred while reading the data."; break;
        case QSerialPort::ResourceError: _lastErrorMessage = "The requested device operation is not supported or prohibited by the running operating system."; break;
        case QSerialPort::UnsupportedOperationError: _lastErrorMessage = ""; break;
        case QSerialPort::TimeoutError: _lastErrorMessage = "A timeout error occurred. "; break;
        case QSerialPort::UnknownError: _lastErrorMessage = "An unidentified error occurred."; break;
    }

    if(error!=QSerialPort::NoError){
        if(_serialPort.isOpen()){
            _serialPort.close();
        }
        _isConnected = false;
    }else{
        _isConnected = true;
    }

    emit connectionChanged();
}
