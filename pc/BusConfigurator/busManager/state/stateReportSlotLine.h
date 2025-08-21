#ifndef STATE_REPORT_SLOT_LINE_WIDGET_H
#define STATE_REPORT_SLOT_LINE_WIDGET_H

#include <QWidget>
#include "protocol/stateSystemProtocol.h"

namespace Ui {
class StateReportSlotLineWidget;
}

class StateReportSlotLineWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StateReportSlotLineWidget(StateSystemProtocol::StateReportSlot* stateReportSlot, QWidget *parent = nullptr);
    ~StateReportSlotLineWidget();

signals:

private slots:

    void on_pushButton_trigger_clicked();

private:
    Ui::StateReportSlotLineWidget *ui;

    StateSystemProtocol::StateReportSlot* _stateReportSlot;
};

#endif // STATE_REPORT_SLOT_LINE_WIDGET_H
