#ifndef ROOMBUSMESSAGE_H
#define ROOMBUSMESSAGE_H

#include <QObject>

namespace RoomBus {

    static constexpr uint8_t BroadcastAddress = 0x7F;

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


    static inline uint8_t unpackUint8(QByteArray data, uint32_t index)
    {
        return static_cast<uint8_t>(data.at(index));
    }

    static inline uint16_t unpackUint16(QByteArray data, uint32_t index)
    {
        uint16_t temp = 0;
        temp |= (static_cast<uint16_t>(data.at(index))<<8)&0xFF00;
        temp |= (static_cast<uint16_t>(data.at(index+1))<<0)&0x00FF;
        return temp;
    }

    static inline uint32_t unpackUint32(QByteArray data, uint32_t index)
    {
        uint32_t temp = 0;
        temp =  (static_cast<uint32_t>(data.at(index))<<24)&0xFF000000;
        temp |= (static_cast<uint32_t>(data.at(index+1))<<16)&0x00FF0000;
        temp |= (static_cast<uint32_t>(data.at(index+2))<<8)&0x0000FF00;
        temp |= (static_cast<uint32_t>(data.at(index+3))<<0)&0x000000FF;
        return temp;
    }

    static inline float unpackFloat32(QByteArray data, uint32_t index)
    {
        uint32_t temp = unpackUint32(data,index);
        float *out;
        out = (float*)&temp;
        return *out;
    }

    static inline QByteArray packUint16(uint16_t value)
    {
        QByteArray temp;
        temp.append(static_cast<char>(static_cast<uint8_t>(value>>24)));
        temp.append(static_cast<char>(static_cast<uint8_t>(value>>16)));
        temp.append(static_cast<char>(static_cast<uint8_t>(value>>8)));
        temp.append(static_cast<char>(static_cast<uint8_t>(value)));
        return temp;
    }

    static inline QByteArray packUint32(uint32_t value)
    {
        QByteArray temp;
        temp.append(static_cast<char>(static_cast<uint8_t>(value>>8)));
        temp.append(static_cast<char>(static_cast<uint8_t>(value)));
        return temp;
    }

    static inline QByteArray packFloat32(float value)
    {
        uint32_t *temp = (uint32_t*)&value;
        return  packUint32(*temp);
    }
}

#endif // ROOMBUSMESSAGE_H
