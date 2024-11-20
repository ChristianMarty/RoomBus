#ifndef EVENT_PROTOCOL_H
#define EVENT_PROTOCOL_H

#include <QObject>
#include <QMap>
#include "busProtocol.h"

#include "roomBusMessage.h"


class EventSystemProtocol : public BusProtocol
{
    Q_OBJECT
public:

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

    EventSystemProtocol(RoomBusDevice *device);

    void pushData(RoomBus::Message msg);
    QList<RoomBus::Protocol> protocol(void);

    void requestSignalInformation(void);
    void requestSlotInformation(void);

    void resetEventTimeout(QList<uint8_t>eventChannels);
    void resetEventTimeout(uint8_t eventChannel);

    void setActiveState(uint8_t eventChannel, bool active);

    void sendEvent(QList<uint8_t>eventChannels);
    void sendEvent(uint8_t eventChannel);

    void reset(void);

    QList<EventSystemProtocol::EventSlot*> eventSlots();
    QList<EventSystemProtocol::EventSignal*> eventSignls();

signals:
    void eventSignalReceived(QList<uint8_t>triggerSignal);

    void eventSignalListChange(void);
    void eventSlotListChange(void);

private:
    QMap<uint16_t, EventSystemProtocol::EventSlot> _eventSlot;
    QMap<uint16_t, EventSystemProtocol::EventSignal> _eventSignal;

    void _parseEvent(RoomBus::Message msg);
    void _parseSignalInformationReport(RoomBus::Message msg);
    void _parseSlotInformationReport(RoomBus::Message msg);
};

#endif // EVENT_PROTOCOL_H
