#include "protocolBase.h"
#include "busDevice.h"

#include "deviceManagementProtocol/deviceManagementProtocol.h"
#include "messageLogProtocol.h"
#include "fileTransferProtocol.h"
#include "triggerProtocol.h"
#include "stateReportProtocol.h"
#include "eventProtocol.h"
#include "serialBridgeProtocol.h"
#include "valueProtocol.h"

ProtocolBase::ProtocolBase(RoomBusDevice *device)
{
    _device = device;
}

ProtocolBase::~ProtocolBase(void)
{
    _device->removeProtocol(this);
}

QString ProtocolBase::protocolName(MiniBus::Protocol protocol)
{
    switch((Protocol)protocol){
        case Protocol::DeviceManagementProtocol:  return "Device Management"; break;
        case Protocol::MessageLogProtocolId:  return "Message Logging"; break;
        case Protocol::FileTransferProtocol:  return "File Transfer"; break;
        case Protocol::TriggerSystemProtocol: return "Trigger"; break;
        case Protocol::EventSystemProtocol:   return "Event"; break;
        case Protocol::StateSystemProtocol:   return "State Report"; break;
        case Protocol::ValueSystemProtocol:   return "Value Report"; break;
        case Protocol::SerialBridgeProtocol:  return "Serial Bridge"; break;
    }
    return "Unknown Protocol";
}

QString ProtocolBase::commandName(MiniBus::Protocol protocol, MiniBus::Command command)
{
    switch((Protocol)protocol){
        case Protocol::DeviceManagementProtocol:   return DeviceManagementProtocol::commandName(command); break;
        case Protocol::MessageLogProtocolId:       return MessageLogProtocol::commandName(command); break;
        case Protocol::FileTransferProtocol:       return FileTransferProtocol::commandName(command); break;
        case Protocol::TriggerSystemProtocol:      return TriggerSystemProtocol::commandName(command); break;
        case Protocol::EventSystemProtocol:        return EventSystemProtocol::commandName(command); break;
        case Protocol::StateSystemProtocol:        return StateSystemProtocol::commandName(command); break;
        case Protocol::ValueSystemProtocol:        return ValueSystemProtocol::commandName(command); break;
        case Protocol::SerialBridgeProtocol:       return SerialBridgeProtocol::commandName(command); break;
    }
    return "Unknown Protocol";
}

QString ProtocolBase::dataDecoder(MiniBus::Protocol protocol, MiniBus::Command command, const QByteArray &data)
{
    switch((Protocol)protocol){
        case Protocol::DeviceManagementProtocol:   return DeviceManagementProtocol::dataDecoder(command, data); break;
        case Protocol::MessageLogProtocolId:       return MessageLogProtocol::dataDecoder(command, data); break;
        case Protocol::FileTransferProtocol:       return FileTransferProtocol::dataDecoder(command, data); break;
        case Protocol::TriggerSystemProtocol:      return TriggerSystemProtocol::dataDecoder(command, data); break;
        case Protocol::EventSystemProtocol:        return EventSystemProtocol::dataDecoder(command, data); break;
        case Protocol::StateSystemProtocol:        return StateSystemProtocol::dataDecoder(command, data); break;
        case Protocol::ValueSystemProtocol:        return ValueSystemProtocol::dataDecoder(command, data); break;
        case Protocol::SerialBridgeProtocol:       return SerialBridgeProtocol::dataDecoder(command, data); break;
    }
    return "Unknown Protocol";
}

void ProtocolBase::sendMessage(MiniBus::Message msg)
{
    msg.destinationAddress = _device->deviceAddress();
    emit _device->dataReady(msg);
}
