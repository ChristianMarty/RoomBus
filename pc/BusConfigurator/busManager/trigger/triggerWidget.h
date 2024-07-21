#ifndef TRIGGER_WIDGET_H
#define TRIGGER_WIDGET_H

#include <QDialog>
#include "busDevice.h"
#include "protocol/triggerProtocol.h"
#include "triggerSignalLine.h"
#include "triggerSlotLine.h"

namespace Ui {
class TriggerWidget;
}

class TriggerWidget : public QDialog
{
    Q_OBJECT
public:
    explicit TriggerWidget(busDevice *busDevice, QWidget *parent = nullptr);
    ~TriggerWidget();

private slots:

    void on_trigger(uint16_t triggerChannel);
    void on_signalListChange(void);
    void on_slotListChange(void);

    void on_pushButton_refresh_clicked();
    void on_pushButton_clear_clicked();

private:
    Ui::TriggerWidget *ui;

    QList<TriggerSignalLineWidget*> _triggerSignalLines;
    QList<TriggerSlotLineWidget*> _triggerSlotLines;

    busDevice *_busDevice;

    class::TriggerProtocol _triggerProtocol;
};

#endif // TRIGGER_WIDGET_H
