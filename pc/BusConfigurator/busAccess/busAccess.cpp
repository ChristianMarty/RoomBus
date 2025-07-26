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
    if(_connection == nullptr){
        switch(_type){
        case Type::Undefined: return "";
        case Type::Serial: return "Serial";
        case Type::Tcp: return "TCP";
        case Type::Udp: return "UDP";
        case Type::SocketCan: return "Socket CAN";
        }
    }

    return _connection->getConnectionName();
}

QString RoomBusAccess::getConnectionPath()
{
    if(_connection == nullptr){
        return _url;
    }

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

void RoomBusAccess::setSocketCanConnection(QString port)
{
    _type = Type::SocketCan;
    _url = port;
}

void RoomBusAccess::setSerialConnection(QString port)
{
    _type = Type::Serial;
    _url = port;
}

void RoomBusAccess::setTcpConnection(QString ip, uint16_t port)
{
    _type = Type::Tcp;
    _url = ip;
    _port = port;
}

void RoomBusAccess::setUdpConnection(QString ip, uint16_t port)
{   
    _type = Type::Udp;
    _url = ip;
    _port = port;
}

void RoomBusAccess::openConnection()
{
    closeConnection();

    switch(_type){
        case Type::Undefined: return;
        case Type::Serial:
            _connection = new SerialConnection(_url, 614400);
            break;
        case Type::Tcp:
            _connection = new TcpConnection(_url, _port);
            break;
        case Type::Udp:
            _connection = new UdpConnection(_url, _port);
            break;
        case Type::SocketCan:
            _connection = new SocketCanConnection(_url);
            break;
    }

    _openConnection();
}

void RoomBusAccess::_openConnection(void)
{
    if(_connection == nullptr) return;

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
