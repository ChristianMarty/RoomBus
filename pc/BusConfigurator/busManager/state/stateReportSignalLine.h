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
    explicit StateReportSignalLineWidget(class::StateSystemProtocol* protocol, StateSystemProtocol::StateReportSignal* stateReportSignal, QWidget *parent = nullptr);
    ~StateReportSignalLineWidget();

signals:

private slots:
    void on_signalStateChnage(uint16_t channel, StateSystemProtocol::SignalState newState);

private:
    Ui::StateReportSignalLineWidget *ui;

    StateSystemProtocol::StateReportSignal* _stateReportSignal;
    class::StateSystemProtocol* _protocol;
};

#endif // STATE_REPORT_SIGNAL_LINE_WIDGET_H
