#ifndef ROOMBUSMESSAGE_H
#define ROOMBUSMESSAGE_H

#include <QObject>

namespace RoomBus {

    static constexpr uint8_t BroadcastAddress = 0x7F;
    static constexpr uint8_t canDlsCode[16] = {0,1,2,3,4,5,6,7,8,12,16,20,24,32,48,64};

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

    enum class Priority {
        Reserved = 0,
        High = 1,
        Normal = 2,
        Low = 3
    };

    using Command = uint8_t;

    struct Message{
        uint8_t sourceAddress;
        uint8_t destinationAddress;
        Protocol protocol;
        Priority priority = Priority::Low;
        Command command;
        QByteArray data;
    };


    enum class DeviceManagementCommand:Command {
        DeviceToHost,
        HostToDevice
    };

    enum class MessageLoggingCommand:Command {
        SystemMessage = 0x00,
        SystemWarning = 0x01,
        SystemError = 0x02,
        Reserved0,

        ApplicationMessage = 0x04,
        ApplicationWarning = 0x05,
        ApplicationError = 0x06,
        Reserved1
    };

    enum class FileTransferCommand:Command {
        Request  = 0,
        Response = 1,

        Read = 4,
        ReadAcknowledgment = 5,
        Write = 6,
        WriteAcknowledgment = 7
    };

    enum class TriggerSystemCommand:Command {
        Trigger,
        Reserved0,
        Reserved1,
        Reserved2,

        SignalInformationReport,
        SlotInformationReport,

        SignalInformationRequest,
        SlotInformationRequest
    };

    enum class EventSystemCommand:Command {
        Event,
        Reserved0,
        Reserved1,
        Reserved2,

        SignalInformationReport,
        SlotInformationReport,

        SignalInformationRequest,
        SlotInformationRequest
    };

    enum class StateSystemCommand:Command {
        State,
        StateRequest,
        Reserved0,
        Reserved1,

        SignalInformationReport,
        SlotInformationReport,

        SignalInformationRequest,
        SlotInformationRequest
    };

    enum class ValueSystemCommand:Command {
        ValueReport,
        ValueRequest,
        ValueCommand,
        Reserved0,

        SignalInformationReport,
        SlotInformationReport,

        SignalInformationRequest,
        SlotInformationRequest
    };

    enum class SerialBridgeCommand:Command {
        Data  = 0,
        PortInfoReport = 4,
        PortInfoRequest = 5
    };


    static inline uint32_t toCanIdentifier(const Message &message)
    {
        uint8_t paddingLength = 0;

        if(message.data.size() > 8){
            for(uint8_t i = 7; i < sizeof(canDlsCode); i++){
                if(canDlsCode[i] >= message.data.size()){
                    paddingLength = canDlsCode[i] - message.data.size();
                    break;
                }
            }
        }

        uint32_t canId = 0;
        canId |= (static_cast<uint32_t>(message.priority)<<27) & 0x18000000;
        canId |= (static_cast<uint32_t>(message.sourceAddress)<<20) & 0x07F00000;
        canId |= (static_cast<uint32_t>(message.destinationAddress)<<13) & 0x000FE000;
        canId |= (static_cast<uint32_t>(message.protocol)<<7) & 0x00001F80;
        canId |= (static_cast<uint32_t>(message.command)<<4) & 0x00000070;
        canId |= (static_cast<uint32_t>(paddingLength)) & 0x0000000F;

        return canId;
    }

    static inline Message toMessage(uint32_t identifier, const QByteArray &data)
    {
        Message message;

        message.priority = static_cast<RoomBus::Priority>((identifier >>27)&0x03);
        message.sourceAddress = static_cast<uint8_t>(identifier >>20)&0x7F;
        message.destinationAddress = static_cast<uint8_t>(identifier >>13)&0x7F;
        message.protocol = static_cast<RoomBus::Protocol>((identifier >>7)&0x3F);
        message.command = static_cast<uint8_t>(identifier>>4)&0x07;

        uint8_t paddingLength = identifier &0x0F;

        message.data = data.mid(0, data.size()-paddingLength);

        return message;
    }

    static inline uint8_t unpackUint8(const QByteArray &data, uint32_t index)
    {
        return static_cast<uint8_t>(data.at(index));
    }

    static inline uint16_t unpackUint16(const QByteArray &data, uint32_t index)
    {
        uint16_t temp = 0;
        temp |= (static_cast<uint16_t>(data.at(index))<<8)&0xFF00;
        temp |= (static_cast<uint16_t>(data.at(index+1))<<0)&0x00FF;
        return temp;
    }

    static inline uint32_t unpackUint32(const QByteArray &data, uint32_t index)
    {
        uint32_t temp = 0;
        temp =  (static_cast<uint32_t>(data.at(index))<<24)&0xFF000000;
        temp |= (static_cast<uint32_t>(data.at(index+1))<<16)&0x00FF0000;
        temp |= (static_cast<uint32_t>(data.at(index+2))<<8)&0x0000FF00;
        temp |= (static_cast<uint32_t>(data.at(index+3))<<0)&0x000000FF;
        return temp;
    }

    static inline float unpackFloat32(const QByteArray &data, uint32_t index)
    {
        uint32_t temp = unpackUint32(data,index);
        float *out;
        out = (float*)&temp;
        return *out;
    }

    static inline QString unpackString(const QByteArray &data, uint32_t index)
    {
        QString output;
        for(uint8_t i = index; i< data.size() ;i++) {
            char temp = data.at(i);
            if(temp == 0) break;
            output += temp;
        }
        return output;
    }

    static inline QByteArray packUint16(uint16_t value)
    {
        QByteArray temp;
        temp.append(static_cast<char>(static_cast<uint8_t>(value>>8)));
        temp.append(static_cast<char>(static_cast<uint8_t>(value)));
        return temp;
    }

    static inline QByteArray packUint32(uint32_t value)
    {
        QByteArray temp;
        temp.append(static_cast<char>(static_cast<uint8_t>(value>>24)));
        temp.append(static_cast<char>(static_cast<uint8_t>(value>>16)));
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
