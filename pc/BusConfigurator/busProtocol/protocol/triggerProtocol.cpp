#include "triggerProtocol.h"
#include "busDevice.h"

TriggerSystemProtocol::TriggerSystemProtocol(RoomBusDevice *device)
    : ProtocolBase(device)
{
    _device->addProtocol(this);
}

void TriggerSystemProtocol::handleMessage(RoomBus::Message message)
{
    if(message.protocol != RoomBus::Protocol::TriggerSystemProtocol){
        return;
    }

    switch((RoomBus::TriggerSystemCommand)message.command){
        case RoomBus::TriggerSystemCommand::Trigger: _parseTrigger(message); break;
        case RoomBus::TriggerSystemCommand::SignalInformationReport: _parseSignalInformationReport(message); break;
        case RoomBus::TriggerSystemCommand::SlotInformationReport: _parseSlotInformationReport(message); break;

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

void TriggerSystemProtocol::_parseTrigger(RoomBus::Message message)
{
    QList<uint16_t> triggerSignals;
    
    for(uint8_t i = 0; i < message.data.size(); i+=2){
        triggerSignals.append(RoomBus::unpackUint16(message.data,i));
    }

    emit triggerSignalReceived(triggerSignals);
}

void TriggerSystemProtocol::_parseSignalInformationReport(RoomBus::Message message)
{
    TriggerSignal signal;
    signal.channel = RoomBus::unpackUint16(message.data,0);
    signal.description = message.data.remove(0,2);

    _triggerSignal[signal.channel] = signal;

    emit triggerSignalListChange();
}

void TriggerSystemProtocol::_parseSlotInformationReport(RoomBus::Message message)
{
    TriggerSlot slot;
    slot.channel = RoomBus::unpackUint16(message.data,0);
    slot.description = message.data.remove(0,2);
    slot.trigger = this;

    _triggerSlot[slot.channel] = slot;

    emit triggerSlotListChange();
}





