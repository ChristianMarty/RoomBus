#include "busProtocol.h"
#include "busDevice.h"

BusProtocol::BusProtocol(busDevice *device)
{
    _device = device;
    _device->addProtocol(this);
}

BusProtocol::~BusProtocol(void)
{
    _device->removeProtocol(this);
}

void BusProtocol::sendMessage(busMessage msg)
{
    msg.dstAddress = _device->deviceAddress();
    _device->dataReady(msg);
}


uint8_t BusProtocol::getUint8(QByteArray data, uint32_t index)
{
    return static_cast<uint8_t>(data.at(index));
}

uint16_t BusProtocol::getUint16(QByteArray data, uint32_t index)
{
    uint16_t temp = 0;

    temp |= (static_cast<uint16_t>(data.at(index))<<8)&0xFF00;
    temp |= (static_cast<uint16_t>(data.at(index+1))<<0)&0x00FF;

    return temp;
}

uint32_t BusProtocol::getUint32(QByteArray data, uint32_t index)
{
    uint32_t temp = 0;

    temp =  (static_cast<uint32_t>(data.at(index))<<24)&0xFF000000;
    temp |= (static_cast<uint32_t>(data.at(index+1))<<16)&0x00FF0000;
    temp |= (static_cast<uint32_t>(data.at(index+2))<<8)&0x0000FF00;
    temp |= (static_cast<uint32_t>(data.at(index+3))<<0)&0x000000FF;

    return temp;
}

float BusProtocol::getFloat32(QByteArray data, uint32_t index)
{
    uint32_t temp = getUint32(data,index);

    float *out;
    out = (float*)&temp;

    return *out;
}

QByteArray BusProtocol::packUint32(uint32_t value)
{
    QByteArray temp;
    temp.append(static_cast<char>(static_cast<uint8_t>(value>>24)));
    temp.append(static_cast<char>(static_cast<uint8_t>(value>>16)));
    temp.append(static_cast<char>(static_cast<uint8_t>(value>>8)));
    temp.append(static_cast<char>(static_cast<uint8_t>(value)));

    return temp;
}

QByteArray BusProtocol::packUint16(uint16_t value)
{
    QByteArray temp;
    temp.append(static_cast<char>(static_cast<uint8_t>(value>>8)));
    temp.append(static_cast<char>(static_cast<uint8_t>(value)));

    return temp;
}

QByteArray BusProtocol::packFloat32(float value)
{
    uint32_t *temp = (uint32_t*)&value;

    return  packUint32(*temp);
}
