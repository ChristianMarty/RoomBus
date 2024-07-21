#ifndef EVENT_PROTOCOL_H
#define EVENT_PROTOCOL_H

#include <QObject>
#include <QMap>
#include "busProtocol.h"
#include "protocol/protocol.h"


class EventProtocol : public BusProtocol
{
    Q_OBJECT
public:

    enum Command {
        Event,
        Reserved0,
        Reserved1,
        Reserved2,

        SignalInformationReport,
        SlotInformationReport,
        SignalInformationRequest,
        SlotInformationRequest
    };

    struct EventSignal {
        uint16_t channel;
        QString description;
        uint16_t interval;
    };

    struct EventSlot {
        uint16_t channel;
        QString description;
        uint16_t timeout;
    };

    EventProtocol(busDevice *device);

    void pushData(busMessage msg);
    QList<Protocol> protocol(void);

    void requestSignalInformation(void);
    void requestSlotInformation(void);

    void resetEventTimeout(QList<uint8_t>eventChannels);
    void resetEventTimeout(uint8_t eventChannel);

    void setActiveState(uint8_t eventChannel, bool active);

    void sendEvent(QList<uint8_t>eventChannels);
    void sendEvent(uint8_t eventChannel);

    void reset(void);

    QList<EventProtocol::EventSlot*> eventSlots();
    QList<EventProtocol::EventSignal*> eventSignls();

signals:
    void eventSignalReceived(QList<uint8_t>triggerSignal);

    void eventSignalListChange(void);
    void eventSlotListChange(void);

private:
    QMap<uint16_t, EventProtocol::EventSlot> _eventSlot;
    QMap<uint16_t, EventProtocol::EventSignal> _eventSignal;

    void _parseEvent(busMessage msg);
    void _parseSignalInformationReport(busMessage msg);
    void _parseSlotInformationReport(busMessage msg);
};

#endif // EVENT_PROTOCOL_H
