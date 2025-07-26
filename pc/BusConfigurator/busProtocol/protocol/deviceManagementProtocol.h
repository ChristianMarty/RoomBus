#ifndef DEVICEMANAGEMENTPROTOCOL_H
#define DEVICEMANAGEMENTPROTOCOL_H

#include "protocolBase.h"

class DeviceManagementProtocol : public ProtocolBase
{
    Q_OBJECT
public:
    DeviceManagementProtocol(RoomBusDevice *device);
};

#endif // DEVICEMANAGEMENTPROTOCOL_H
