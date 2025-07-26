#include "protocolBase.h"
#include "busDevice.h"

ProtocolBase::ProtocolBase(RoomBusDevice *device)
{
    _device = device;
    _device->addProtocol(this);
}

ProtocolBase::~ProtocolBase(void)
{
    _device->removeProtocol(this);
}

void ProtocolBase::sendMessage(RoomBus::Message msg)
{
    msg.destinationAddress = _device->deviceAddress();
    _device->dataReady(msg);
}
