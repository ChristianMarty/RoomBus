#include "stateReportProtocol.h"

StateReportProtocol::StateReportProtocol(busDevice *device):BusProtocol(device)
{
}

void StateReportProtocol::pushData(BusMessage msg)
{
    if(msg.protocol == Protocol::StateReportProtocol)
    {
        if(msg.command == State) _parseStateReport(msg);
        else if(msg.command == SignalInformationReport) _parseSignalInformationReport(msg);
        else if(msg.command == SlotInformationReport) _parseSlotInformationReport(msg);
    }
}

QList<Protocol> StateReportProtocol::protocol(void)
{
    QList<Protocol> temp;
    temp.append(Protocol::StateReportProtocol);
    return temp;
}

void StateReportProtocol::requestSignalInformation()
{
    BusMessage msg;

    msg.protocol = Protocol::StateReportProtocol;
    msg.command = Command::SignalInformationRequest;

    sendMessage(msg);
}

void StateReportProtocol::requestSlotInformation()
{
    BusMessage msg;

    msg.protocol = Protocol::StateReportProtocol;
    msg.command = Command::SlotInformationRequest;

    sendMessage(msg);
}

void StateReportProtocol::requestAllState()
{
    BusMessage msg;

    msg.protocol = Protocol::StateReportProtocol;
    msg.command = Command::StateRequest;

    sendMessage(msg);
}

void StateReportProtocol::reset(void)
{
    _stateReportSignal.clear();
    _stateReportSlot.clear();
}

QList<StateReportProtocol::StateReportSignal *> StateReportProtocol::stateReportSignls()
{
    QList<StateReportProtocol::StateReportSignal *> output;
    for(auto &item: _stateReportSignal){
        output.append(&item);
    }
    return output;
}

QList<StateReportProtocol::StateReportSlot *> StateReportProtocol::stateReportSlots()
{
    QList<StateReportProtocol::StateReportSlot *> output;
    for(auto &item: _stateReportSlot){
        output.append(&item);
    }
    return output;
}

QMap<uint16_t, StateReportProtocol::StateReportSignal> StateReportProtocol::stateReportSignalMap() const
{
    return _stateReportSignal;
}

QMap<uint16_t, StateReportProtocol::StateReportSlot> StateReportProtocol::stateReportSlotMap() const
{
    return _stateReportSlot;
}

void StateReportProtocol::_parseStateReport(BusMessage msg)
{
    for(uint8_t i = 0; i < msg.data.length(); i+=3)
    {
        uint16_t channel = getUint16(msg.data.mid(i,2), 0);
        StateReportProtocol::SignalState state = (StateReportProtocol::SignalState) msg.data.at(i+2);

        _signalState[channel] = state;
        emit signalStateChnage(channel, state);
    }
}

void StateReportProtocol::_parseSignalInformationReport(BusMessage msg)
{
    StateReportSignal signal;
    signal.channel = getUint16(msg.data,0);
    signal.interval = getUint16(msg.data,2);
    signal.description = msg.data.remove(0,4);

    _stateReportSignal[signal.channel] = signal;

    emit signalListChange();
}

void StateReportProtocol::_parseSlotInformationReport(BusMessage msg)
{
    StateReportSlot slot;
    slot.channel = getUint16(msg.data,0);
    slot.timeout = getUint16(msg.data,2);
    slot.description = msg.data.remove(0,4);

    _stateReportSlot[slot.channel] = slot;

    emit slotListChange();
}
