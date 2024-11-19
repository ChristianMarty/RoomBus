#ifndef STATE_REPORT_WIDGET_H
#define STATE_REPORT_WIDGET_H

#include <QDialog>
#include "busDevice.h"
#include "protocol/stateReportProtocol.h"
#include "stateReportSignalLine.h"
#include "stateReportSlotLine.h"

namespace Ui {
class StateReportWidget;
}

class StateReportWidget : public QDialog
{
    Q_OBJECT
public:
    explicit StateReportWidget(busDevice *busDevice, QWidget *parent = nullptr);
    ~StateReportWidget();

private slots:

    void on_signalListChange(void);
    void on_slotListChange(void);

    void on_pushButton_refresh_clicked();
    void on_pushButton_clear_clicked();

private:
    Ui::StateReportWidget *ui;

    QList<StateReportSignalLineWidget*> _stateReportSignalLines;
    QList<StateReportSlotLineWidget*> _stateReportSlotLines;

    busDevice *_busDevice;

    class::StateSystemProtocol _stateReportProtocol;
};

#endif // STATE_REPORT_WIDGET_H
