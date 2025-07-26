#include "eventProtocol.h"

EventSystemProtocol::EventSystemProtocol(RoomBusDevice *device)
    : ProtocolBase(device)
{
}

void EventSystemProtocol::pushData(RoomBus::Message msg)
{
    if(msg.protocol != RoomBus::Protocol::EventSystemProtocol){
        return;
    }

    switch((RoomBus::EventSystemCommand)msg.command){
        case RoomBus::EventSystemCommand::Event: _parseEvent(msg); break;
        case RoomBus::EventSystemCommand::SignalInformationReport: _parseSignalInformationReport(msg); break;
        case RoomBus::EventSystemCommand::SlotInformationReport: _parseSlotInformationReport(msg); break;

        case RoomBus::EventSystemCommand::Reserved0:
        case RoomBus::EventSystemCommand::Reserved1:
        case RoomBus::EventSystemCommand::Reserved2:
        case RoomBus::EventSystemCommand::SignalInformationRequest:
        case RoomBus::EventSystemCommand::SlotInformationRequest:
            break;
    }
}

QList<RoomBus::Protocol> EventSystemProtocol::protocol(void)
{
    QList<RoomBus::Protocol> output;
    output.append(RoomBus::Protocol::EventSystemProtocol);
    return output;
}

void EventSystemProtocol::requestSignalInformation(void)
{
    RoomBus::Message msg;
    msg.protocol = RoomBus::Protocol::EventSystemProtocol;
    msg.command = (RoomBus::Command)RoomBus::EventSystemCommand::SignalInformationRequest;

    sendMessage(msg);
}

void EventSystemProtocol::requestSlotInformation(void)
{
    RoomBus::Message msg;
    msg.protocol = RoomBus::Protocol::EventSystemProtocol;
    msg.command = (RoomBus::Command)RoomBus::EventSystemCommand::SlotInformationRequest;

    sendMessage(msg);
}

void EventSystemProtocol::clearInformation()
{
    _eventSlot.clear();
    _eventSignal.clear();

    emit eventSignalListChange();
    emit eventSlotListChange();
}

void EventSystemProtocol::sendEvent(QList<uint16_t>eventChannels)
{
    RoomBus::Message msg;
    msg.protocol = RoomBus::Protocol::EventSystemProtocol;
    msg.command = (RoomBus::Command)RoomBus::EventSystemCommand::Event;

    for(const uint16_t &channel: eventChannels){
        msg.data.append(RoomBus::packUint16(channel));
    }

    sendMessage(msg);
}

void EventSystemProtocol::sendEvent(uint16_t eventChannel)
{
   QList<uint16_t>eventChannels;
   eventChannels.append(eventChannel);

   sendEvent(eventChannels);
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
    for(EventSystemProtocol::EventSignal &item: _eventSignal){
        output.append(&item);
    }
    return output;
}

void EventSystemProtocol::_parseEvent(RoomBus::Message msg)
{
    QList<uint16_t> eventSignal;

    for(uint8_t i = 0; i< msg.data.size(); i+= 2){
        eventSignal.append(RoomBus::unpackUint16(msg.data,i));
    }

    emit eventSignalReceived(eventSignal);
}

void EventSystemProtocol::_parseSignalInformationReport(RoomBus::Message msg)
{
    EventSignal signal;
    signal.channel = RoomBus::unpackUint16(msg.data,0);
    signal.interval = RoomBus::unpackUint16(msg.data,2);
    signal.description = msg.data.remove(0,4);
    _eventSignal[signal.channel] = signal;

    emit eventSignalListChange();
}

void EventSystemProtocol::_parseSlotInformationReport(RoomBus::Message msg)
{
    EventSlot slot;
    slot.channel = RoomBus::unpackUint16(msg.data,0);
    slot.timeout = RoomBus::unpackUint16(msg.data,2);
    slot.description = msg.data.remove(0,4);
    _eventSlot[slot.channel] = slot;

    emit eventSlotListChange();
}





