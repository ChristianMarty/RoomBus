#ifndef PROTOCOLDECODER_H
#define PROTOCOLDECODER_H

#include <QString>
#include "roomBusMessage.h"

class ProtocolDecoder
{
public:

    static QString protocolName(RoomBus::Protocol protocol);
    static QString commandName(RoomBus::Protocol protocol, int command);

    static QString dataDecoded(RoomBus::Protocol protocol, int command, QByteArray data);

    static QString DeviceManagementCommandlName(RoomBus::DeviceManagementCommand command);
    static QString MessageLoggingCommandlName(RoomBus::MessageLoggingCommand command);
    static QString FileTransferCommandlName(RoomBus::FileTransferCommand command);

    static QString TriggerCommandlName(RoomBus::TriggerSystemCommand command);
    static QString EventCommandlName(RoomBus::EventSystemCommand command);
    static QString StateReportCommandlName(RoomBus::StateSystemCommand command);
    static QString ValueReportCommandlName(RoomBus::ValueSystemCommand command);
    static QString SerialBridgeCommandlName(RoomBus::SerialBridgeCommand command);

    static QString DeviceManagementDataDecoder(RoomBus::DeviceManagementCommand command, QByteArray data);
    static QString MessageLoggingDataDecoder(RoomBus::MessageLoggingCommand command, QByteArray data);
    static QString FileTransferDecoder(RoomBus::FileTransferCommand command, QByteArray data);

    static QString TriggerDecoder(RoomBus::TriggerSystemCommand command, QByteArray data);
    static QString EventDecoder(RoomBus::EventSystemCommand command, QByteArray data);
    static QString StateReportDecoder(RoomBus::StateSystemCommand command, QByteArray data);
    static QString ValueReportDecoder(RoomBus::ValueSystemCommand command, QByteArray data);
    static QString SerialBridgeDecoder(RoomBus::SerialBridgeCommand command, QByteArray data);
};

#endif // PROTOCOLDECODER_H

