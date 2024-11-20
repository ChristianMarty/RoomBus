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

void RoomBusInterface::sendTrigger(uint16_t triggerChannel)
{
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
