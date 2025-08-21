#ifndef STATESYSTEMPROTOCOL_H
#define STATESYSTEMPROTOCOL_H

#include <QObject>
#include <QMap>

#include "protocolBase.h"

class StateSystemProtocol : public ProtocolBase
{
    Q_OBJECT
public:

    enum class Command:MiniBus::Command {
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

    StateSystemProtocol(RoomBusDevice *device);

    void handleMessage(MiniBus::Message msg);

    void requestSignalInformation(void);
    void requestSlotInformation(void);

    void requestAllState(void);

    void reset(void);

    QList<StateSystemProtocol::StateReportSignal*> stateReportSignls();
    QList<StateSystemProtocol::StateReportSlot*> stateReportSlots();

    QMap<uint16_t, StateSystemProtocol::StateReportSignal> stateReportSignalMap() const;
    QMap<uint16_t, StateSystemProtocol::StateReportSlot> stateReportSlotMap() const;

    static QString commandName(MiniBus::Command command);
    static QString dataDecoder(MiniBus::Command command, const QByteArray &data);

signals:
    void signalStateChnage(uint16_t channel, SignalState newState);

    void signalListChange(void);
    void slotListChange(void);

private:
    QMap<uint16_t, StateSystemProtocol::StateReportSignal> _stateReportSignal;
    QMap<uint16_t, StateSystemProtocol::StateReportSlot> _stateReportSlot;

    QMap<uint16_t, StateSystemProtocol::SignalState> _signalState;

    void _parseStateReport(MiniBus::Message msg);
    void _parseSignalInformationReport(MiniBus::Message msg);
    void _parseSlotInformationReport(MiniBus::Message msg);
};

#endif // STATESYSTEMPROTOCOL_H
