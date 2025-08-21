#include "valueSystemProtocol.h"
#include "busDevice.h"

ValueSystemProtocol::ValueSystemProtocol(RoomBusDevice *device)
    : ProtocolBase(device)
{
    _device->addProtocol(this);
}

void ValueSystemProtocol::handleMessage(MiniBus::Message message)
{
    if(message.protocol != (MiniBus::Protocol)Protocol::ValueSystemProtocol){
        return;
    }

    switch((Command)message.command){
        case Command::ValueReport: _parseValue(message); break;
        case Command::SignalInformationReport: _parseSignalInformationReport(message); break;
        case Command::SlotInformationReport: _parseSlotInformationReport(message); break;

        case Command::ValueRequest:
        case Command::ValueCommand:
        case Command::Reserved0:
        case Command::SignalInformationRequest:
        case Command::SlotInformationRequest:
            break;
    }
}

void ValueSystemProtocol::requestSignalInformation()
{
    MiniBus::Message msg;
    msg.protocol = (MiniBus::Protocol)Protocol::ValueSystemProtocol;
    msg.command = (MiniBus::Command)Command::SignalInformationRequest;

    sendMessage(msg);
}

void ValueSystemProtocol::requestSlotInformation()
{
    MiniBus::Message msg;
    msg.protocol = (MiniBus::Protocol)Protocol::ValueSystemProtocol;
    msg.command = (MiniBus::Command)Command::SlotInformationRequest;

    sendMessage(msg);
}

void ValueSystemProtocol::clearInformation()
{
    _valueSignal.clear();
    _valueSlot.clear();

    emit signalListChange();
    emit slotListChange();
}

QString ValueSystemProtocol::valueToString(UnitType type, ValueData data)
{
    switch(type){
        case Long: return QString::number(data.Long);
        case Number: return QString::number(data.Number);
    }
    return "Decode Error";
}

ValueSystemProtocol::ValueData ValueSystemProtocol::stringToValue(UnitType type, QString value)
{
    ValueData output;
    output.Long = 0;

    switch(type){
        case Long: {
            output.Long = (uint32_t)value.toLong();
            break;
        }
        case Number: {
            output.Number = value.toFloat();
            break;
        }
    }
    return output;
}

ValueSystemProtocol::UnitType ValueSystemProtocol::uomToType(UnitOfMeasure uom)
{
    return UnitOfMeasurements[uom].type;
}

QString ValueSystemProtocol::uomName(UnitOfMeasure uom)
{
    return UnitOfMeasurements[uom].name;
}

QString ValueSystemProtocol::uomUnit(UnitOfMeasure uom)
{
    return UnitOfMeasurements[uom].unit;
}

QString ValueSystemProtocol::valueString(uint16_t channel)
{
    ValueSignal val = _valueSignal[channel];
    return valueToString(UnitOfMeasurements[val.uom].type, val.value);
}

QList<ValueSystemProtocol::ValueSignal *> ValueSystemProtocol::valueSignls()
{
    QList<ValueSystemProtocol::ValueSignal *> output;
    for(auto &item: _valueSignal){
        output.append(&item);
    }
    return output;
}

QString ValueSystemProtocol::commandName(MiniBus::Command command)
{
    switch((Command)command){
        case Command::ValueReport: return "Value Report"; break;
        case Command::ValueRequest: return "Value Request"; break;
        case Command::ValueCommand: return "Value Command"; break;
        case Command::Reserved0 : return "Reserved";

        case Command::SignalInformationReport : return "Signal Information Report";
        case Command::SlotInformationReport : return "Slot Information Report";
        case Command::SignalInformationRequest : return "Signal Information Request";
        case Command::SlotInformationRequest : return "Slot Information Request";
    }

    return "Unknown Command";
}

QString ValueSystemProtocol::dataDecoder(MiniBus::Command command, const QByteArray &data)
{
    Q_UNUSED(command);
    Q_UNUSED(data);
    return "Not implemented";
}

void ValueSystemProtocol::_parseValue(MiniBus::Message msg)
{
    uint16_t channel = MiniBus::unpackUint16(msg.data, 0);

    uint16_t uomIndex = _valueSignal[channel].uom;

    UnitType type = UnitOfMeasurements[uomIndex].type;

    _valueSignal[channel].value = _decodeData(type, msg.data.mid(2));

    emit signalValueChnage(channel);
}

QList<ValueSystemProtocol::ValueSlot *> ValueSystemProtocol::valueSlots()
{
    QList<ValueSystemProtocol::ValueSlot *> output;
    for(ValueSystemProtocol::ValueSlot &item: _valueSlot){
        output.append(&item);
    }
    return output;
}

void ValueSystemProtocol::_parseSignalInformationReport(MiniBus::Message msg)
{
    ValueSignal signal;
    signal.channel = MiniBus::unpackUint16(msg.data,0);
    signal.interval = MiniBus::unpackUint16(msg.data,2);

    uint16_t uomIndex = (UnitOfMeasure)MiniBus::unpackUint16(msg.data,4);
    UnitType type = UnitOfMeasurements[uomIndex].type;

    signal.readOnly = (uomIndex & 0x8000);
    signal.uom = (UnitOfMeasure)(uomIndex &0x7FFF);
    signal.minimum = _decodeData(type, msg.data.mid(6,4));
    signal.maximum = _decodeData(type, msg.data.mid(10,4));
    signal.description = msg.data.mid(14);

    _valueSignal[signal.channel] = signal;

    emit signalListChange();
}

void ValueSystemProtocol::_parseSlotInformationReport(MiniBus::Message msg)
{
    ValueSlot slot;
    slot.channel = MiniBus::unpackUint16(msg.data,0);
    slot.timeout = MiniBus::unpackUint16(msg.data,2);
    slot.description = msg.data.mid(4);

    _valueSlot[slot.channel] = slot;

    emit slotListChange();
}

uint8_t ValueSystemProtocol::_typeToSize(UnitType type)
{
    switch(type){
        case Long: return 4;
        case Number: return 4;
    }
    return 0;
}

ValueSystemProtocol::ValueData ValueSystemProtocol::_decodeData(UnitType type, QByteArray data)
{
    ValueData value;
    switch(type){
        case Long: {
            uint32_t rawValue  = (((uint32_t)data.at(0))<<24) & 0xFF000000;
            rawValue |= (((uint32_t)data.at(1))<<16) & 0x00FF0000;
            rawValue |= (((uint32_t)data.at(2))<<8) & 0x0000FF00;
            rawValue |= ((uint32_t)data.at(3))& 0x000000FF;

            value.Long = rawValue;
            break;
        }
        case Number: {
            uint32_t rawValue  = (((uint32_t)data.at(0))<<24) & 0xFF000000;
            rawValue |= (((uint32_t)data.at(1))<<16) & 0x00FF0000;
            rawValue |= (((uint32_t)data.at(2))<<8) & 0x0000FF00;
            rawValue |= ((uint32_t)data.at(3))& 0x000000FF;

            value.Number = *((float*)&rawValue);
            break;
        }
    }
    return value;
}

void ValueSystemProtocol::sendValueCommand(uint16_t channel, ValueData value)
{
    MiniBus::Message msg;

    msg.protocol = (MiniBus::Protocol)Protocol::ValueSystemProtocol;
    msg.command = (MiniBus::Command)Command::ValueCommand;

    msg.data.append(MiniBus::packUint16(channel));
    msg.data.append((uint8_t)ValueCommand::setLongValueClamp);
    msg.data.append(MiniBus::packUint32(value.Long));

    sendMessage(msg);
}

