#include "eventSystemProtocol.h"
#include "busDevice.h"

EventSystemProtocol::EventSystemProtocol(RoomBusDevice *device)
    : ProtocolBase(device)
{
    _device->addProtocol(this);
}

void EventSystemProtocol::handleMessage(MiniBus::Message msg)
{
    if(msg.protocol != (MiniBus::Protocol)Protocol::EventSystemProtocol){
        return;
    }

    switch((Command)msg.command){
        case Command::Event: _parseEvent(msg); break;
        case Command::SignalInformationReport: _parseSignalInformationReport(msg); break;
        case Command::SlotInformationReport: _parseSlotInformationReport(msg); break;

        case Command::Reserved0:
        case Command::Reserved1:
        case Command::Reserved2:
        case Command::SignalInformationRequest:
        case Command::SlotInformationRequest:
            break;
    }
}

void EventSystemProtocol::requestSignalInformation(void)
{
    MiniBus::Message msg;
    msg.protocol = (MiniBus::Protocol)Protocol::EventSystemProtocol;
    msg.command = (MiniBus::Command)Command::SignalInformationRequest;

    sendMessage(msg);
}

void EventSystemProtocol::requestSlotInformation(void)
{
    MiniBus::Message msg;
    msg.protocol = (MiniBus::Protocol)Protocol::EventSystemProtocol;
    msg.command = (MiniBus::Command)Command::SlotInformationRequest;

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
    MiniBus::Message msg;
    msg.protocol = (MiniBus::Protocol)Protocol::EventSystemProtocol;
    msg.command = (MiniBus::Command)Command::Event;

    for(const uint16_t &channel: eventChannels){
        msg.data.append(MiniBus::packUint16(channel));
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

QString EventSystemProtocol::commandName(MiniBus::Command command)
{
    switch((Command)command){
    case Command::Event : return "Event";
    case Command::Reserved0 :
    case Command::Reserved1 :
    case Command::Reserved2 : return "Reserved";

    case Command::SignalInformationReport : return "Signal Information Report";
    case Command::SlotInformationReport : return "Slot Information Report";
    case Command::SignalInformationRequest : return "Signal Information Request";
    case Command::SlotInformationRequest : return "Slot Information Request";
    }

    return "Unknown Command";
}

QString EventSystemProtocol::dataDecoder(MiniBus::Command command, const QByteArray &data)
{
    Q_UNUSED(command);
    Q_UNUSED(data);
    return "Not implemented";
}

void EventSystemProtocol::_parseEvent(MiniBus::Message msg)
{
    QList<uint16_t> eventSignal;

    for(uint8_t i = 0; i< msg.data.size(); i+= 2){
        eventSignal.append(MiniBus::unpackUint16(msg.data,i));
    }

    emit eventSignalReceived(eventSignal);
}

void EventSystemProtocol::_parseSignalInformationReport(MiniBus::Message msg)
{
    EventSignal signal;
    signal.channel = MiniBus::unpackUint16(msg.data,0);
    signal.interval = MiniBus::unpackUint16(msg.data,2);
    signal.description = msg.data.remove(0,4);
    _eventSignal[signal.channel] = signal;

    emit eventSignalListChange();
}

void EventSystemProtocol::_parseSlotInformationReport(MiniBus::Message msg)
{
    EventSlot slot;
    slot.channel = MiniBus::unpackUint16(msg.data,0);
    slot.timeout = MiniBus::unpackUint16(msg.data,2);
    slot.description = msg.data.remove(0,4);
    _eventSlot[slot.channel] = slot;

    emit eventSlotListChange();
}





