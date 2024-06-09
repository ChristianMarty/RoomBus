#ifndef TRIGGERPROTOCOL_H
#define TRIGGERPROTOCOL_H

#include <QObject>
#include <QMap>
#include "busProtocol.h"





class triggerProtocol : public busProtocol
{
    Q_OBJECT
public:

    struct triggerSlot
    {
        uint8_t channel;
        QString name;
        uint8_t sourceAddressFiler;
    };

    triggerProtocol(busDevice *device);

    void pushData(busMessage msg);
    QList<Protocol> protocol(void);

    void requestTriggerSignalNames(void);
    void requestTriggerSlotNames(void);

    void sendTrigger(QList<uint8_t>triggerChannels);
    void sendTrigger(uint8_t triggerChannel);

    void reset(void);

    QMap<uint8_t, QString> triggerSignalNames() const;
    QMap<uint8_t, triggerProtocol::triggerSlot> triggerSlotMap() const;

signals:

    void triggerSignalReceived(QList<uint8_t>triggerSignal);
    void triggerSignalListChange(void);
    void triggerSlotListChange(void);

private:
    QMap<uint8_t, QString> _triggerSignalNames;
    QMap<uint8_t, uint8_t> _triggerDestinationAddress;

    QMap<uint8_t, triggerProtocol::triggerSlot> _triggerSlot;

    void parseTriggerSignal(busMessage msg);
    void parseTriggerSignalNameReport(busMessage msg);
    void parseTriggerSlotNameReport(busMessage msg);
};

#endif // TRIGGERPROTOCOL_H
