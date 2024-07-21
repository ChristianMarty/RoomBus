#ifndef EVENT_SIGNAL_LINE_WIDGET_H
#define EVENT_SIGNAL_LINE_WIDGET_H

#include <QWidget>
#include "protocol/eventProtocol.h"

namespace Ui {
class EventSignalLineWidget;
}

class EventSignalLineWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EventSignalLineWidget(EventProtocol::EventSignal *eventSignal, QWidget *parent = nullptr);
    ~EventSignalLineWidget();

signals:

private slots:

private:
    Ui::EventSignalLineWidget *ui;
    EventProtocol::EventSignal *_eventSignal;
};

#endif // EVENT_SIGNAL_LINE_WIDGET_H
