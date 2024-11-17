#ifndef STATEREPORTPROTOCOL_H
#define STATEREPORTPROTOCOL_H

#include <QObject>
#include <QMap>
#include "busProtocol.h"

class StateReportProtocol : public BusProtocol
{
    Q_OBJECT
public:
    enum Command{
        State,
        StateRequest,
        Reserved0,
        Reserved1,

        SignalInformationReport,
        SlotInformationReport,

        SignalInformationRequest,
        SlotInformationRequest
    };

    typedef enum {
        off = 0,
        on = 1,
        transitioning = 2,
        undefined = 3,
        unknown = 4
    } SignalState;

    struct StateReportSignal {
        uint16_t channel;
        QString description;
        uint16_t interval;
    };

    struct StateReportSlot {
        uint16_t channel;
        QString description;
        uint16_t timeout;
    };

    StateReportProtocol(busDevice *device);

    void pushData(BusMessage msg);
    QList<Protocol> protocol(void);

    void requestSignalInformation(void);
    void requestSlotInformation(void);

    void requestAllState(void);

    void reset(void);

    QList<StateReportProtocol::StateReportSignal*> stateReportSignls();
    QList<StateReportProtocol::StateReportSlot*> stateReportSlots();

    QMap<uint16_t, StateReportProtocol::StateReportSignal> stateReportSignalMap() const;
    QMap<uint16_t, StateReportProtocol::StateReportSlot> stateReportSlotMap() const;

signals:
    void signalStateChnage(uint16_t channel, SignalState newState);

    void signalListChange(void);
    void slotListChange(void);

private:
    QMap<uint16_t, StateReportProtocol::StateReportSignal> _stateReportSignal;
    QMap<uint16_t, StateReportProtocol::StateReportSlot> _stateReportSlot;

    QMap<uint16_t, StateReportProtocol::SignalState> _signalState;


    void _parseStateReport(BusMessage msg);
    void _parseSignalInformationReport(BusMessage msg);
    void _parseSlotInformationReport(BusMessage msg);
};

#endif // STATEREPORTPROTOCOL_H
