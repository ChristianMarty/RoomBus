#include "eventProtocol.h"

EventSystemProtocol::EventSystemProtocol(busDevice *device):BusProtocol(device)
{
}

void EventSystemProtocol::pushData(RoomBus::Message msg)
{
    if(msg.protocol != RoomBus::Protocol::EventSystemProtocol) return;

    if(msg.command == (uint8_t)RoomBus::EventSystemCommand::Event) _parseEvent(msg);
    else if(msg.command == (uint8_t)RoomBus::EventSystemCommand::SignalInformationReport) _parseSignalInformationReport(msg);
    else if(msg.command == (uint8_t)RoomBus::EventSystemCommand::SlotInformationReport) _parseSlotInformationReport(msg);

}

QList<RoomBus::Protocol> EventSystemProtocol::protocol(void)
{
    QList<RoomBus::Protocol> temp;
    temp.append(RoomBus::Protocol::EventSystemProtocol);
    return temp;
}

void EventSystemProtocol::requestSignalInformation(void)
{
    RoomBus::Message msg;

    msg.protocol = RoomBus::Protocol::EventSystemProtocol;
    msg.command = (uint8_t)RoomBus::EventSystemCommand::SignalInformationRequest;

    sendMessage(msg);
}

void EventSystemProtocol::requestSlotInformation(void)
{
    RoomBus::Message msg;

    msg.protocol = RoomBus::Protocol::EventSystemProtocol;
    msg.command = (uint8_t)RoomBus::EventSystemCommand::SlotInformationRequest;

    sendMessage(msg);
}

void EventSystemProtocol::setActiveState(uint8_t eventChannel, bool active)
{
  /*  if(_eventSlots.contains(eventChannel))
    {
        _eventSlots[eventChannel].active = active;
    }

    sendEvent(eventChannel);*/
}

void EventSystemProtocol::sendEvent(QList<uint8_t>eventChannels)
{
    RoomBus::Message msg;

    msg.protocol = RoomBus::Protocol::EventSystemProtocol;
    msg.command = 0x00;

    for (uint8_t i = 0; i<eventChannels.size(); i++)
    {
        msg.data.append(static_cast<char>(eventChannels.at(i)));
    }

    sendMessage(msg);
}

void EventSystemProtocol::sendEvent(uint8_t eventChannel)
{
   QList<uint8_t>eventChannels;
   eventChannels.append(eventChannel);

   sendEvent(eventChannels);
}

void EventSystemProtocol::reset()
{
    _eventSlot.clear();
    _eventSignal.clear();

    emit eventSignalListChange();
    emit eventSlotListChange();
}

QList<EventSystemProtocol::EventSlot *> EventSystemProtocol::eventSlots()
{
    QList<EventSystemProtocol::EventSlot *> output;
    for(auto &item: _eventSlot){
        output.append(&item);
    }
    return output;
}

QList<EventSystemProtocol::EventSignal *> EventSystemProtocol::eventSignls()
{
    QList<EventSystemProtocol::EventSignal *> output;
    for(auto &item: _eventSignal){
        output.append(&item);
    }
    return output;
}

void EventSystemProtocol::_parseEvent(RoomBus::Message msg)
{
    QList<uint8_t> eventrSignals;

    for (uint8_t i = 0; i<msg.data.size(); i++)
    {
        eventrSignals.append(static_cast<uint8_t>(msg.data.at(i)));
    }

    emit eventSignalReceived(eventrSignals);
}

void EventSystemProtocol::_parseSignalInformationReport(RoomBus::Message msg)
{
    EventSignal signal;
    signal.channel = getUint16(msg.data,0);
    signal.interval = getUint16(msg.data,2);
    signal.description = msg.data.remove(0,4);

    _eventSignal[signal.channel] = signal;

    emit eventSignalListChange();
}

void EventSystemProtocol::_parseSlotInformationReport(RoomBus::Message msg)
{
    EventSlot slot;
    slot.channel = getUint16(msg.data,0);
    slot.timeout = getUint16(msg.data,2);
    slot.description = msg.data.remove(0,4);

    _eventSlot[slot.channel] = slot;

    emit eventSlotListChange();
}





