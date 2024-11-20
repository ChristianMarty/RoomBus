#ifndef TRIGGER_PROTOCOL_H
#define TRIGGER_PROTOCOL_H

#include <QObject>
#include <QMap>
#include "busProtocol.h"

class TriggerSystemProtocol : public BusProtocol
{
    Q_OBJECT
public:

    struct TriggerSignal {
        uint16_t channel;
        QString description;
    };

    struct TriggerSlot {
        uint16_t channel;
        QString description;
        TriggerSystemProtocol *trigger = nullptr;
    };

    TriggerSystemProtocol(RoomBusDevice *device);

    void pushData(RoomBus::Message msg);
    QList<RoomBus::Protocol> protocol(void);

    void requestSignalInformation(void);
    void requestSlotInformation(void);

    void sendTrigger(QList<uint16_t>triggerChannels);
    void sendTrigger(uint16_t triggerChannel);

    void emulateTrigger(uint16_t triggerChannel);

    void reset(void);

    QList<TriggerSystemProtocol::TriggerSlot*> triggerSlots();
    QList<TriggerSystemProtocol::TriggerSignal*> triggerSignls();

    QMap<uint16_t, TriggerSystemProtocol::TriggerSlot> triggerSlotMap() const;
    QMap<uint16_t, TriggerSystemProtocol::TriggerSignal> triggerSignalMap() const;

signals:
    void triggerSignalReceived(QList<uint8_t>triggerSignal);

    void triggerSignalListChange(void);
    void triggerSlotListChange(void);

private:
    QMap<uint16_t, TriggerSystemProtocol::TriggerSlot> _triggerSlot;
    QMap<uint16_t, TriggerSystemProtocol::TriggerSignal> _triggerSignal;

    void _parseTrigger(RoomBus::Message msg);
    void _parseSignalInformationReport(RoomBus::Message msg);
    void _parseSlotInformationReport(RoomBus::Message msg);
};

#endif // TRIGGER_PROTOCOL_H
