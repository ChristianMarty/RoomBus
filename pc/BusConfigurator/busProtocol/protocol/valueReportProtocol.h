#ifndef VALUE_REPORT_PROTOCOL_H
#define VALUE_REPORT_PROTOCOL_H

#include <QObject>
#include <QMap>
#include <QDateTime>

#include "busProtocol.h"


class valueReportProtocol : public BusProtocol
{
    Q_OBJECT
public:

    enum unitOfMeasure_t{
        UOM_RAW_FLOAT,
        UOM_RAW_UINT32,
        UOM_ABSOLUTE_TIME,
        UOM_RELATIVE_TIME,
        UOM_TEMPERATURE
    };

    enum valueCommand_t{
        setToMinimum = 0x00,
        setToMaximum = 0x01,
        invert = 0x02,
        setToValue = 0x03
    };

    enum commands_t{
        valueReport  = 0,
        valueCommand  = 1,
        metaDataReport = 2,
        valueRequest = 3,
        metaDataRequest= 4
    };

    valueReportProtocol(busDevice *device);

    void pushData(busMessage msg);
    QList<Protocol> protocol(void);

    struct valueItem_t{
        uint8_t channel;
        QString description;
        bool readOnly;
        float min;
        float max;
        float value;
        unitOfMeasure_t uom;
    };

    QMap<uint16_t,valueItem_t> values;

    void setValue(uint16_t channel, float value);

    void requestMetaData();

signals:
    void newValue(uint16_t channel, float value);

    void metaDataChange(uint16_t channel, valueItem_t item);

private:

};

#endif // VALUE_REPORT_PROTOCOL_H
