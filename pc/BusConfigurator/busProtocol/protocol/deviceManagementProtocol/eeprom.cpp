#include "eeprom.h"
#include "deviceManagementProtocol.h"

Eeprom::Eeprom(DeviceManagementProtocol *parent)
    :_parent{parent}
{
    clear();
}

void Eeprom::readAll()
{
    _readAll = true;

    _readPositon = 0;
    requestEepromRead(_readPositon, _messageSize);
    _readPositon += _messageSize;
}

void Eeprom::requestEepromRead(uint16_t offset, uint8_t size)
{
    RoomBus::Message msg;
    msg.data.append((uint8_t)DeviceManagementProtocol::DeviceManagementSubCommand::EepromReadRequest);
    msg.data.append(RoomBus::packUint16(offset));
    msg.data.append(RoomBus::packUint8(size));

    _parent->sendMessage(msg);
}

uint16_t Eeprom::eepromSize() const
{
    return _eepromSize;
}

Eeprom::EepromByte Eeprom::at(uint16_t index) const
{
    if(index >= _eepromSize) return EepromByte();

    return _eeprom[index];
}

void Eeprom::_decodeEepromReadReport(const QByteArray &data)
{
    if(data.length() < 2){
        _readAll = false;
        return;
    }

    uint16_t offste = RoomBus::unpackUint16(data,0);
    uint16_t length = data.length()-2;

    if(offste+length > _eepromSize){
        _readAll = false;
        return;
    }

    for(uint16_t i = 0; i < length; i++){
        _eeprom[offste+i] = EepromByte({.read = true, .byte = (uint8_t)data.at(i+2) });
    }

    emit _parent->eepromDataChanged();

    if(_readAll){

        if(_readPositon >= _eepromSize){
            _readAll = false;
            return;
        }

        uint8_t readSize = _messageSize;
        if(_readPositon+_messageSize > _eepromSize){
            readSize = _eepromSize - _readPositon;
        }
        requestEepromRead(_readPositon, readSize);
        _readPositon += readSize;
    }
}

void Eeprom::clear()
{
    for(uint16_t i = 0; i < _eepromSize; i++){
        _eeprom[i] = EepromByte();
    }

    emit _parent->eepromDataChanged();
}
