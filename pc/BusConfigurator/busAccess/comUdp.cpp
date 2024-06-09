#include "comUdp.h"

using namespace std;

udpCom::udpCom(void)
{

}

udpCom::~udpCom()
{
    _udpClient.disconnectFromHost();
}

bool udpCom::openConnection(QString ip, uint16_t port)
{

    _udpIp = ip;
    _udpPort = port;
    _udpHost.setAddress(_udpIp);

    //_udpClient.connectToHost(_udpIp ,_udpPort);
    _udpClient.bind(_udpHost ,_udpPort);


    connect(&_udpClient, &QUdpSocket::readyRead, this, &udpCom::on_udpRx);
    connect(&_udpClient, &QUdpSocket::connected,this, &udpCom::on_udpConnect);
    connect(&_udpClient, &QUdpSocket::disconnected,this, &udpCom::on_udpDisconnect);
    connect(&_udpClient, &QUdpSocket::stateChanged, this , &udpCom::on_stateChanged);

    //_udpClient.setSocketOption(QAbstractSocket::LowDelayOption, 1);

    emit connectionChanged(true);

    return true;
}

QString udpCom::getConnectionName()
{
    return "UDP";
}

QString udpCom::getConnectionPath()
{
    return "IP "+_udpIp+":"+QString::number(_udpPort);
}

bool udpCom::isConnected(void)
{
    return _udpClient.isOpen();
}

void udpCom::on_udpConnect(void)
{
    emit connectionChanged(true);
}

void udpCom::on_udpDisconnect(void)
{
    emit connectionChanged(false);
}

void udpCom::on_stateChanged(QAbstractSocket::SocketState socketState)
{
    if(socketState != QAbstractSocket::SocketState::ConnectedState) emit connectionChanged(false);
    else emit connectionChanged(true);
}

void udpCom::write(QByteArray data)
{
    _udpClient.write(data,data.size());
}

void udpCom::on_udpRx(void)
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
