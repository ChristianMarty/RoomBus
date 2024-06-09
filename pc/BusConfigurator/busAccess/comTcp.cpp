#include "comTcp.h"

using namespace std;

tcpCom::tcpCom(void)
{

}

tcpCom::~tcpCom()
{
    _tcpClient.disconnectFromHost();
}

bool tcpCom::openConnection(QString ip, uint16_t port)
{
    _tcpIp = ip;
    _tcpPort = port;
    _tcpClient.connectToHost(_tcpIp ,_tcpPort);

    connect(&_tcpClient, &QTcpSocket::readyRead, this, &tcpCom::on_tcpRx);
    connect(&_tcpClient, &QTcpSocket::connected,this, &tcpCom::on_tcpConnect);
    connect(&_tcpClient, &QTcpSocket::disconnected,this, &tcpCom::on_tcpDisconnect);
    connect(&_tcpClient, &QTcpSocket::stateChanged, this , &tcpCom::on_stateChanged);

    _tcpClient.setSocketOption(QAbstractSocket::LowDelayOption, 1);

    emit connectionChanged(true);

    return true;
}

void tcpCom::write(QByteArray data)
{
    _tcpClient.write(data,data.size());
}

QString tcpCom::getConnectionName()
{
    return "TCP";
}

QString tcpCom::getConnectionPath()
{
    return "IP "+_tcpIp+":"+QString::number(_tcpPort);
}

bool tcpCom::isConnected(void)
{
    return _tcpClient.isOpen();
}

void tcpCom::on_tcpConnect(void)
{
    emit connectionChanged(true);
}

void tcpCom::on_tcpDisconnect(void)
{
    emit connectionChanged(false);
}

void tcpCom::on_stateChanged(QAbstractSocket::SocketState socketState)
{
    if(socketState != QAbstractSocket::SocketState::ConnectedState) emit connectionChanged(false);
    else emit connectionChanged(true);
}

void tcpCom::on_tcpRx(void)
{
    emit receive(_tcpClient.readAll());
}

