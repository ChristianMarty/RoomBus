#ifndef EEPROM_H
#define EEPROM_H

#include <QObject>
#include "uiComponents/uiDatatypes.h"

class DeviceManagementProtocol;
class Eeprom
{
public:
    explicit Eeprom(DeviceManagementProtocol *parent);

    void clear(void);
    void readAll(void);

    void requestEepromRead(uint16_t offset, uint8_t size);

    uint16_t eepromSize(void) const;

    UiDatatypes::MemoryByte at(uint16_t index) const;
    QList<UiDatatypes::MemoryByte> data(void) const;

    friend DeviceManagementProtocol;
private:
    DeviceManagementProtocol *_parent;

    void _decodeEepromReadReport(const QByteArray &data);

    uint16_t _readPositon = 0;
    bool _readAll = false;
    static const uint8_t _messageSize = 60;

    static const uint16_t _eepromSize = 512;
    UiDatatypes::MemoryByte _eeprom[_eepromSize];
};

#endif // EEPROM_H
