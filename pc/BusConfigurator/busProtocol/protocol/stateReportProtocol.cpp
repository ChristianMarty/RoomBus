#include "stateReportProtocol.h"

StateSystemProtocol::StateSystemProtocol(RoomBusDevice *device):BusProtocol(device)
{
}

void StateSystemProtocol::pushData(RoomBus::Message msg)
{
    if(msg.protocol != RoomBus::Protocol::StateSystemProtocol) return;

    switch((RoomBus::StateSystemCommand)msg.command)
    {
        case RoomBus::StateSystemCommand::State: _parseStateReport(msg); break;
        case RoomBus::StateSystemCommand::SignalInformationReport: _parseSignalInformationReport(msg); break;
        case RoomBus::StateSystemCommand::SlotInformationReport: _parseSlotInformationReport(msg); break;
    }
}

QList<RoomBus::Protocol> StateSystemProtocol::protocol(void)
{
    QList<RoomBus::Protocol> temp;
    temp.append(RoomBus::Protocol::StateSystemProtocol);
    return temp;
}

void StateSystemProtocol::requestSignalInformation()
{
    RoomBus::Message msg;

    msg.protocol = RoomBus::Protocol::StateSystemProtocol;
    msg.command = (uint8_t)RoomBus::StateSystemCommand::SignalInformationRequest;

    sendMessage(msg);
}

void StateSystemProtocol::requestSlotInformation()
{
    RoomBus::Message msg;

    msg.protocol = RoomBus::Protocol::StateSystemProtocol;
    msg.command = (uint8_t)RoomBus::StateSystemCommand::SlotInformationRequest;

    sendMessage(msg);
}

void StateSystemProtocol::requestAllState()
{
    RoomBus::Message msg;

    msg.protocol = RoomBus::Protocol::StateSystemProtocol;
    msg.command = (uint8_t)RoomBus::StateSystemCommand::StateRequest;

    sendMessage(msg);
}

void StateSystemProtocol::reset(void)
{
    _stateReportSignal.clear();
    _stateReportSlot.clear();
}

QList<StateSystemProtocol::StateReportSignal *> StateSystemProtocol::stateReportSignls()
{
    QList<StateSystemProtocol::StateReportSignal *> output;
    for(auto &item: _stateReportSignal){
        output.append(&item);
    }
    return output;
}

QList<StateSystemProtocol::StateReportSlot *> StateSystemProtocol::stateReportSlots()
{
    QList<StateSystemProtocol::StateReportSlot *> output;
    for(auto &item: _stateReportSlot){
        output.append(&item);
    }
    return output;
}

QMap<uint16_t, StateSystemProtocol::StateReportSignal> StateSystemProtocol::stateReportSignalMap() const
{
    return _stateReportSignal;
}

QMap<uint16_t, StateSystemProtocol::StateReportSlot> StateSystemProtocol::stateReportSlotMap() const
{
    return _stateReportSlot;
}

void StateSystemProtocol::_parseStateReport(RoomBus::Message msg)
{
    for(uint8_t i = 0; i < msg.data.length(); i+=3)
    {
        uint16_t channel = RoomBus::unpackUint16(msg.data.mid(i,2), 0);
        StateSystemProtocol::SignalState state = (StateSystemProtocol::SignalState) msg.data.at(i+2);

        _signalState[channel] = state;
        emit signalStateChnage(channel, state);
    }
}

void StateSystemProtocol::_parseSignalInformationReport(RoomBus::Message msg)
{
    StateReportSignal signal;
    signal.channel = RoomBus::unpackUint16(msg.data,0);
    signal.interval = RoomBus::unpackUint16(msg.data,2);
    signal.description = msg.data.remove(0,4);

    _stateReportSignal[signal.channel] = signal;

    emit signalListChange();
}

void StateSystemProtocol::_parseSlotInformationReport(RoomBus::Message msg)
{
    StateReportSlot slot;
    slot.channel = RoomBus::unpackUint16(msg.data,0);
    slot.timeout = RoomBus::unpackUint16(msg.data,2);
    slot.description = msg.data.remove(0,4);

    _stateReportSlot[slot.channel] = slot;

    emit slotListChange();
}
