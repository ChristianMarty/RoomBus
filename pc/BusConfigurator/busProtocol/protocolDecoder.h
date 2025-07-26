#ifndef PROTOCOLDECODER_H
#define PROTOCOLDECODER_H

#include <QString>
#include "roomBusMessage.h"

class ProtocolDecoder
{
public:

    static QString protocolName(RoomBus::Protocol protocol);
    static QString commandName(RoomBus::Protocol protocol, int command);

    static QString dataDecoded(RoomBus::Protocol protocol, int command, const QByteArray &data);

    static QString DeviceManagementCommandlName(RoomBus::DeviceManagementCommand command);
    static QString MessageLoggingCommandlName(RoomBus::MessageLoggingCommand command);
    static QString FileTransferCommandlName(RoomBus::FileTransferCommand command);

    static QString TriggerCommandlName(RoomBus::TriggerSystemCommand command);
    static QString EventCommandlName(RoomBus::EventSystemCommand command);
    static QString StateReportCommandlName(RoomBus::StateSystemCommand command);
    static QString ValueReportCommandlName(RoomBus::ValueSystemCommand command);
    static QString SerialBridgeCommandlName(RoomBus::SerialBridgeCommand command);

    static QString DeviceManagementDataDecoder(RoomBus::DeviceManagementCommand command, const QByteArray &data);
    static QString MessageLoggingDataDecoder(RoomBus::MessageLoggingCommand command, const QByteArray &data);
    static QString FileTransferDecoder(RoomBus::FileTransferCommand command, const QByteArray &data);

    static QString TriggerDecoder(RoomBus::TriggerSystemCommand command, const QByteArray &data);
    static QString EventDecoder(RoomBus::EventSystemCommand command, const QByteArray &data);
    static QString StateReportDecoder(RoomBus::StateSystemCommand command, const QByteArray &data);
    static QString ValueReportDecoder(RoomBus::ValueSystemCommand command, const QByteArray &data);
    static QString SerialBridgeDecoder(RoomBus::SerialBridgeCommand command, const QByteArray &data);
};

#endif // PROTOCOLDECODER_H

