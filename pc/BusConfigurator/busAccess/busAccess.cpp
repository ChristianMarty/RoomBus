#include "busAccess.h"

#include "connection/serialConnection.h"
#include "connection/tcpConnection.h"
#include "connection/udpConnection.h"
#include "connection/socketCanConnection.h"

RoomBusAccess::RoomBusAccess(void)
{
}

RoomBusAccess::~RoomBusAccess(void)
{
    closeConnection();
}

QString RoomBusAccess::getConnectionName()
{
    if(_connection == nullptr) return "";

    return _connection->getConnectionName();
}

QString RoomBusAccess::getConnectionPath()
{
    if(_connection == nullptr) return "";

    return _connection->getConnectionPath();
}

void RoomBusAccess::on_received(RoomBus::Message message)
{
    rxMsgBuffer.append(message);
    emit newData();
}

void RoomBusAccess::on_connectionChanged()
{
    emit connectionChanged();
}

void RoomBusAccess::openSocketCanConnection(QString port)
{
    closeConnection();
    _connection = new SocketCanConnection(port);
    _openConnection();
}

void RoomBusAccess::openTcpConnection(QString ip, uint16_t port)
{
    closeConnection();
    _connection = new TcpConnection(ip,port);
    _openConnection();
}

void RoomBusAccess::openUdpConnection(QString ip, uint16_t port)
{
    closeConnection();
    _connection = new UdpConnection(ip,port);
    _openConnection();
}

void RoomBusAccess::openSerialConnection(QString port)
{
    closeConnection();
    _connection = new SerialConnection(port, 614400);
    _openConnection();
}

void RoomBusAccess::_openConnection(void)
{
    if(_connection == nullptr) return;

    _sourceAddress = 0x7E; // DEC 126

    connect(_connection, &RoomBusConnection::received, this, &RoomBusAccess::on_received);
    connect(_connection, &RoomBusConnection::connectionChanged, this, &RoomBusAccess::on_connectionChanged);

    _connection->open();

    emit connectionChanged();
}

void RoomBusAccess::closeConnection(void)
{
    if(_connection == nullptr) return;

    disconnect(_connection, &RoomBusConnection::received, this, &RoomBusAccess::on_received);
    disconnect(_connection, &RoomBusConnection::connectionChanged, this, &RoomBusAccess::on_connectionChanged);

    delete _connection;
    _connection = nullptr;

    emit connectionChanged();
}

bool RoomBusAccess::write(RoomBus::Message message)
{
    if(_connection == nullptr){
        return false;
    }

    message.sourceAddress = _sourceAddress;
    _connection->write(message);

    return true;
}

bool RoomBusAccess::isConnected(void)
{
    if(_connection == nullptr) return false;
    else return _connection->connected();
}

QString RoomBusAccess::lastError()
{
    if(_connection == nullptr) return "No open connection";
    else return _connection->lastError();
}
