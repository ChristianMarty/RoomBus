#ifndef VALUE_WIDGET_H
#define VALUE_WIDGET_H

#include <QDialog>
#include "busDevice.h"
#include "protocol/valueProtocol.h"
#include "valueSignalLine.h"
#include "valueSlotLine.h"

namespace Ui {
class ValueWidget;
}

class ValueWidget : public QDialog
{
    Q_OBJECT
public:
    explicit ValueWidget(busDevice *busDevice, QWidget *parent = nullptr);
    ~ValueWidget();

private slots:

    void on_trigger(uint16_t triggerChannel);
    void on_signalListChange(void);
    void on_slotListChange(void);

    void on_pushButton_refresh_clicked();
    void on_pushButton_clear_clicked();

private:
    Ui::ValueWidget *ui;

    QList<ValueSignalLineWidget*> _triggerSignalLines;
    QList<ValueSlotLineWidget*> _triggerSlotLines;

    busDevice *_busDevice;

    class::ValueSystemProtocol _valueProtocol;
};

#endif // VALUE_WIDGET_H
