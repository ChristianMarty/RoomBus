#include "messageLogProtocol.h"
#include "busDevice.h"

MessageLogProtocol::MessageLogProtocol(RoomBusDevice *device)
    : ProtocolBase(device)
{
    _device->addProtocol(this);
}

void MessageLogProtocol::handleMessage(const MiniBus::Message &message)
{
    if(message.protocol != (MiniBus::Protocol)Protocol::MessageLogProtocolId){
        return;
    }
    if(message.data.size() < 8){
        return;
    }

    LogMessage temp;
    if(message.command == 0x00) temp.messageType = LogMessage::sysMessage;
    else if(message.command == 0x01) temp.messageType = LogMessage::sysWarning;
    else if(message.command == 0x02) temp.messageType = LogMessage::sysError;
    else if(message.command == 0x03) temp.messageType = LogMessage::sysReserved;
    else if(message.command == 0x04) temp.messageType = LogMessage::appMessage;
    else if(message.command == 0x05) temp.messageType = LogMessage::appWarning;
    else if(message.command == 0x06) temp.messageType = LogMessage::appError;
    else if(message.command == 0x07) temp.messageType = LogMessage::appReserved;

    temp.tickTime = MiniBus::unpackUint32(message.data, 0);
    temp.messageCode = MiniBus::unpackUint32(message.data, 4);

    QString temp2 = "";

    for(uint8_t i = 8; i<message.data.size(); i++)
    {
        char byte = message.data.at(i);
        if(byte == 0) break;
        temp2 += byte;
    }

    temp.massage = temp2;

    _messages.append(temp);
    if(_messages.size() > 1000) _messages.removeFirst();

    emit newMessage(temp);
}

void MessageLogProtocol::clearLog(void)
{
    _messages.clear();
}

QString MessageLogProtocol::commandName(MiniBus::Command command)
{
    switch((Command)command){
        case Command::SystemMessage: return "System Message"; break;
        case Command::SystemWarning: return "System Warning"; break;
        case Command::SystemError: return "System Error"; break;
        case Command::ApplicationMessage: return "Application Message"; break;
        case Command::ApplicationWarning: return "Application Warning"; break;
        case Command::ApplicationError: return "Application Error"; break;
    }

    return "Unknown Command";
}

QString MessageLogProtocol::dataDecoder(MiniBus::Command command, const QByteArray &data)
{
    Q_UNUSED(command);
    Q_UNUSED(data);
    return "Not implemented";
}

QList<LogMessage> MessageLogProtocol::messages() const
{
    return _messages;
}

