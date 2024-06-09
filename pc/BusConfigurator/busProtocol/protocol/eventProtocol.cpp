#include "eventProtocol.h"

eventProtocol::eventProtocol(busDevice *device):busProtocol(device)
{
}

void eventProtocol::pushData(busMessage msg)
{
    if(msg.protocol == Protocol::EventProtocol)
    {
        if(msg.command == 0x00) parseEventSignal(msg);
        if(msg.command == 0x01) parseEventSignalNameReport(msg);
        else if(msg.command == 0x02) parseEventSlotNameReport(msg);
    }
}

QList<Protocol> eventProtocol::protocol(void)
{
    QList<Protocol> temp;
    temp.append(EventProtocol);
    return temp;
}

void eventProtocol::requestEventSignalNames(void)
{
    busMessage msg;

    msg.protocol = Protocol::EventProtocol;
    msg.command = 0x03;

    for (uint8_t i = 0; i<64; i++)
    {
        msg.data.append(static_cast<char>(i));
    }

    sendMessage(msg);
}

void eventProtocol::requestEventSlotNames(void)
{
    busMessage msg;

    msg.protocol = Protocol::EventProtocol;
    msg.command = 0x04;

    for (uint8_t i = 0; i<64; i++)
    {
        msg.data.append(static_cast<char>(i));
    }

    sendMessage(msg);
}

void eventProtocol::setActiveState(uint8_t eventChannel, bool active)
{
    if(_eventSlots.contains(eventChannel))
    {
        _eventSlots[eventChannel].active = active;
    }

    sendEvent(eventChannel);
}

void eventProtocol::sendEvent(QList<uint8_t>eventChannels)
{
    busMessage msg;

    msg.protocol = Protocol::EventProtocol;
    msg.command = 0x00;

    for (uint8_t i = 0; i<eventChannels.size(); i++)
    {
        msg.data.append(static_cast<char>(eventChannels.at(i)));
    }

    sendMessage(msg);
}

void eventProtocol::sendEvent(uint8_t eventChannel)
{
   QList<uint8_t>eventChannels;
   eventChannels.append(eventChannel);

   sendEvent(eventChannels);
}

QList<eventSlot*> eventProtocol::eventSlots()
{
    QList<eventSlot*> temp;

    QMapIterator<uint8_t, eventSlot> i(_eventSlots);

    while (i.hasNext())
    {
        i.next();

        temp.append(&_eventSlots[i.key()]);
    }

    return temp;
}

void eventProtocol::parseEventSignal(busMessage msg)
{
    QList<uint8_t> eventrSignals;

    for (uint8_t i = 0; i<msg.data.size(); i++)
    {
        eventrSignals.append(static_cast<uint8_t>(msg.data.at(i)));
    }

    emit eventSignalReceived(eventrSignals);
}

void eventProtocol::parseEventSignalNameReport(busMessage msg)
{
    uint8_t eventChannel = static_cast<uint8_t>(msg.data.at(0));
    QString name = msg.data.remove(0,1);

    _eventSlots[eventChannel].name = name;

    emit eventSignalListChange();
}

void eventProtocol::parseEventSlotNameReport(busMessage msg)
{
    uint8_t eventChannel = static_cast<uint8_t>(msg.data.at(0)); 
    uint16_t timeout = 0;
    timeout |= (static_cast<uint16_t>(msg.data.at(1)) << 8);
    timeout |= (static_cast<uint16_t>(msg.data.at(2)) & 0xFF);
    QString name = msg.data.remove(0,3);

    _eventSlots[eventChannel].name = name;
    _eventSlots[eventChannel].timeout = timeout;
    _eventSlots[eventChannel].channel = eventChannel;

    emit eventSlotListChange();
}





