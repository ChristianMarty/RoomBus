#ifndef STATE_REPORT_SIGNAL_LINE_WIDGET_H
#define STATE_REPORT_SIGNAL_LINE_WIDGET_H

#include <QWidget>
#include "protocol/stateReportProtocol.h"

namespace Ui {
class StateReportSignalLineWidget;
}

class StateReportSignalLineWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StateReportSignalLineWidget(class::StateReportProtocol* protocol, StateReportProtocol::StateReportSignal* stateReportSignal, QWidget *parent = nullptr);
    ~StateReportSignalLineWidget();

signals:

private slots:
    void on_signalStateChnage(uint16_t channel, StateReportProtocol::SignalState newState);
private:
    Ui::StateReportSignalLineWidget *ui;

    StateReportProtocol::StateReportSignal* _stateReportSignal;
    class::StateReportProtocol* _protocol;
};

#endif // STATE_REPORT_SIGNAL_LINE_WIDGET_H
