#include "tcpConnection.h"

using namespace std;

TcpConnection::TcpConnection(QString ip, uint16_t port)
    :_ip{ip}
    ,_port{port}
{
    _canSerial.setBaudrate(CANbeSerial::Baud500k);
    _canSerial.setDataBaudrate(CANbeSerial::Baud500k);
    _canSerial.setTxPaddingEnable(true, 0x00);

    connect(&_canSerial, &CANbeSerial::writeReady, this, &TcpConnection::on_canBeSerial_writeReady);
    connect(&_canSerial, &CANbeSerial::readReady, this, &TcpConnection::on_canBeSerial_readReady);

    connect(&_tcpClient, &QTcpSocket::readyRead, this, &TcpConnection::on_readyRead);
    connect(&_tcpClient, &QTcpSocket::connected,this, &TcpConnection::on_tcpConnect);
    connect(&_tcpClient, &QTcpSocket::disconnected,this, &TcpConnection::on_tcpDisconnect);
    connect(&_tcpClient, &QTcpSocket::stateChanged, this , &TcpConnection::on_stateChanged);
}

TcpConnection::~TcpConnection()
{
    close();
}

void TcpConnection::open()
{
    _tcpClient.connectToHost(_ip ,_port);
    _tcpClient.setSocketOption(QAbstractSocket::LowDelayOption, 1);

    emit connectionChanged();
}

void TcpConnection::close()
{
    _tcpClient.disconnectFromHost();
}

bool TcpConnection::write(RoomBus::Message message)
{
    if(!_isConnected){
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

QString TcpConnection::getConnectionName()
{
    return "TCP";
}

QString TcpConnection::getConnectionPath()
{
    return "IP "+_ip+":"+QString::number(_port);
}

void TcpConnection::on_canBeSerial_writeReady(QByteArray data)
{
    _tcpClient.write(data,data.size());
}

void TcpConnection::on_canBeSerial_readReady(CanBusFrame frame)
{
    emit received(RoomBus::toMessage(frame.identifier, frame.data));
}

void TcpConnection::on_tcpConnect(void)
{
    _isConnected = _tcpClient.isOpen();

    if(_isConnected){
        _canSerial.setEnabled(true);
    }

    emit connectionChanged();
}

void TcpConnection::on_tcpDisconnect(void)
{
    _isConnected = _tcpClient.isOpen();
    emit connectionChanged();
}

void TcpConnection::on_stateChanged(QAbstractSocket::SocketState socketState)
{
    _isConnected = _tcpClient.isOpen();
    emit connectionChanged();
}

void TcpConnection::on_readyRead(void)
{
    _canSerial.receive(_tcpClient.readAll());
}

