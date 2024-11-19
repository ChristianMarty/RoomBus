#ifndef ROOMBUSMESSAGE_H
#define ROOMBUSMESSAGE_H

#include <QObject>

namespace RoomBus {
    enum class Protocol {
        DeviceManagementProtocol = 0,
        MessageLogProtocolId = 1,
        FileTransferProtocol = 2,

        TriggerSystemProtocol = 8,
        EventSystemProtocol = 9,
        StateSystemProtocol = 10,
        ValueSystemProtocol = 11,
        SerialBridgeProtocol = 12
    };

    struct Message{
        uint8_t sourceAddress;
        uint8_t destinationAddress;
        Protocol protocol;
        uint8_t command;
        QByteArray data;
    };

    enum class DeviceManagementCommand {
        DeviceToHost,
        HostToDevice
    };

    enum class MessageLoggingCommand {
        SystemMessage = 0x00,
        SystemWarning = 0x01,
        SystemError = 0x02,
        Reserved0,

        ApplicationMessage = 0x04,
        ApplicationWarning = 0x05,
        ApplicationError = 0x06,
        Reserved1
    };

    enum class FileTransferCommand{
        Request  = 0,
        Response = 1,

        Read = 4,
        ReadAcknowledgment = 5,
        Write = 6,
        WriteAcknowledgment = 7
    };

    enum class TriggerSystemCommand {
        Trigger,
        Reserved0,
        Reserved1,
        Reserved2,

        SignalInformationReport,
        SlotInformationReport,

        SignalInformationRequest,
        SlotInformationRequest
    };

    enum class EventSystemCommand {
        Event,
        Reserved0,
        Reserved1,
        Reserved2,

        SignalInformationReport,
        SlotInformationReport,

        SignalInformationRequest,
        SlotInformationRequest
    };

    enum class StateSystemCommand{
        State,
        StateRequest,
        Reserved0,
        Reserved1,

        SignalInformationReport,
        SlotInformationReport,

        SignalInformationRequest,
        SlotInformationRequest
    };

    enum class ValueSystemCommand {
        ValueReport,
        ValueRequest,
        ValueCommand,
        Reserved0,

        SignalInformationReport,
        SlotInformationReport,

        SignalInformationRequest,
        SlotInformationRequest
    };

    enum class SerialBridgeCommand{
        Data  = 0,
        PortInfoReport = 4,
        PortInfoRequest = 5
    };

}



#endif // ROOMBUSMESSAGE_H
