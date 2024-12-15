 #include "roomBus.h"

RoomBusInterface::RoomBusInterface(QObject *parent)
    : QObject{parent}
{
    qDebug("Open com port");
    _busConnection.openSerialConnection("COM8");
}

RoomBusInterface::~RoomBusInterface()
{
    _busConnection.closeConnection();
}

void RoomBusInterface::sendValue(uint16_t valueChannel, uint32_t value)
{
    if(valueChannel == 0) return;

    RoomBus::Message msg;

    msg.destinationAddress = RoomBus::BroadcastAddress;
    msg.sourceAddress = _sourceAddress;

    msg.protocol = RoomBus::Protocol::ValueSystemProtocol;
    msg.command = (uint8_t)RoomBus::ValueSystemCommand::ValueCommand;
    msg.data.append(RoomBus::packUint16(valueChannel));
    msg.data.append(0x11); // Set value 	Clamp 	Long
    msg.data.append(RoomBus::packUint32(value));

    _busConnection.write(msg);
}

void RoomBusInterface::sendTrigger(uint16_t triggerChannel)
{
    if(triggerChannel == 0) return;

    RoomBus::Message msg;

    msg.destinationAddress = RoomBus::BroadcastAddress;
    msg.sourceAddress = _sourceAddress;

    msg.protocol = RoomBus::Protocol::TriggerSystemProtocol;
    msg.command = (uint8_t)RoomBus::TriggerSystemCommand::Trigger;
    msg.data.append(RoomBus::packUint16(triggerChannel));

    _busConnection.write(msg);
}

RoomBusAccess *RoomBusInterface::busConnection()
{
    return &_busConnection;
}
