#include "tcpConnection.h"

using namespace std;

TcpConnection::TcpConnection(QString ip, uint16_t port)
    :_ip{ip}
    ,_port{port}
{
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

bool TcpConnection::write(QByteArray data)
{
    if(!_isConnected) return false;

    _tcpClient.write(data,data.size());
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

void TcpConnection::on_tcpConnect(void)
{
    _isConnected = _tcpClient.isOpen();
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
    emit receive(_tcpClient.readAll());
}

