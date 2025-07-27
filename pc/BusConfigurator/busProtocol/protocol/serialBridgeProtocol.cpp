#include "serialBridgeProtocol.h"
#include "busDevice.h"

SerialBridgeProtocol::SerialBridgeProtocol(RoomBusDevice *device)
    : ProtocolBase(device)
{
    _device->addProtocol(this);
}

void SerialBridgeProtocol::sendData(uint8_t port, QByteArray data)
{
    RoomBus::Message msg;

    msg.protocol = RoomBus::Protocol::SerialBridgeProtocol;
    msg.command = (uint8_t)RoomBus::SerialBridgeCommand::Data;
    //msg.data.append(port);
    //msg.data.append(static_cast<uint8_t>(sbp_status_t::ok));
    msg.data.append(data);

    sendMessage(msg);
}

void SerialBridgeProtocol::handleMessage(RoomBus::Message msg)
{
    if(msg.protocol != RoomBus::Protocol::SerialBridgeProtocol) return;

    if(msg.command == (uint8_t)RoomBus::SerialBridgeCommand::Data){
        _parseData(msg.data);
    }
}

void SerialBridgeProtocol::_parseData(QByteArray data)
{
    if(data.size() < 2) return;

    uint8_t port = static_cast<uint8_t>(data.at(0));
    sbp_status_t status = static_cast<sbp_status_t>(data.at(1));
    QByteArray rxData = data.remove(0,2);
    emit receiveData(port, status, rxData);
}
