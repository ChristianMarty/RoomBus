#include "triggerProtocol.h"

triggerProtocol::triggerProtocol(busDevice *device):busProtocol(device)
{
}

void triggerProtocol::pushData(busMessage msg)
{
    if(msg.protocol == Protocol::TriggerProtocol)
    {
        if(msg.command == 0x00) parseTriggerSignal(msg);
        else if(msg.command == 0x01) parseTriggerSignalNameReport(msg);
        else if(msg.command == 0x02) parseTriggerSlotNameReport(msg);
    }
}

QList<Protocol> triggerProtocol::protocol(void)
{
    QList<Protocol> temp;
    temp.append(Protocol::TriggerProtocol);
    return temp;
}

void triggerProtocol::requestTriggerSignalNames(void)
{
    busMessage msg;

    msg.protocol = Protocol::TriggerProtocol;
    msg.command = 0x03;

   /* for (uint8_t i = 0; i<64; i++)
    {
        msg.data.append(static_cast<char>(i));
    }*/

    sendMessage(msg);
}

void triggerProtocol::requestTriggerSlotNames(void)
{
    busMessage msg;

    msg.protocol = Protocol::TriggerProtocol;
    msg.command = 0x04;

   /* for (uint8_t i = 0; i<64; i++)
    {
        msg.data.append(static_cast<char>(i));
    }*/

    sendMessage(msg);
}

void triggerProtocol::sendTrigger(QList<uint8_t>triggerChannels)
{
    busMessage msg;

    msg.protocol = Protocol::TriggerProtocol;
    msg.command = 0x00;

    for (uint8_t i = 0; i<triggerChannels.size(); i++)
    {
        msg.data.append(static_cast<char>(triggerChannels.at(i)));
    }

    sendMessage(msg);
}

void triggerProtocol::sendTrigger(uint8_t triggerChannel)
{
   QList<uint8_t>triggerChannels;
   triggerChannels.append(triggerChannel);

   sendTrigger(triggerChannels);
}

void triggerProtocol::reset()
{
    _triggerSignalNames.clear();
    _triggerSlot.clear();
    _triggerDestinationAddress.clear();
}

QMap<uint8_t, QString> triggerProtocol::triggerSignalNames() const
{
    return _triggerSignalNames;
}

QMap<uint8_t, triggerProtocol::triggerSlot> triggerProtocol::triggerSlotMap() const
{
    return _triggerSlot;
}

void triggerProtocol::parseTriggerSignal(busMessage msg)
{
    QList<uint8_t> triggerSignals;
    
    for (uint8_t i = 0; i<msg.data.size(); i++)
    {
        triggerSignals.append(static_cast<uint8_t>(msg.data.at(i)));
    }

    emit triggerSignalReceived(triggerSignals);
}

void triggerProtocol::parseTriggerSignalNameReport(busMessage msg)
{
    uint8_t destinationAddress = static_cast<uint8_t>(msg.data.at(0));
    uint8_t triggerChannel = static_cast<uint8_t>(msg.data.at(1));
    QString name = msg.data.remove(0,2);
    _triggerSignalNames[triggerChannel] = name;
    _triggerDestinationAddress[triggerChannel] = destinationAddress;

    emit triggerSignalListChange();
}

void triggerProtocol::parseTriggerSlotNameReport(busMessage msg)
{
    triggerSlot slot;
    slot.sourceAddressFiler = static_cast<uint8_t>(msg.data.at(0));
    slot.channel = static_cast<uint8_t>(msg.data.at(1));
    slot.name = msg.data.remove(0,2);;

    _triggerSlot[slot.channel] = slot;

    emit triggerSlotListChange();
}





