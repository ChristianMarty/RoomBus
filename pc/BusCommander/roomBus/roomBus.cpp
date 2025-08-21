 #include "roomBus.h"

RoomBusInterface::RoomBusInterface(Type type,  QString port, QObject *parent)
    : QObject{parent}
    , _type{type}
{
    switch(type){
        case Type::Serial:
            qDebug(QString("Open serial port: "+port).toLocal8Bit());
            _busConnection.setSerialConnection(port);
            break;

        case Type::Can:
            qDebug(QString("Open SocketCan port: "+port).toLocal8Bit());
            _busConnection.setSocketCanConnection(port);
            break;
    }

    _busConnection.openConnection();
}

RoomBusInterface::~RoomBusInterface()
{
    _busConnection.closeConnection();
}

void RoomBusInterface::sendValue(uint16_t valueChannel, uint32_t value)
{
    if(valueChannel == 0) return;

    MiniBus::Message msg;

    msg.destinationAddress = MiniBus::BroadcastAddress;
    msg.sourceAddress = _sourceAddress;

    msg.protocol = MiniBus::Protocol::ValueSystemProtocol;
    msg.command = (uint8_t)MiniBus::ValueSystemCommand::ValueCommand;
    msg.data.append(MiniBus::packUint16(valueChannel));
    msg.data.append(0x11); // Set value 	Clamp 	Long
    msg.data.append(MiniBus::packUint32(value));

    _busConnection.write(msg);
}

void RoomBusInterface::sendTrigger(uint16_t triggerChannel)
{
    if(triggerChannel == 0) return;

    MiniBus::Message msg;

    msg.destinationAddress = MiniBus::BroadcastAddress;
    msg.sourceAddress = _sourceAddress;

    msg.protocol = MiniBus::Protocol::TriggerSystemProtocol;
    msg.command = (uint8_t)MiniBus::TriggerSystemCommand::Trigger;
    msg.data.append(MiniBus::packUint16(triggerChannel));

    _busConnection.write(msg);
}

MiniBusAccess *RoomBusInterface::busConnection()
{
    return &_busConnection;
}
