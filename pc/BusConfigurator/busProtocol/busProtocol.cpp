#include "busProtocol.h"
#include "busDevice.h"

BusProtocol::BusProtocol(RoomBusDevice *device)
{
    _device = device;
    _device->addProtocol(this);
}

BusProtocol::~BusProtocol(void)
{
    _device->removeProtocol(this);
}

void BusProtocol::sendMessage(RoomBus::Message msg)
{
    msg.destinationAddress = _device->deviceAddress();
    _device->dataReady(msg);
}
