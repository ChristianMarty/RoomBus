#ifndef BUS_PROTOCOL_H
#define BUS_PROTOCOL_H

#include "miniBusMessage.h"

class RoomBusDevice;

class ProtocolBase: public QObject
{
    Q_OBJECT
public:

    enum class Protocol:MiniBus::Protocol {
        DeviceManagementProtocol = 0,
        MessageLogProtocolId = 1,
        FileTransferProtocol = 2,

        TriggerSystemProtocol = 8,
        EventSystemProtocol = 9,
        StateSystemProtocol = 10,
        ValueSystemProtocol = 11,
        SerialBridgeProtocol = 12
    };

    ProtocolBase(RoomBusDevice *device);
    ~ProtocolBase(void);

    virtual void handleMessage(const MiniBus::Message &message) = 0;

    static QString protocolName(MiniBus::Protocol protocol);
    static QString commandName(MiniBus::Protocol protocol, MiniBus::Command command);
    static QString dataDecoder(MiniBus::Protocol protocol, MiniBus::Command command, const QByteArray &data);

protected:
    RoomBusDevice *_device;

    void sendMessage(MiniBus::Message msg);
};
#endif // BUS_PROTOCOL_H
