#include "valueProtocol.h"

ValueProtocol::ValueProtocol(busDevice *device):BusProtocol(device)
{
}

void ValueProtocol::pushData(BusMessage msg)
{
    if(msg.protocol != Protocol::ValueReportProtocol) return;

    if(msg.command == Command::ValueReport) _parseValue(msg);
    else if(msg.command == Command::SignalInformationReport) _parseSignalInformationReport(msg);
    else if(msg.command == Command::SlotInformationReport) _parseSlotInformationReport(msg);
}

void ValueProtocol::requestSignalInformation()
{
    BusMessage msg;

    msg.protocol = Protocol::ValueReportProtocol;
    msg.command = Command::SignalInformationRequest;

    sendMessage(msg);
}

void ValueProtocol::requestSlotInformation()
{
    BusMessage msg;

    msg.protocol = Protocol::ValueReportProtocol;
    msg.command = Command::SlotInformationRequest;

    sendMessage(msg);
}

void ValueProtocol::reset()
{
    _valueSignal.clear();
    _valueSlot.clear();

    emit signalListChange();
    emit slotListChange();
}

QString ValueProtocol::valueToString(UnitType type, ValueData data)
{
    switch(type){
        case Long: return QString::number(data.Long);
        case Number: return QString::number(data.Number);
    }
    return "Decode Error";
}

ValueProtocol::ValueData ValueProtocol::stringToValue(UnitType type, QString value)
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

ValueProtocol::UnitType ValueProtocol::uomToType(UnitOfMeasure uom)
{
    return UnitofMeasurements[uom].type;
}

QString ValueProtocol::uomName(UnitOfMeasure uom)
{
    return UnitofMeasurements[uom].name;
}

QString ValueProtocol::uomUnit(UnitOfMeasure uom)
{
    return UnitofMeasurements[uom].unit;
}

QString ValueProtocol::valueString(uint16_t channel)
{
    ValueSignal val = _valueSignal[channel];
    return valueToString(UnitofMeasurements[val.uom].type, val.value);
}

QList<ValueProtocol::ValueSignal *> ValueProtocol::valueSignls()
{
    QList<ValueProtocol::ValueSignal *> output;
    for(auto &item: _valueSignal){
        output.append(&item);
    }
    return output;
}

void ValueProtocol::_parseValue(BusMessage msg)
{
    uint16_t channel = getUint16(msg.data, 0);

    uint16_t uomIndex = _valueSignal[channel].uom;

    UnitType type = UnitofMeasurements[uomIndex].type;

    _valueSignal[channel].value = _decodeData(type, msg.data.mid(2));

    emit signalValueChnage(channel);
}

QList<ValueProtocol::ValueSlot *> ValueProtocol::valueSlots()
{
    QList<ValueProtocol::ValueSlot *> output;
    for(auto &item: _valueSlot){
        output.append(&item);
    }
    return output;
}

void ValueProtocol::_parseSignalInformationReport(BusMessage msg)
{
    ValueSignal signal;
    signal.channel = getUint16(msg.data,0);
    signal.interval = getUint16(msg.data,2);

    uint16_t uomIndex = (UnitOfMeasure)getUint16(msg.data,4);
    UnitType type = UnitofMeasurements[uomIndex].type;

    signal.readOnly = (uomIndex & 0x8000);
    signal.uom = (UnitOfMeasure)(uomIndex &0x7FFF);
    signal.minimum = _decodeData(type, msg.data.mid(6,4));
    signal.maximum = _decodeData(type, msg.data.mid(10,4));
    signal.description = msg.data.mid(14);

    _valueSignal[signal.channel] = signal;

    emit signalListChange();
}

void ValueProtocol::_parseSlotInformationReport(BusMessage msg)
{
    ValueSlot slot;
    slot.channel = getUint16(msg.data,0);
    slot.timeout = getUint16(msg.data,2);
    slot.description = msg.data.mid(4);

    _valueSlot[slot.channel] = slot;

    emit slotListChange();
}

uint8_t ValueProtocol::_typeToSize(UnitType type)
{
    switch(type){
        case Long: return 4;
        case Number: return 4;
    }
    return 0;
}

ValueProtocol::ValueData ValueProtocol::_decodeData(UnitType type, QByteArray data)
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

QList<Protocol> ValueProtocol::protocol(void)
{
    QList<Protocol> temp;
    temp.append(Protocol::ValueReportProtocol);
    return temp;
}

void ValueProtocol::sendValueCommand(uint16_t channel, ValueData value)
{
    BusMessage msg;

    msg.protocol = Protocol::ValueReportProtocol;
    msg.command = Command::ValueCommand;

    msg.data.append(packUint16(channel));
    msg.data.append(packUint32(value.Long));

    sendMessage(msg);
}

