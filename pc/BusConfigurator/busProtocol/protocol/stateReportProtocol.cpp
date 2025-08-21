#include "stateReportProtocol.h"
#include "busDevice.h"

StateSystemProtocol::StateSystemProtocol(RoomBusDevice *device)
    : ProtocolBase(device)
{
    _device->addProtocol(this);
}

void StateSystemProtocol::handleMessage(MiniBus::Message msg)
{
    if(msg.protocol != (MiniBus::Protocol)Protocol::StateSystemProtocol) return;

    switch((Command)msg.command)
    {
        case Command::State: _parseStateReport(msg); break;
        case Command::SignalInformationReport: _parseSignalInformationReport(msg); break;
        case Command::SlotInformationReport: _parseSlotInformationReport(msg); break;
    }
}

void StateSystemProtocol::requestSignalInformation()
{
    MiniBus::Message msg;

    msg.protocol = (MiniBus::Protocol)Protocol::StateSystemProtocol;
    msg.command = (uint8_t)Command::SignalInformationRequest;

    sendMessage(msg);
}

void StateSystemProtocol::requestSlotInformation()
{
    MiniBus::Message msg;

    msg.protocol = (MiniBus::Protocol)Protocol::StateSystemProtocol;
    msg.command = (uint8_t)Command::SlotInformationRequest;

    sendMessage(msg);
}

void StateSystemProtocol::requestAllState()
{
    MiniBus::Message msg;

    msg.protocol = (MiniBus::Protocol)Protocol::StateSystemProtocol;
    msg.command = (uint8_t)Command::StateRequest;

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

QString StateSystemProtocol::commandName(MiniBus::Command command)
{
    switch((Command)command){
        case Command::State : return "State Report";
        case Command::StateRequest : return "State Report Request";
        case Command::Reserved0 :
        case Command::Reserved1 : return "Reserved";

        case Command::SignalInformationReport : return "Signal Information Report";
        case Command::SlotInformationReport : return "Slot Information Report";
        case Command::SignalInformationRequest : return "Signal Information Request";
        case Command::SlotInformationRequest : return "Slot Information Request";
    }

    return "Unknown Command";
}

QString StateSystemProtocol::dataDecoder(MiniBus::Command command, const QByteArray &data)
{
    Q_UNUSED(command);
    Q_UNUSED(data);
    return "Not implemented";
}

void StateSystemProtocol::_parseStateReport(MiniBus::Message msg)
{
    for(uint8_t i = 0; i < msg.data.length(); i+=3)
    {
        uint16_t channel = MiniBus::unpackUint16(msg.data.mid(i,2), 0);
        StateSystemProtocol::SignalState state = (StateSystemProtocol::SignalState) msg.data.at(i+2);

        _signalState[channel] = state;
        emit signalStateChnage(channel, state);
    }
}

void StateSystemProtocol::_parseSignalInformationReport(MiniBus::Message msg)
{
    StateReportSignal signal;
    signal.channel = MiniBus::unpackUint16(msg.data,0);
    signal.interval = MiniBus::unpackUint16(msg.data,2);
    signal.description = msg.data.remove(0,4);

    _stateReportSignal[signal.channel] = signal;

    emit signalListChange();
}

void StateSystemProtocol::_parseSlotInformationReport(MiniBus::Message msg)
{
    StateReportSlot slot;
    slot.channel = MiniBus::unpackUint16(msg.data,0);
    slot.timeout = MiniBus::unpackUint16(msg.data,2);
    slot.description = msg.data.remove(0,4);

    _stateReportSlot[slot.channel] = slot;

    emit slotListChange();
}
