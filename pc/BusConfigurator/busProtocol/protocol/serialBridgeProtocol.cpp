#include "serialBridgeProtocol.h"

serialBridgeProtocol::serialBridgeProtocol(busDevice *device):BusProtocol(device)
{

}

void serialBridgeProtocol::sendData(uint8_t port, QByteArray data)
{
    BusMessage msg;

    msg.protocol = Protocol::SerialBridgeProtocol;
    msg.command = commands_t::data;
    //msg.data.append(port);
    //msg.data.append(static_cast<uint8_t>(sbp_status_t::ok));
    msg.data.append(data);

    sendMessage(msg);
}

void serialBridgeProtocol::pushData(BusMessage msg)
{
    if(msg.protocol == Protocol::SerialBridgeProtocol)
    {
        if(msg.command == commands_t::data) _parseData(msg.data);

    }
}

QList<Protocol> serialBridgeProtocol::protocol()
{
    QList<Protocol> temp;
    temp.append(Protocol::SerialBridgeProtocol);
    return temp;
}

void serialBridgeProtocol::_parseData(QByteArray data)
{
    if(data.size() < 2) return;

    uint8_t port = static_cast<uint8_t>(data.at(0));
    sbp_status_t status = static_cast<sbp_status_t>(data.at(1));
    QByteArray rxData = data.remove(0,2);
    emit receiveData(port, status, rxData);
}
