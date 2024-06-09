#ifndef PROTOCOLDECODER_H
#define PROTOCOLDECODER_H

#include <QString>
#include "protocol.h"

class ProtocolDecoder
{
public:

    static QString protocolName(Protocol protocol);
    static QString commandName(Protocol protocol, int command);

    static QString dataDecoded(Protocol protocol, int command, QByteArray data);

    static QString DeviceManagementCommandlName(DeviceManagementCommand command);
    static QString MessageLoggingCommandlName(MessageLoggingCommand command);
    static QString FileTransferCommandlName(FileTransferCommand command);

    static QString TriggerCommandlName(TriggerCommand command);
    static QString EventCommandlName(EventCommand command);
    static QString StateReportCommandlName(StateReportCommand command);
    static QString ValueReportCommandlName(ValueReportCommand command);
    static QString SerialBridgeCommandlName(SerialBridgeCommand command);

    static QString DeviceManagementDataDecoder(DeviceManagementCommand command, QByteArray data);
    static QString MessageLoggingDataDecoder(MessageLoggingCommand command, QByteArray data);
    static QString FileTransferDecoder(FileTransferCommand command, QByteArray data);

    static QString TriggerDecoder(TriggerCommand command, QByteArray data);
    static QString EventDecoder(EventCommand command, QByteArray data);
    static QString StateReportDecoder(StateReportCommand command, QByteArray data);
    static QString ValueReportDecoder(ValueReportCommand command, QByteArray data);
    static QString SerialBridgeDecoder(SerialBridgeCommand command, QByteArray data);
};

#endif // PROTOCOLDECODER_H

