#ifndef TRIGGER_PROTOCOL_H
#define TRIGGER_PROTOCOL_H

#include <QObject>
#include <QMap>
#include "busProtocol.h"

class TriggerProtocol : public BusProtocol
{
    Q_OBJECT
public:

    enum Command {
        Trigger,
        Reserved0,
        Reserved1,
        Reserved2,

        SignalInformationReport,
        SlotInformationReport,

        SignalInformationRequest,
        SlotInformationRequest
    };

    struct TriggerSignal {
        uint16_t channel;
        QString description;
    };

    struct TriggerSlot {
        uint16_t channel;
        QString description;
        TriggerProtocol *trigger = nullptr;
    };

    TriggerProtocol(busDevice *device);

    void pushData(busMessage msg);
    QList<Protocol> protocol(void);

    void requestSignalInformation(void);
    void requestSlotInformation(void);

    void sendTrigger(QList<uint16_t>triggerChannels);
    void sendTrigger(uint16_t triggerChannel);

    void emulateTrigger(uint16_t triggerChannel);

    void reset(void);

    QList<TriggerProtocol::TriggerSlot*> triggerSlots();
    QList<TriggerProtocol::TriggerSignal*> triggerSignls();

    QMap<uint16_t, TriggerProtocol::TriggerSlot> triggerSlotMap() const;
    QMap<uint16_t, TriggerProtocol::TriggerSignal> triggerSignalMap() const;

signals:
    void triggerSignalReceived(QList<uint8_t>triggerSignal);

    void triggerSignalListChange(void);
    void triggerSlotListChange(void);

private:
    QMap<uint16_t, TriggerProtocol::TriggerSlot> _triggerSlot;
    QMap<uint16_t, TriggerProtocol::TriggerSignal> _triggerSignal;

    void _parseTrigger(busMessage msg);
    void _parseSignalInformationReport(busMessage msg);
    void _parseSlotInformationReport(busMessage msg);
};

#endif // TRIGGER_PROTOCOL_H
