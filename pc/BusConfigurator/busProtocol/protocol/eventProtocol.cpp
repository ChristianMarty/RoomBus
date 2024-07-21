#include "eventProtocol.h"

EventProtocol::EventProtocol(busDevice *device):BusProtocol(device)
{
}

void EventProtocol::pushData(busMessage msg)
{
    if(msg.protocol == Protocol::EventProtocol)
    {
        if(msg.command == Command::Event) _parseEvent(msg);
        else if(msg.command == Command::SignalInformationReport) _parseSignalInformationReport(msg);
        else if(msg.command == Command::SlotInformationReport) _parseSlotInformationReport(msg);
    }
}

QList<Protocol> EventProtocol::protocol(void)
{
    QList<Protocol> temp;
    temp.append(Protocol::EventProtocol);
    return temp;
}

void EventProtocol::requestSignalInformation(void)
{
    busMessage msg;

    msg.protocol = Protocol::EventProtocol;
    msg.command = Command::SignalInformationRequest;

    sendMessage(msg);
}

void EventProtocol::requestSlotInformation(void)
{
    busMessage msg;

    msg.protocol = Protocol::EventProtocol;
    msg.command = Command::SlotInformationRequest;

    sendMessage(msg);
}

void EventProtocol::setActiveState(uint8_t eventChannel, bool active)
{
  /*  if(_eventSlots.contains(eventChannel))
    {
        _eventSlots[eventChannel].active = active;
    }

    sendEvent(eventChannel);*/
}

void EventProtocol::sendEvent(QList<uint8_t>eventChannels)
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

void EventProtocol::sendEvent(uint8_t eventChannel)
{
   QList<uint8_t>eventChannels;
   eventChannels.append(eventChannel);

   sendEvent(eventChannels);
}

void EventProtocol::reset()
{
    _eventSlot.clear();
    _eventSignal.clear();

    emit eventSignalListChange();
    emit eventSlotListChange();
}

QList<EventProtocol::EventSlot *> EventProtocol::eventSlots()
{
    QList<EventProtocol::EventSlot *> output;
    for(auto &item: _eventSlot){
        output.append(&item);
    }
    return output;
}

QList<EventProtocol::EventSignal *> EventProtocol::eventSignls()
{
    QList<EventProtocol::EventSignal *> output;
    for(auto &item: _eventSignal){
        output.append(&item);
    }
    return output;
}

void EventProtocol::_parseEvent(busMessage msg)
{
    QList<uint8_t> eventrSignals;

    for (uint8_t i = 0; i<msg.data.size(); i++)
    {
        eventrSignals.append(static_cast<uint8_t>(msg.data.at(i)));
    }

    emit eventSignalReceived(eventrSignals);
}

void EventProtocol::_parseSignalInformationReport(busMessage msg)
{
    EventSignal signal;
    signal.channel = getUint16(msg.data,0);
    signal.interval = getUint16(msg.data,2);
    signal.description = msg.data.remove(0,4);

    _eventSignal[signal.channel] = signal;

    emit eventSignalListChange();
}

void EventProtocol::_parseSlotInformationReport(busMessage msg)
{
    EventSlot slot;
    slot.channel = getUint16(msg.data,0);
    slot.timeout = getUint16(msg.data,2);
    slot.description = msg.data.remove(0,4);

    _eventSlot[slot.channel] = slot;

    emit eventSlotListChange();
}





