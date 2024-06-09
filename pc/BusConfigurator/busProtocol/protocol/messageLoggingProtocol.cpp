#include "messageLoggingProtocol.h"

messageLoggingProtocol::messageLoggingProtocol(busDevice *device):busProtocol(device)
{
}

void messageLoggingProtocol::pushData(busMessage msg)
{
    if(msg.protocol != Protocol::MessageLoggingProtocol) return;

    logMessage temp;

    if(msg.command == 0x00) temp.messageType = logMessage::sysMessage;
    else if(msg.command == 0x01) temp.messageType = logMessage::sysWarning;
    else if(msg.command == 0x02) temp.messageType = logMessage::sysError;
    else if(msg.command == 0x03) temp.messageType = logMessage::sysReserved;
    else if(msg.command == 0x04) temp.messageType = logMessage::appMessage;
    else if(msg.command == 0x05) temp.messageType = logMessage::appWarning;
    else if(msg.command == 0x06) temp.messageType = logMessage::appError;
    else if(msg.command == 0x07) temp.messageType = logMessage::appReserved;

    temp.messageCode = static_cast<uint8_t>(msg.data.at(0));

    QString temp2 = "";

    for(uint8_t i= 0; i<(msg.data.size()-1);i++)
    {
        char temp = msg.data.at(1+i);
        if(temp == 0) break;
        temp2 += temp;
    }

    temp.massage = temp2;

    _messages.append(temp);
    if(_messages.size() > 1000) _messages.removeFirst();

    emit newMessage(temp);
}

void messageLoggingProtocol::clearLog(void)
{
    _messages.clear();
}

QList<Protocol> messageLoggingProtocol::protocol(void)
{
    QList<Protocol> temp;
    temp.append(Protocol::MessageLoggingProtocol);
    return temp;
}

QList<logMessage> messageLoggingProtocol::messages() const
{
    return _messages;
}

