#include "messageLogProtocol.h"

MessageLogProtocol::MessageLogProtocol(busDevice *device):BusProtocol(device)
{
}

void MessageLogProtocol::pushData(busMessage msg)
{
    if(msg.protocol != Protocol::MessageLogProtocolId) return;

    LogMessage temp;

    if(msg.command == 0x00) temp.messageType = LogMessage::sysMessage;
    else if(msg.command == 0x01) temp.messageType = LogMessage::sysWarning;
    else if(msg.command == 0x02) temp.messageType = LogMessage::sysError;
    else if(msg.command == 0x03) temp.messageType = LogMessage::sysReserved;
    else if(msg.command == 0x04) temp.messageType = LogMessage::appMessage;
    else if(msg.command == 0x05) temp.messageType = LogMessage::appWarning;
    else if(msg.command == 0x06) temp.messageType = LogMessage::appError;
    else if(msg.command == 0x07) temp.messageType = LogMessage::appReserved;

    temp.messageCode = BusProtocol::getUint32(msg.data, 0);

    QString temp2 = "";

    for(uint8_t i= 0; i<(msg.data.size()-1);i++)
    {
        char temp = msg.data.at(1+i);

        // *** Legacy: Remove after kernel update
        if(i==0&&temp == 0)continue;
        if(i==1&&temp == 0)continue;
        if(i==2&&temp == 0)continue;
        // ***

        if(temp == 0) break;
        temp2 += temp;
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

QList<Protocol> MessageLogProtocol::protocol(void)
{
    QList<Protocol> temp;
    temp.append(Protocol::MessageLogProtocolId);
    return temp;
}

QList<LogMessage> MessageLogProtocol::messages() const
{
    return _messages;
}

