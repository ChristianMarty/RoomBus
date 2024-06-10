#ifndef EVENTPROTOCOL_H
#define EVENTPROTOCOL_H

#include <QObject>
#include <QMap>
#include "busProtocol.h"
#include "protocol/protocol.h"


class eventSlot
{
public:
    QString name;
    uint8_t channel;
    uint16_t timeout;
    uint8_t sourceAddress;
    bool active;
};

class eventProtocol : public BusProtocol
{
    Q_OBJECT
public:

    eventProtocol(busDevice *device);

    void pushData(busMessage msg);
    QList<Protocol> protocol(void);

    void requestEventSignalNames(void);
    void requestEventSlotNames(void);

    void resetEventTimeout(QList<uint8_t>eventChannels);
    void resetEventTimeout(uint8_t eventChannel);

    void setActiveState(uint8_t eventChannel, bool active);

    void sendEvent(QList<uint8_t>eventChannels);
    void sendEvent(uint8_t eventChannel);

    QList<eventSlot*> eventSlots();

signals:
    void eventSignalReceived(QList<uint8_t>triggerSignal);
    void eventSignalListChange(void);
    void eventSlotListChange(void);

private:
    QMap<uint8_t, eventSlot> _eventSlots;

    void parseEventSignal(busMessage msg);
    void parseEventSignalNameReport(busMessage msg);
    void parseEventSlotNameReport(busMessage msg);
};

#endif // EVENTPROTOCOL_H
