#include "messageLogProtocol.h"

MessageLogProtocol::MessageLogProtocol(RoomBusDevice *device):BusProtocol(device)
{
}

void MessageLogProtocol::pushData(RoomBus::Message msg)
{
    if(msg.protocol != RoomBus::Protocol::MessageLogProtocolId) return;
    if(msg.data.size() < 8) return;

    LogMessage temp;

    if(msg.command == 0x00) temp.messageType = LogMessage::sysMessage;
    else if(msg.command == 0x01) temp.messageType = LogMessage::sysWarning;
    else if(msg.command == 0x02) temp.messageType = LogMessage::sysError;
    else if(msg.command == 0x03) temp.messageType = LogMessage::sysReserved;
    else if(msg.command == 0x04) temp.messageType = LogMessage::appMessage;
    else if(msg.command == 0x05) temp.messageType = LogMessage::appWarning;
    else if(msg.command == 0x06) temp.messageType = LogMessage::appError;
    else if(msg.command == 0x07) temp.messageType = LogMessage::appReserved;

    temp.tickTime = RoomBus::unpackUint32(msg.data, 0);
    temp.messageCode = RoomBus::unpackUint32(msg.data, 4);

    QString temp2 = "";

    for(uint8_t i = 8; i<msg.data.size(); i++)
    {
        char byte = msg.data.at(i);
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

QList<RoomBus::Protocol> MessageLogProtocol::protocol(void)
{
    QList<RoomBus::Protocol> temp;
    temp.append(RoomBus::Protocol::MessageLogProtocolId);
    return temp;
}

QList<LogMessage> MessageLogProtocol::messages() const
{
    return _messages;
}

