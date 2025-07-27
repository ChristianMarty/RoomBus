#ifndef VALUE_REPORT_PROTOCOL_H
#define VALUE_REPORT_PROTOCOL_H

#include <QObject>
#include <QMap>
#include <QDateTime>

#include "protocolBase.h"


class ValueSystemProtocol : public ProtocolBase
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

    static const constexpr UnitOfMeasurement UnitOfMeasurements[] = {
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

    enum class ValueCommand {
        setMinimum = 0x00,
        setMaximum,
        invert,

        setLongValueReject = 0x10,
        setLongValueClamp,

        addLongReject = 0x20,
        addLongClamp,

        subtractLongReject = 0x30,
        subtractLongClamp,


        setNumberValueReject = 0x40,
        setNumberValueClamp,

        addNumberReject = 0x50,
        addNumberClamp
    };

    ValueSystemProtocol(RoomBusDevice *device);

    void handleMessage(RoomBus::Message msg);

    void sendValueCommand(uint16_t channel, ValueData value);

    void requestMetaData();

    void requestSignalInformation(void);
    void requestSlotInformation(void);
    void clearInformation(void);

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
