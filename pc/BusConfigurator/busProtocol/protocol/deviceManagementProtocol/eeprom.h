#ifndef EEPROM_H
#define EEPROM_H

#include <QObject>

class DeviceManagementProtocol;
class Eeprom
{
public:
    explicit Eeprom(DeviceManagementProtocol *parent);

    struct EepromByte {
        bool read = false;
        uint8_t byte = 0xFF;
    };

    void clear(void);
    void readAll(void);

    void requestEepromRead(uint16_t offset, uint8_t size);

    uint16_t eepromSize(void) const;

    EepromByte at(uint16_t index) const;

    friend DeviceManagementProtocol;
private:
    DeviceManagementProtocol *_parent;

    void _decodeEepromReadReport(const QByteArray &data);

    uint16_t _readPositon = 0;
    bool _readAll = false;
    static const uint8_t _messageSize = 60;

    static const uint16_t _eepromSize = 512;
    EepromByte _eeprom[_eepromSize];
};

#endif // EEPROM_H
