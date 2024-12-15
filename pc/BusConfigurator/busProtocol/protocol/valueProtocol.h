#ifndef VALUE_REPORT_PROTOCOL_H
#define VALUE_REPORT_PROTOCOL_H

#include <QObject>
#include <QMap>
#include <QDateTime>

#include "busProtocol.h"


class ValueSystemProtocol : public BusProtocol
{
    Q_OBJECT
public:

    enum UnitType {
        Long,
        Number
    };

    union ValueData{
        uint32_t Long;
        float Number;
    };

    struct UnitOfMeasurement {
        const char* name;
        const char* unit;
        UnitType type;
    };

    static const constexpr UnitOfMeasurement UnitofMeasurements[] = {
        {"Long","", UnitType::Long}, //0x00
        {"Number","", UnitType::Number},
        {},{},{},{},{},
        {},{},{},{},{},{},{},{},{},
        {"Absolute Time", "", UnitType::Long}, // 0x10
        {"Time of Day", "", UnitType::Long},
    };

    enum UnitOfMeasure{
        uom_long,
        uom_number,

        uom_absoluteTime = 0x10,
        uom_relativeTime,
        uom_temperature
    };

    struct ValueSignal {
        uint16_t channel;
        QString description;
        uint16_t interval;

        ValueData minimum;
        ValueData maximum;

        UnitOfMeasure uom;
        bool readOnly;

        ValueData value;
    };

    struct ValueSlot {
        uint16_t channel;
        QString description;
        uint16_t timeout;
    };

    enum class ValueCommand{
        vsp_vCmd_setMinimum = 0x00,
        vsp_vCmd_setMaximum,
        vsp_vCmd_invert,

        vsp_vCmd_setLongValueReject = 0x10,
        vsp_vCmd_setLongValueClamp,

        vsp_vCmd_addLongReject = 0x20,
        vsp_vCmd_addLongClamp,

        vsp_vCmd_subtractLongReject = 0x30,
        vsp_vCmd_subtractLongClamp,


        vsp_vCmd_setNumberValueReject = 0x40,
        vsp_vCmd_setNumberValueClamp,

        vsp_vCmd_addNumberReject = 0x50,
        vsp_vCmd_addNumberClamp

    };

    ValueSystemProtocol(RoomBusDevice *device);

    void pushData(RoomBus::Message msg);
    QList<RoomBus::Protocol> protocol(void);

    void sendValueCommand(uint16_t channel, ValueData value);

    void requestMetaData();

    void requestSignalInformation(void);
    void requestSlotInformation(void);

    void reset(void);

    static QString valueToString(UnitType type, ValueData data);
    static ValueData stringToValue(UnitType type, QString value);
    static UnitType uomToType(UnitOfMeasure uom);
    static QString uomName(UnitOfMeasure uom);
    static QString uomUnit(UnitOfMeasure uom);

    QString valueString(uint16_t channel);

    QList<ValueSystemProtocol::ValueSlot*> valueSlots();
    QList<ValueSystemProtocol::ValueSignal*> valueSignls();

signals:
    void signalValueChnage(uint16_t channel);

    void signalListChange(void);
    void slotListChange(void);

private:
    QMap<uint16_t, ValueSystemProtocol::ValueSignal> _valueSignal;
    QMap<uint16_t, ValueSystemProtocol::ValueSlot> _valueSlot;

    void _parseValue(RoomBus::Message msg);
    void _parseSignalInformationReport(RoomBus::Message msg);
    void _parseSlotInformationReport(RoomBus::Message msg);

    uint8_t _typeToSize(UnitType type);
    ValueData _decodeData(UnitType type, QByteArray data);
};

#endif // VALUE_REPORT_PROTOCOL_H
