#ifndef STATEREPORTPROTOCOL_H
#define STATEREPORTPROTOCOL_H

#include <QObject>
#include <QMap>
#include "busProtocol.h"

class StateSystemProtocol : public BusProtocol
{
    Q_OBJECT
public:
    typedef enum {
        off = 0,
        on = 1,
        transitioning = 2,
        undefined = 3,
        unknown = 4
    } SignalState;

    enum SerialBridgeType {
        SBC_Type_UART = 0x00,
        SBC_Type_I2C = 0x01
    };

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

    void pushData(RoomBus::Message msg);
    QList<RoomBus::Protocol> protocol(void);

    void requestSignalInformation(void);
    void requestSlotInformation(void);

    void requestAllState(void);

    void reset(void);

    QList<StateSystemProtocol::StateReportSignal*> stateReportSignls();
    QList<StateSystemProtocol::StateReportSlot*> stateReportSlots();

    QMap<uint16_t, StateSystemProtocol::StateReportSignal> stateReportSignalMap() const;
    QMap<uint16_t, StateSystemProtocol::StateReportSlot> stateReportSlotMap() const;

signals:
    void signalStateChnage(uint16_t channel, SignalState newState);

    void signalListChange(void);
    void slotListChange(void);

private:
    QMap<uint16_t, StateSystemProtocol::StateReportSignal> _stateReportSignal;
    QMap<uint16_t, StateSystemProtocol::StateReportSlot> _stateReportSlot;

    QMap<uint16_t, StateSystemProtocol::SignalState> _signalState;

    void _parseStateReport(RoomBus::Message msg);
    void _parseSignalInformationReport(RoomBus::Message msg);
    void _parseSlotInformationReport(RoomBus::Message msg);
};

#endif // STATEREPORTPROTOCOL_H
