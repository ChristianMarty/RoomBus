#include "busAccess.h"

busAccess::busAccess(comType_t type)
{
    switch(type)
    {
        case serial:    _com = new serialCom();
                        break;

        case tcp:       _com = new tcpCom();
                        break;

        case udp:       _com = new udpCom();
                        break;
    }
    isConnected = false;

    _canSerial.setBaudrate(CANbeSerial::Baud500k);
    _canSerial.setDataBaudrate(CANbeSerial::Baud500k);
    _canSerial.setTxPaddingEnable(true, 0x00);

    connect(&_canSerial, &CANbeSerial::writeReady, this, &busAccess::on_writeReady);
    connect(&_canSerial, &CANbeSerial::readReady, this, &busAccess::on_readReady);
}

busAccess::~busAccess(void)
{
    delete _com;
}

QString busAccess::getConnectionName()
{
    return _com->getConnectionName();
}

QString busAccess::getConnectionPath()
{
    return _com->getConnectionPath();
}

void busAccess::on_writeReady(QByteArray data)
{
    _com->write(data);
}

void busAccess::on_readReady(CanBusFrame frame)
{
    busMessage msg;
    uint32_t canId = frame.identifier;

    msg.srcAddress = static_cast<uint8_t>(canId >>20)&0x7F;
    msg.dstAddress = static_cast<uint8_t>(canId >>13)&0x7F;
    msg.protocol = static_cast<Protocol>(static_cast<uint8_t>(canId >>7)&0x3F);
    msg.command = static_cast<uint8_t>(canId>>4)&0x07;

    uint8_t paddingLength = canId &0x0F;

    msg.data = frame.data.mid(0,frame.data.size()-paddingLength);

    rxMsgBuffer.append(msg);
    emit newData();
}

void busAccess::on_receive(QByteArray data)
{
    _canSerial.receive(data);
}

void busAccess::openTcpConnection(QString ip, uint16_t port)
{
    openConnection();
    ((tcpCom*)_com)->openConnection(ip,port);
}

void busAccess::openUdpConnection(QString ip, uint16_t port)
{
    openConnection();
    ((udpCom*)_com)->openConnection(ip,port);
}

void busAccess::openSerialConnection(QString port)
{
    openConnection();
    ((serialCom*)_com)->openConnection(port);

}

void busAccess::openConnection(void)
{
    _srcAddress = 0x7E; // DEC 126

    connect(_com, &busCom::receive, this, &busAccess::on_receive);
    connect(_com, &busCom::connectionChanged, this, &busAccess::on_connectionChanged);

    isConnected = true;

    emit connectionChanged(isConnected);
}

void busAccess::closeConnection(void)
{
    disconnect(_com, &busCom::receive, this, &busAccess::on_receive);
    disconnect(_com, &busCom::connectionChanged, this, &busAccess::on_connectionChanged);

    delete _com;

    isConnected = false;
    emit connectionChanged(isConnected);
}

void busAccess::setPriority(busPriority priority)
{
    _priority = priority;
}

void busAccess::setPriority(uint8_t priority)
{
    if(priority>= 8) priority = 7;
    setPriority((busPriority)priority);
}

bool busAccess::write(busMessage msg)
{
    return write(msg, _priority);
}

bool busAccess::write(busMessage msg, busPriority priority)
{
    return write(msg.dstAddress, msg.protocol, msg.command, msg.data, priority);
}

bool busAccess::write(uint8_t destAddress, Protocol protocol, uint8_t command, QByteArray data)
{
    return write(destAddress, protocol, command, data, _priority);
}

bool busAccess::write(uint8_t destAddress, Protocol protocol, uint8_t command, QByteArray data, busPriority priority)
{
    if(!isConnected) return false;
    uint8_t paddingLength = 0;

    if(data.size() > 8)
    {
        for(auto i = 7; i< sizeof(_dlsCode); i++)
        {
            if(_dlsCode[i] >= data.size())
            {
                paddingLength = _dlsCode[i] - data.size();
                break;
            }
        }
    }

    CanBusFrame frame;

    uint32_t canId = 0;
    canId |= (static_cast<uint32_t>(priority)<<27) & 0x18000000;
    canId |= (static_cast<uint32_t>(_srcAddress)<<20) & 0x07F00000;
    canId |= (static_cast<uint32_t>(destAddress)<<13) & 0x000FE000;
    canId |= (static_cast<uint32_t>(protocol)<<7) & 0x00001F80;
    canId |= (static_cast<uint32_t>(command)<<4) & 0x00000070;
    canId |= (static_cast<uint32_t>(paddingLength)) & 0x0000000F;

    frame.identifier = canId;
    frame.extended = true;
    frame.fd = true;
    frame.data = data;
    frame.rtr = false;

    _canSerial.write(frame);

    return true;
}

void busAccess::on_connectionChanged(bool connected)
{
    if(connected){
        _canSerial.setEnabled(true);
    }

    isConnected = connected;
    emit connectionChanged(isConnected);
}

bool busAccess::getIsConnected(void)
{
    return isConnected;
}


