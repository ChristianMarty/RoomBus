#include "busDevice.h"
#include <QTimer>
#include "protocol/protocolBase.h"

RoomBusDevice::RoomBusDevice(uint8_t deviceAddress, QObject *parent)
    : QObject(parent)
    ,_deviceManagementProtocol{this}
{
    _deviceAddress = deviceAddress;
}

void RoomBusDevice::setDeviceAddress(uint8_t deviceAddress)
{
    _deviceAddress = deviceAddress;
}

uint8_t RoomBusDevice::deviceAddress() const
{
    return _deviceAddress;
}

QString RoomBusDevice::deviceName() const
{
    return _deviceManagementProtocol._deviceName;
}

QString RoomBusDevice::hardwareName() const
{
    return _deviceManagementProtocol._hardwareName;
}

QString RoomBusDevice::applicationName() const
{
    return _deviceManagementProtocol._applicationName;
}

QString RoomBusDevice::kernelVersionString(void) const
{
    uint8_t high = (uint8_t)((_deviceManagementProtocol._kernelVersion >> 8) & 0xFF);
    uint8_t low = (uint8_t)(_deviceManagementProtocol._kernelVersion & 0xFF);

    return QString::number(high,10).rightJustified(2, '0')+"."+QString::number(low,10).rightJustified(2, '0');
}

QString RoomBusDevice::hardwareVersionString(void) const
{
    uint8_t high = (uint8_t)((_deviceManagementProtocol._hardwareVersion >> 8) & 0xFF);
    uint8_t low = (uint8_t)(_deviceManagementProtocol._hardwareVersion & 0xFF);

    return QString::number(high,10).rightJustified(2, '0')+"."+QString::number(low,10).rightJustified(2, '0');
}

QString RoomBusDevice::deviceIdentificationString(void) const
{
    switch(_deviceManagementProtocol._deviceIdentificationCode) {
        case 0x61810502: return "ATSAME51J19A (0x61810502)";
        case 0x61810002: return "ATSAME51J19A (0x61810002)";
        case 0x61810303: return "ATSAME51J18A (0x61810303)";
        default: return "Unknown Device (0x"+QString::number(_deviceManagementProtocol._deviceIdentificationCode,16).rightJustified(8, '0').toUpper()+")";
    }
}

QString RoomBusDevice::deviceSerialNumberString(void) const
{
    QString temp;
    temp  = QString::number(_deviceManagementProtocol._serialNumber.word3, 16).rightJustified(8, '0').toUpper()+"-";
    temp += QString::number(_deviceManagementProtocol._serialNumber.word2, 16).rightJustified(8, '0').toUpper()+"-";
    temp += QString::number(_deviceManagementProtocol._serialNumber.word1, 16).rightJustified(8, '0').toUpper()+"-";
    temp += QString::number(_deviceManagementProtocol._serialNumber.word0, 16).rightJustified(8, '0').toUpper();
    return temp;
}

DeviceManagementProtocol::SystemStatus RoomBusDevice::systemStatus() const
{
    return _deviceManagementProtocol._systemStatus;
}

bool RoomBusDevice::timeoutStatus() const
{
    return _deviceManagementProtocol._timeoutStatus;
}

DeviceManagementProtocol &RoomBusDevice::management()
{
    return _deviceManagementProtocol;
}

QDateTime RoomBusDevice::lastHeartbeat() const
{
    return _deviceManagementProtocol._lastMessage;
}

void RoomBusDevice::addProtocol(ProtocolBase* protocol)
{
    _protocols.append(protocol);
}

void RoomBusDevice::removeProtocol(ProtocolBase* protocol)
{
    _protocols.removeOne(protocol);
}

void RoomBusDevice::handleMessage(MiniBus::Message message)
{
    if(message.sourceAddress != _deviceAddress) return;

    _deviceManagementProtocol.handleMessage(message);

    for(ProtocolBase* protocol: _protocols) {
        if(protocol == nullptr) continue;
        protocol->handleMessage(message);
    }
}

MiniBus::Message RoomBusDevice::busScan()
{
    MiniBus::Message msg;
    msg.destinationAddress = MiniBus::BroadcastAddress;
    msg.protocol = (MiniBus::Protocol)ProtocolBase::Protocol::DeviceManagementProtocol;
    msg.command = (MiniBus::Command)DeviceManagementProtocol::Command::HostToDevice;
    msg.data.append((uint8_t)DeviceManagementProtocol::DeviceManagementSubCommand::SystemInformationRequest);
    msg.priority = MiniBus::Priority::Normal;

    return msg;
}
