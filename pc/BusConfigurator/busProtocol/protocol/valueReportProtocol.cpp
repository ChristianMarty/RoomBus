#include "valueReportProtocol.h"

valueReportProtocol::valueReportProtocol(busDevice *device):busProtocol(device)
{
}

void valueReportProtocol::pushData(busMessage msg)
{
    if(msg.protocol != Protocol::ValueReportProtocol) return;
    if(msg.command == valueReport)
    {
        uint16_t channel =  (uint16_t)msg.data[0];

        uint32_t val_raw = ((uint32_t)msg.data[1])<<24 & 0xFF000000;
        val_raw |= ((uint32_t)msg.data[2])<<16 & 0x00FF0000;
        val_raw |= ((uint32_t)msg.data[3])<<8 & 0x00000FF00;
        val_raw |= ((uint32_t)msg.data[4]) & 0x0000000FF;

        float vlaue = *((float*)&val_raw);
        emit newValue(channel,vlaue);
    }
    else if(msg.command == metaDataReport)
    {
        valueItem_t item;

        item.channel = (uint8_t)msg.data[0];
        item.readOnly = (bool)(msg.data[1]&0x80);
        item.uom = (unitOfMeasure_t)(msg.data[1]&0x7F);
        item.min = getFloat32(msg.data,2);
        item.max = getFloat32(msg.data,6);

        item.description = msg.data.remove(0,10);

        values[item.channel] = item;
        emit metaDataChange(item.channel,item);
    }
}

void valueReportProtocol::setValue(uint16_t channel, float value)
{
    busMessage msg;

    msg.protocol = Protocol::ValueReportProtocol;
    msg.command = valueCommand;
    msg.data.append(static_cast<char>(channel));
    msg.data.append(static_cast<char>(valueCommand_t::setToValue));
    msg.data.append(packFloat32(value));

    sendMessage(msg);
}

void valueReportProtocol::requestMetaData()
{
    busMessage msg;

    msg.protocol = Protocol::ValueReportProtocol;
    msg.command = metaDataRequest;

    sendMessage(msg);
}

QList<Protocol> valueReportProtocol::protocol(void)
{
    QList<Protocol> temp;
    temp.append(Protocol::ValueReportProtocol);
    return temp;
}

