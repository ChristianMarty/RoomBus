#ifndef TRIGGER_SIGNAL_LINE_WIDGET_H
#define TRIGGER_SIGNAL_LINE_WIDGET_H

#include <QWidget>
#include "protocol/triggerProtocol.h"

namespace Ui {
class TriggerSignalLineWidget;
}

class TriggerSignalLineWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TriggerSignalLineWidget(TriggerProtocol::TriggerSignal* triggerSignal, QWidget *parent = nullptr);
    ~TriggerSignalLineWidget();

signals:

private slots:

private:
    Ui::TriggerSignalLineWidget *ui;

    TriggerProtocol::TriggerSignal* _triggerSignal;
};

#endif // TRIGGER_SIGNAL_LINE_WIDGET_H
