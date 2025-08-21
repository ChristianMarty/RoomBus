#ifndef TRIGGER_SLOT_LINE_WIDGET_H
#define TRIGGER_SLOT_LINE_WIDGET_H

#include <QWidget>
#include "protocol/triggerSystemProtocol.h"

namespace Ui {
class TriggerSlotLineWidget;
}

class TriggerSlotLineWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TriggerSlotLineWidget(TriggerSystemProtocol::TriggerSlot* triggerSlot, QWidget *parent = nullptr);
    ~TriggerSlotLineWidget();

signals:

private slots:

    void on_pushButton_trigger_clicked();

private:
    Ui::TriggerSlotLineWidget *ui;

    TriggerSystemProtocol::TriggerSlot* _triggerSlot;
};

#endif // TRIGGER_SLOT_LINE_WIDGET_H
