#ifndef STATE_REPORT_SLOT_LINE_WIDGET_H
#define STATE_REPORT_SLOT_LINE_WIDGET_H

#include <QWidget>
#include "protocol/stateReportProtocol.h"

namespace Ui {
class StateReportSlotLineWidget;
}

class StateReportSlotLineWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StateReportSlotLineWidget(StateReportProtocol::StateReportSlot* stateReportSlot, QWidget *parent = nullptr);
    ~StateReportSlotLineWidget();

signals:

private slots:

    void on_pushButton_trigger_clicked();

private:
    Ui::StateReportSlotLineWidget *ui;

    StateReportProtocol::StateReportSlot* _stateReportSlot;
};

#endif // STATE_REPORT_SLOT_LINE_WIDGET_H
