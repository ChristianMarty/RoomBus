#include "triggerProtocol.h"

TriggerProtocol::TriggerProtocol(busDevice *device):BusProtocol(device)
{
}

void TriggerProtocol::pushData(BusMessage msg)
{
    if(msg.protocol == Protocol::TriggerProtocol)
    {
        if(msg.command == Command::Trigger) _parseTrigger(msg);
        else if(msg.command == Command::SignalInformationReport) _parseSignalInformationReport(msg);
        else if(msg.command == Command::SlotInformationReport) _parseSlotInformationReport(msg);
    }
}

QList<Protocol> TriggerProtocol::protocol(void)
{
    QList<Protocol> temp;
    temp.append(Protocol::TriggerProtocol);
    return temp;
}

void TriggerProtocol::requestSignalInformation(void)
{
    BusMessage msg;

    msg.protocol = Protocol::TriggerProtocol;
    msg.command = Command::SignalInformationRequest;

    sendMessage(msg);
}

void TriggerProtocol::requestSlotInformation(void)
{
    BusMessage msg;

    msg.protocol = Protocol::TriggerProtocol;
    msg.command = Command::SlotInformationRequest;

    sendMessage(msg);
}

void TriggerProtocol::sendTrigger(QList<uint16_t> triggerChannels)
{
    BusMessage msg;

    msg.protocol = Protocol::TriggerProtocol;
    msg.command = Command::Trigger;

    for (uint8_t i = 0; i<triggerChannels.size(); i++)
    {
        uint16_t channel = triggerChannels.at(i);
        msg.data.append(packUint16(channel));
    }

    sendMessage(msg);
}

void TriggerProtocol::sendTrigger(uint16_t triggerChannel)
{
   QList<uint16_t>triggerChannels;
   triggerChannels.append(triggerChannel);

   sendTrigger(triggerChannels);
}

void TriggerProtocol::emulateTrigger(uint16_t triggerChannel)
{

}

void TriggerProtocol::reset()
{
    _triggerSlot.clear();
    _triggerSignal.clear();

    emit triggerSignalListChange();
    emit triggerSlotListChange();
}

QList<TriggerProtocol::TriggerSlot *> TriggerProtocol::triggerSlots()
{
    QList<TriggerProtocol::TriggerSlot *> output;
    for(auto &item: _triggerSlot){
        output.append(&item);
    }
    return output;
}

QList<TriggerProtocol::TriggerSignal *> TriggerProtocol::triggerSignls()
{
    QList<TriggerProtocol::TriggerSignal *> output;
    for(auto &item: _triggerSignal){
        output.append(&item);
    }
    return output;
}

QMap<uint16_t, TriggerProtocol::TriggerSlot> TriggerProtocol::triggerSlotMap() const
{
    return _triggerSlot;
}

QMap<uint16_t, TriggerProtocol::TriggerSignal> TriggerProtocol::triggerSignalMap() const
{
    return _triggerSignal;
}

void TriggerProtocol::_parseTrigger(BusMessage msg)
{
    QList<uint8_t> triggerSignals;
    
    for (uint8_t i = 0; i<msg.data.size(); i++)
    {
        triggerSignals.append(static_cast<uint8_t>(msg.data.at(i)));
    }

    emit triggerSignalReceived(triggerSignals);
}

void TriggerProtocol::_parseSignalInformationReport(BusMessage msg)
{
    TriggerSignal signal;
    signal.channel = getUint16(msg.data,0);
    signal.description = msg.data.remove(0,2);

    _triggerSignal[signal.channel] = signal;

    emit triggerSignalListChange();
}

void TriggerProtocol::_parseSlotInformationReport(BusMessage msg)
{
    TriggerSlot slot;
    slot.channel = getUint16(msg.data,0);
    slot.description = msg.data.remove(0,2);
    slot.trigger = this;

    _triggerSlot[slot.channel] = slot;

    emit triggerSlotListChange();
}





