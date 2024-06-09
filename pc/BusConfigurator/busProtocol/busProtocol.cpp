#include "busProtocol.h"
#include "busDevice.h"

busProtocol::busProtocol(busDevice *device)
{
    _device = device;
    _device->addProtocol(this);
}

busProtocol::~busProtocol(void)
{
    _device->removeProtocol(this);
}

void busProtocol::sendMessage(busMessage msg)
{
    msg.dstAddress = _device->deviceAddress();
    _device->dataReady(msg);
}

uint8_t busProtocol::getUint8(QByteArray data, uint32_t index)
{
    return static_cast<uint8_t>(data.at(index));
}

uint16_t busProtocol::getUint16(QByteArray data, uint32_t index)
{
    uint16_t temp = 0;

    temp |= (static_cast<uint16_t>(data.at(index))<<8)&0xFF00;
    temp |= (static_cast<uint16_t>(data.at(index+1))<<0)&0x00FF;

    return temp;
}

uint32_t busProtocol::getUint32(QByteArray data, uint32_t index)
{
    uint32_t temp = 0;

    temp =  (static_cast<uint32_t>(data.at(index))<<24)&0xFF000000;
    temp |= (static_cast<uint32_t>(data.at(index+1))<<16)&0x00FF0000;
    temp |= (static_cast<uint32_t>(data.at(index+2))<<8)&0x0000FF00;
    temp |= (static_cast<uint32_t>(data.at(index+3))<<0)&0x000000FF;

    return temp;
}

float busProtocol::getFloat32(QByteArray data, uint32_t index)
{
    uint32_t temp = getUint32(data,index);

    float *out;
    out = (float*)&temp;

    return *out;
}

QByteArray busProtocol::packUint32(uint32_t value)
{
    QByteArray temp;
    temp.append(static_cast<char>(static_cast<uint8_t>(value>>24)));
    temp.append(static_cast<char>(static_cast<uint8_t>(value>>16)));
    temp.append(static_cast<char>(static_cast<uint8_t>(value>>8)));
    temp.append(static_cast<char>(static_cast<uint8_t>(value)));

    return temp;
}

QByteArray busProtocol::packFloat32(float value)
{
    uint32_t *temp = (uint32_t*)&value;

    return  packUint32(*temp);
}
