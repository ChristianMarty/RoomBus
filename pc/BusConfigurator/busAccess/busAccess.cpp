#include "busAccess.h"

#include "connection/serialConnection.h"
#include "connection/tcpConnection.h"
#include "connection/udpConnection.h"

RoomBusAccess::RoomBusAccess(void)
{
    _canSerial.setBaudrate(CANbeSerial::Baud500k);
    _canSerial.setDataBaudrate(CANbeSerial::Baud500k);
    _canSerial.setTxPaddingEnable(true, 0x00);

    connect(&_canSerial, &CANbeSerial::writeReady, this, &RoomBusAccess::on_writeReady);
    connect(&_canSerial, &CANbeSerial::readReady, this, &RoomBusAccess::on_readReady);
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

void RoomBusAccess::on_writeReady(QByteArray data)
{
    if(_connection == nullptr) return;

    _connection->write(data);
}

void RoomBusAccess::on_readReady(CanBusFrame frame)
{
    RoomBus::Message msg;
    uint32_t canId = frame.identifier;

    msg.sourceAddress = static_cast<uint8_t>(canId >>20)&0x7F;
    msg.destinationAddress = static_cast<uint8_t>(canId >>13)&0x7F;
    msg.protocol = static_cast<RoomBus::Protocol>(static_cast<uint8_t>(canId >>7)&0x3F);
    msg.command = static_cast<uint8_t>(canId>>4)&0x07;

    uint8_t paddingLength = canId &0x0F;

    msg.data = frame.data.mid(0,frame.data.size()-paddingLength);

    rxMsgBuffer.append(msg);
    emit newData();
}

void RoomBusAccess::on_receive(QByteArray data)
{
    _canSerial.receive(data);
}

void RoomBusAccess::on_connectionChanged()
{
    if(isConnected()){
        _canSerial.setEnabled(true);
    }

    emit connectionChanged();
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

    connect(_connection, &RoomBusConnection::receive, this, &RoomBusAccess::on_receive);
    connect(_connection, &RoomBusConnection::connectionChanged, this, &RoomBusAccess::on_connectionChanged);

    _connection->open();

    emit connectionChanged();
}

void RoomBusAccess::closeConnection(void)
{
    if(_connection == nullptr) return;

    disconnect(_connection, &RoomBusConnection::receive, this, &RoomBusAccess::on_receive);
    disconnect(_connection, &RoomBusConnection::connectionChanged, this, &RoomBusAccess::on_connectionChanged);

    delete _connection;
    _connection = nullptr;

    emit connectionChanged();
}

void RoomBusAccess::setDefaultPriority(Priority priority)
{
    _priority = priority;
}

bool RoomBusAccess::write(RoomBus::Message message)
{
    return write(message, _priority);
}

bool RoomBusAccess::write(RoomBus::Message message, Priority priority)
{
    if(_connection == nullptr) return false;

    uint8_t paddingLength = 0;

    if(message.data.size() > 8){
        for(auto i = 7; i < sizeof(_dlsCode); i++){
            if(_dlsCode[i] >= message.data.size()){
                paddingLength = _dlsCode[i] - message.data.size();
                break;
            }
        }
    }

    uint32_t canId = 0;
    canId |= (static_cast<uint32_t>(priority)<<27) & 0x18000000;
    canId |= (static_cast<uint32_t>(_sourceAddress)<<20) & 0x07F00000;
    canId |= (static_cast<uint32_t>(message.destinationAddress)<<13) & 0x000FE000;
    canId |= (static_cast<uint32_t>(message.protocol)<<7) & 0x00001F80;
    canId |= (static_cast<uint32_t>(message.command)<<4) & 0x00000070;
    canId |= (static_cast<uint32_t>(paddingLength)) & 0x0000000F;

    CanBusFrame frame;
    frame.identifier = canId;
    frame.extended = true;
    frame.fd = true;
    frame.data = message.data;
    frame.rtr = false;

    _canSerial.write(frame);

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


