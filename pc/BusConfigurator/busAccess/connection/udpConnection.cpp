#include "udpConnection.h"

using namespace std;

UdpConnection::UdpConnection(QString ip, uint16_t port)
    :_ip{ip}
    ,_port{port}
{
    connect(&_udpClient, &QUdpSocket::readyRead, this, &UdpConnection::on_readyRead);
    connect(&_udpClient, &QUdpSocket::connected,this, &UdpConnection::on_udpConnect);
    connect(&_udpClient, &QUdpSocket::disconnected,this, &UdpConnection::on_udpDisconnect);
    connect(&_udpClient, &QUdpSocket::stateChanged, this , &UdpConnection::on_stateChanged);
}

UdpConnection::~UdpConnection()
{
    close();
}

void UdpConnection::open()
{
    _udpHost.setAddress(_ip);
    emit connectionChanged();
}

void UdpConnection::close()
{
    _udpClient.disconnectFromHost();
}


QString UdpConnection::getConnectionName()
{
    return "UDP";
}

QString UdpConnection::getConnectionPath()
{
    return "IP "+_ip+":"+QString::number(_port);
}

void UdpConnection::on_udpConnect(void)
{
    _isConnected = _udpClient.isOpen();
    emit connectionChanged();
}

void UdpConnection::on_udpDisconnect(void)
{
    _isConnected = _udpClient.isOpen();
    emit connectionChanged();
}

void UdpConnection::on_stateChanged(QAbstractSocket::SocketState socketState)
{
    _isConnected = _udpClient.isOpen();
    emit connectionChanged();
}

bool UdpConnection::write(QByteArray data)
{
    if(!_isConnected) return false;

    _udpClient.write(data,data.size());
    return true;
}

void UdpConnection::on_readyRead(void)
{
    while (_udpClient.hasPendingDatagrams())
    {
        QHostAddress sender;
        uint16_t port;
        QByteArray datagram;
        datagram.resize(_udpClient.pendingDatagramSize());
        _udpClient.readDatagram(datagram.data(),datagram.size(), &sender, &port);


        //uint32_t inPort = _udpClient.();
        //port == inPort
        if(_udpHost.isEqual(sender))
        {
            emit receive(datagram);
        }
    }
}
