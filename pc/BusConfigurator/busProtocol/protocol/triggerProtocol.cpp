#include "triggerProtocol.h"
#include "busDevice.h"

TriggerSystemProtocol::TriggerSystemProtocol(RoomBusDevice *device)
    : ProtocolBase(device)
{
    _device->addProtocol(this);
}

void TriggerSystemProtocol::handleMessage(RoomBus::Message msg)
{
    if(msg.protocol != RoomBus::Protocol::TriggerSystemProtocol){
        return;
    }

    switch((RoomBus::TriggerSystemCommand)msg.command){
        case RoomBus::TriggerSystemCommand::Trigger: _parseTrigger(msg); break;
        case RoomBus::TriggerSystemCommand::SignalInformationReport: _parseSignalInformationReport(msg); break;
        case RoomBus::TriggerSystemCommand::SlotInformationReport: _parseSlotInformationReport(msg); break;

        case RoomBus::TriggerSystemCommand::Reserved0:
        case RoomBus::TriggerSystemCommand::Reserved1:
        case RoomBus::TriggerSystemCommand::Reserved2:
        case RoomBus::TriggerSystemCommand::SignalInformationRequest:
        case RoomBus::TriggerSystemCommand::SlotInformationRequest:
            break;
    }
}

void TriggerSystemProtocol::requestSignalInformation(void)
{
    RoomBus::Message msg;
    msg.protocol = RoomBus::Protocol::TriggerSystemProtocol;
    msg.command = (RoomBus::Command)RoomBus::TriggerSystemCommand::SignalInformationRequest;

    sendMessage(msg);
}

void TriggerSystemProtocol::requestSlotInformation(void)
{
    RoomBus::Message msg;
    msg.protocol = RoomBus::Protocol::TriggerSystemProtocol;
    msg.command = (RoomBus::Command)RoomBus::TriggerSystemCommand::SlotInformationRequest;

    sendMessage(msg);
}

void TriggerSystemProtocol::sendTrigger(QList<uint16_t> triggerChannels)
{
    RoomBus::Message msg;
    msg.protocol = RoomBus::Protocol::TriggerSystemProtocol;
    msg.command = (RoomBus::Command)RoomBus::TriggerSystemCommand::Trigger;

    for(uint16_t channel: triggerChannels){
        msg.data.append(RoomBus::packUint16(channel));
    }

    sendMessage(msg);
}

void TriggerSystemProtocol::sendTrigger(uint16_t triggerChannel)
{
   QList<uint16_t>triggerChannels;
   triggerChannels.append(triggerChannel);

   sendTrigger(triggerChannels);
}

void TriggerSystemProtocol::clearInformation()
{
    _triggerSlot.clear();
    _triggerSignal.clear();

    emit triggerSignalListChange();
    emit triggerSlotListChange();
}

QList<TriggerSystemProtocol::TriggerSlot *> TriggerSystemProtocol::triggerSlots()
{
    QList<TriggerSystemProtocol::TriggerSlot *> output;
    for(TriggerSystemProtocol::TriggerSlot &item: _triggerSlot){
        output.append(&item);
    }
    return output;
}

QList<TriggerSystemProtocol::TriggerSignal *> TriggerSystemProtocol::triggerSignls()
{
    QList<TriggerSystemProtocol::TriggerSignal *> output;
    for(TriggerSystemProtocol::TriggerSignal &item: _triggerSignal){
        output.append(&item);
    }
    return output;
}

QMap<uint16_t, TriggerSystemProtocol::TriggerSlot> TriggerSystemProtocol::triggerSlotMap() const
{
    return _triggerSlot;
}

QMap<uint16_t, TriggerSystemProtocol::TriggerSignal> TriggerSystemProtocol::triggerSignalMap() const
{
    return _triggerSignal;
}

void TriggerSystemProtocol::_parseTrigger(RoomBus::Message msg)
{
    QList<uint16_t> triggerSignals;
    
    for(uint8_t i = 0; i < msg.data.size(); i+=2){
        triggerSignals.append(RoomBus::unpackUint16(msg.data,i));
    }

    emit triggerSignalReceived(triggerSignals);
}

void TriggerSystemProtocol::_parseSignalInformationReport(RoomBus::Message msg)
{
    TriggerSignal signal;
    signal.channel = RoomBus::unpackUint16(msg.data,0);
    signal.description = msg.data.remove(0,2);

    _triggerSignal[signal.channel] = signal;

    emit triggerSignalListChange();
}

void TriggerSystemProtocol::_parseSlotInformationReport(RoomBus::Message msg)
{
    TriggerSlot slot;
    slot.channel = RoomBus::unpackUint16(msg.data,0);
    slot.description = msg.data.remove(0,2);
    slot.trigger = this;

    _triggerSlot[slot.channel] = slot;

    emit triggerSlotListChange();
}





