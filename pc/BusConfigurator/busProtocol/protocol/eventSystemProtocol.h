#ifndef EVENT_PROTOCOL_H
#define EVENT_PROTOCOL_H

#include <QObject>
#include <QMap>

#include "protocolBase.h"

class EventSystemProtocol : public ProtocolBase
{
    Q_OBJECT
public:

    enum class Command:MiniBus::Command {
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

    EventSystemProtocol(RoomBusDevice *device);

    void handleMessage(const MiniBus::Message &message) override;

    void requestSignalInformation(void);
    void requestSlotInformation(void);
    void clearInformation(void);

    void sendEvent(QList<uint16_t>eventChannels);
    void sendEvent(uint16_t eventChannel);

    QList<EventSystemProtocol::EventSlot*> eventSlots();
    QList<EventSystemProtocol::EventSignal*> eventSignls();

    static QString commandName(MiniBus::Command command);
    static QString dataDecoder(MiniBus::Command command, const QByteArray &data);

signals:
    void eventSignalReceived(QList<uint16_t>eventSignal);

    void eventSignalListChange(void);
    void eventSlotListChange(void);

private:
    QMap<uint16_t, EventSystemProtocol::EventSlot> _eventSlot;
    QMap<uint16_t, EventSystemProtocol::EventSignal> _eventSignal;

    void _parseEvent(MiniBus::Message msg);
    void _parseSignalInformationReport(MiniBus::Message msg);
    void _parseSlotInformationReport(MiniBus::Message msg);
};

#endif // EVENT_PROTOCOL_H
