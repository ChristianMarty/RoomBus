#ifndef TRIGGER_SLOT_LINE_WIDGET_H
#define TRIGGER_SLOT_LINE_WIDGET_H

#include <QWidget>
#include "protocol/triggerProtocol.h"

namespace Ui {
class TriggerSlotLineWidget;
}

class TriggerSlotLineWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TriggerSlotLineWidget(TriggerProtocol::TriggerSlot* triggerSlot, QWidget *parent = nullptr);
    ~TriggerSlotLineWidget();

signals:

private slots:

    void on_pushButton_trigger_clicked();

private:
    Ui::TriggerSlotLineWidget *ui;

    TriggerProtocol::TriggerSlot* _triggerSlot;
};

#endif // TRIGGER_SLOT_LINE_WIDGET_H
