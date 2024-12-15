#include "valueProtocol.h"

ValueSystemProtocol::ValueSystemProtocol(RoomBusDevice *device):BusProtocol(device)
{
}

void ValueSystemProtocol::pushData(RoomBus::Message msg)
{
    if(msg.protocol != RoomBus::Protocol::ValueSystemProtocol) return;

    switch((RoomBus::ValueSystemCommand)msg.command){
        case RoomBus::ValueSystemCommand::ValueReport: _parseValue(msg); break;
        case RoomBus::ValueSystemCommand::SignalInformationReport: _parseSignalInformationReport(msg); break;
        case RoomBus::ValueSystemCommand::SlotInformationReport: _parseSlotInformationReport(msg); break;
    }
}

void ValueSystemProtocol::requestSignalInformation()
{
    RoomBus::Message msg;

    msg.protocol = RoomBus::Protocol::ValueSystemProtocol;
    msg.command = (uint8_t)RoomBus::ValueSystemCommand::SignalInformationRequest;

    sendMessage(msg);
}

void ValueSystemProtocol::requestSlotInformation()
{
    RoomBus::Message msg;

    msg.protocol = RoomBus::Protocol::ValueSystemProtocol;
    msg.command = (uint8_t)RoomBus::ValueSystemCommand::SlotInformationRequest;

    sendMessage(msg);
}

void ValueSystemProtocol::reset()
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
    return UnitofMeasurements[uom].type;
}

QString ValueSystemProtocol::uomName(UnitOfMeasure uom)
{
    return UnitofMeasurements[uom].name;
}

QString ValueSystemProtocol::uomUnit(UnitOfMeasure uom)
{
    return UnitofMeasurements[uom].unit;
}

QString ValueSystemProtocol::valueString(uint16_t channel)
{
    ValueSignal val = _valueSignal[channel];
    return valueToString(UnitofMeasurements[val.uom].type, val.value);
}

QList<ValueSystemProtocol::ValueSignal *> ValueSystemProtocol::valueSignls()
{
    QList<ValueSystemProtocol::ValueSignal *> output;
    for(auto &item: _valueSignal){
        output.append(&item);
    }
    return output;
}

void ValueSystemProtocol::_parseValue(RoomBus::Message msg)
{
    uint16_t channel = RoomBus::unpackUint16(msg.data, 0);

    uint16_t uomIndex = _valueSignal[channel].uom;

    UnitType type = UnitofMeasurements[uomIndex].type;

    _valueSignal[channel].value = _decodeData(type, msg.data.mid(2));

    emit signalValueChnage(channel);
}

QList<ValueSystemProtocol::ValueSlot *> ValueSystemProtocol::valueSlots()
{
    QList<ValueSystemProtocol::ValueSlot *> output;
    for(auto &item: _valueSlot){
        output.append(&item);
    }
    return output;
}

void ValueSystemProtocol::_parseSignalInformationReport(RoomBus::Message msg)
{
    ValueSignal signal;
    signal.channel = RoomBus::unpackUint16(msg.data,0);
    signal.interval = RoomBus::unpackUint16(msg.data,2);

    uint16_t uomIndex = (UnitOfMeasure)RoomBus::unpackUint16(msg.data,4);
    UnitType type = UnitofMeasurements[uomIndex].type;

    signal.readOnly = (uomIndex & 0x8000);
    signal.uom = (UnitOfMeasure)(uomIndex &0x7FFF);
    signal.minimum = _decodeData(type, msg.data.mid(6,4));
    signal.maximum = _decodeData(type, msg.data.mid(10,4));
    signal.description = msg.data.mid(14);

    _valueSignal[signal.channel] = signal;

    emit signalListChange();
}

void ValueSystemProtocol::_parseSlotInformationReport(RoomBus::Message msg)
{
    ValueSlot slot;
    slot.channel = RoomBus::unpackUint16(msg.data,0);
    slot.timeout = RoomBus::unpackUint16(msg.data,2);
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

QList<RoomBus::Protocol> ValueSystemProtocol::protocol(void)
{
    QList<RoomBus::Protocol> temp;
    temp.append(RoomBus::Protocol::ValueSystemProtocol);
    return temp;
}

void ValueSystemProtocol::sendValueCommand(uint16_t channel, ValueData value)
{
    RoomBus::Message msg;

    msg.protocol = RoomBus::Protocol::ValueSystemProtocol;
    msg.command = (uint8_t)RoomBus::ValueSystemCommand::ValueCommand;

    msg.data.append(RoomBus::packUint16(channel));
    msg.data.append((uint8_t)ValueCommand::vsp_vCmd_setLongValueClamp);
    msg.data.append(RoomBus::packUint32(value.Long));

    sendMessage(msg);
}

