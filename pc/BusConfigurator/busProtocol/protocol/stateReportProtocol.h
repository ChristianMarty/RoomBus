#ifndef STATEREPORTPROTOCOL_H
#define STATEREPORTPROTOCOL_H

#include <QObject>
#include <QMap>
#include "busProtocol.h"

class stateReportProtocol : public BusProtocol
{
    Q_OBJECT
public:
    enum commands_t{
        group0Report  = 0,
        group1Report  = 1,
        individualStateReport = 4,
        stateReportRequest = 5,
        stateReportChannelNameReporting = 6,
        stateReportChannelNameRequest= 7
    };

    enum state_t {
        off = 0,
        on = 1,
        transitioning = 2,
        undefined = 3,
        unknown = 4
    };

    stateReportProtocol(busDevice *device);

    void pushData(busMessage msg);
    QList<Protocol> protocol(void);

    void requestStateReportChannelNames(void);

    void requestState(uint8_t channel);
    void requestStateAll(void);

    void reset(void);

    QMap<uint8_t, QString> stateReportNames() const;
    QMap<uint8_t, state_t> stateReports() const;

signals:
    void stateReportListChange(void);
    void stateChange(void);

private:
    QMap<uint8_t, QString> _stateReportNames;
    QMap<uint8_t, state_t> _stateReports;

    void parseStateNameReport(busMessage msg);
    void parseGroup0Report(busMessage msg);
    void parseIndividualReport(busMessage msg);
};

#endif // STATEREPORTPROTOCOL_H
