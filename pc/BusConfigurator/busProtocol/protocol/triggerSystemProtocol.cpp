#include "triggerSystemProtocol.h"
#include "busDevice.h"

TriggerSystemProtocol::TriggerSystemProtocol(RoomBusDevice *device)
    : ProtocolBase(device)
{
    _device->addProtocol(this);
}

void TriggerSystemProtocol::handleMessage(const MiniBus::Message &message)
{
    if(message.protocol != (MiniBus::Protocol)Protocol::TriggerSystemProtocol){
        return;
    }

    switch((Command)message.command){
        case Command::Trigger: _parseTrigger(message); break;
        case Command::SignalInformationReport: _parseSignalInformationReport(message); break;
        case Command::SlotInformationReport: _parseSlotInformationReport(message); break;

        case Command::Reserved0:
        case Command::Reserved1:
        case Command::Reserved2:
        case Command::SignalInformationRequest:
        case Command::SlotInformationRequest:
            break;
    }
}

void TriggerSystemProtocol::requestSignalInformation(void)
{
    MiniBus::Message msg;
    msg.protocol = (MiniBus::Protocol)Protocol::TriggerSystemProtocol;
    msg.command = (MiniBus::Command)Command::SignalInformationRequest;

    sendMessage(msg);
}

void TriggerSystemProtocol::requestSlotInformation(void)
{
    MiniBus::Message msg;
    msg.protocol = (MiniBus::Protocol)Protocol::TriggerSystemProtocol;
    msg.command = (MiniBus::Command)Command::SlotInformationRequest;

    sendMessage(msg);
}

void TriggerSystemProtocol::sendTrigger(QList<uint16_t> triggerChannels)
{
    MiniBus::Message msg;
    msg.protocol = (MiniBus::Protocol)Protocol::TriggerSystemProtocol;
    msg.command = (MiniBus::Command)Command::Trigger;

    for(uint16_t channel: triggerChannels){
        msg.data.append(MiniBus::packUint16(channel));
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

QString TriggerSystemProtocol::commandName(MiniBus::Command command)
{
    switch((Command)command){
        case Command::Trigger : return "Trigger";
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

QString TriggerSystemProtocol::dataDecoder(MiniBus::Command command, const QByteArray &data)
{
    Q_UNUSED(command);
    Q_UNUSED(data);
    return "Not implemented";
}

void TriggerSystemProtocol::_parseTrigger(MiniBus::Message message)
{
    QList<uint16_t> triggerSignals;
    
    for(uint8_t i = 0; i < message.data.size(); i+=2){
        triggerSignals.append(MiniBus::unpackUint16(message.data,i));
    }

    emit triggerSignalReceived(triggerSignals);
}

void TriggerSystemProtocol::_parseSignalInformationReport(MiniBus::Message message)
{
    TriggerSignal signal;
    signal.channel = MiniBus::unpackUint16(message.data,0);
    signal.description = message.data.remove(0,2);

    _triggerSignal[signal.channel] = signal;

    emit triggerSignalListChange();
}

void TriggerSystemProtocol::_parseSlotInformationReport(MiniBus::Message message)
{
    TriggerSlot slot;
    slot.channel = MiniBus::unpackUint16(message.data,0);
    slot.description = message.data.remove(0,2);
    slot.trigger = this;

    _triggerSlot[slot.channel] = slot;

    emit triggerSlotListChange();
}





