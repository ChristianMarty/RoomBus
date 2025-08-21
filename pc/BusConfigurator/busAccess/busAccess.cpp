#include "busAccess.h"
#include "connection/connection.h"

#include "connection/serialConnection.h"
#include "connection/tcpConnection.h"
#include "connection/udpConnection.h"
#include "connection/socketCanConnection.h"

MiniBusAccess::MiniBusAccess(void)
{
}

MiniBusAccess::~MiniBusAccess(void)
{
    closeConnection();
}

QString MiniBusAccess::getConnectionName()
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

QString MiniBusAccess::getConnectionPath()
{
    if(_connection == nullptr){
        return _url;
    }

    return _connection->getConnectionPath();
}

bool MiniBusAccess::setSocketCanConnection(QString port)
{
    if(_type != Type::Undefined) return false;
    if(isConnected()) return false;

    _type = Type::SocketCan;
    _url = port;

    return true;
}

bool MiniBusAccess::setSerialConnection(QString port)
{
    if(_type != Type::Undefined) return false;
    if(isConnected()) return false;

    _type = Type::Serial;
    _url = port;

    return true;
}

bool MiniBusAccess::setTcpConnection(QString ip, uint16_t port)
{
    if(_type != Type::Undefined) return false;
    if(isConnected()) return false;

    _type = Type::Tcp;
    _url = ip;
    _port = port;

    return true;
}

bool MiniBusAccess::setUdpConnection(QString ip, uint16_t port)
{
    if(_type != Type::Undefined) return false;
    if(isConnected()) return false;

    _type = Type::Udp;
    _url = ip;
    _port = port;

    return true;
}

void MiniBusAccess::restConnection(void)
{
    if(isConnected()){
        closeConnection();
    }

    _type = Type::Undefined;
    _url.clear();
    _port = 0;
}

void MiniBusAccess::openConnection()
{
    closeConnection();

    switch(_type){
        case Type::Undefined: return;
        case Type::Serial:
            _connection = new SerialConnection(this, _url, 614400);
            break;
        case Type::Tcp:
            _connection = new TcpConnection(this, _url, _port);
            break;
        case Type::Udp:
            _connection = new UdpConnection(this, _url, _port);
            break;
        case Type::SocketCan:
            _connection = new SocketCanConnection(this, _url);
            break;
    }

    _openConnection();
}

void MiniBusAccess::_openConnection(void)
{
    if(_connection == nullptr) return;

    _connection->open();

    emit connectionChanged();
}

void MiniBusAccess::_handleMessageReceived(const MiniBus::Message &message)
{
    rxMessageBuffer.append(message);
    emit messageReceived();
}

void MiniBusAccess::setSourceAddress(uint8_t newSourceAddress)
{
    _sourceAddress = newSourceAddress;
}

void MiniBusAccess::closeConnection(void)
{
    if(_connection == nullptr) return;

    delete _connection;
    _connection = nullptr;

    emit connectionChanged();
}

bool MiniBusAccess::write(MiniBus::Message message, bool sourceOverride)
{
    if(_connection == nullptr){
        return false;
    }

    if(sourceOverride){
        message.sourceAddress = _sourceAddress;
    }

    if(message.sourceAddress >= 0x7F) return false;
    if(message.destinationAddress > 0x7F) return false;
    if(static_cast<uint8_t>(message.protocol) > 0x3F) return false;
    if(static_cast<uint8_t>(message.priority) > 0x03) return false;
    if(static_cast<uint8_t>(message.command) > 0x07) return false;
    if(message.data.length() > 64) return false;

    _connection->write(message);

    return true;
}

bool MiniBusAccess::isConnected(void)
{
    if(_connection == nullptr) return false;
    else return _connection->connected();
}

QString MiniBusAccess::lastError()
{
    if(_connection == nullptr) return "No open connection";
    else return _connection->lastError();
}
