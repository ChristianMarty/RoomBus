#include "protocolBase.h"
#include "busDevice.h"

ProtocolBase::ProtocolBase(RoomBusDevice *device)
{
    _device = device;
}

ProtocolBase::~ProtocolBase(void)
{
    _device->removeProtocol(this);
}

void ProtocolBase::sendMessage(RoomBus::Message msg)
{
    msg.destinationAddress = _device->deviceAddress();
    emit _device->dataReady(msg);
}
