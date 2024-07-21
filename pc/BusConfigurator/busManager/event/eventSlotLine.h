#ifndef EVENT_SLOT_LINE_WIDGET_H
#define EVENT_SLOT_LINE_WIDGET_H

#include <QWidget>
#include "protocol/eventProtocol.h"

namespace Ui {
class EventSlotLineWidget;
}

class EventSlotLineWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EventSlotLineWidget(EventProtocol::EventSlot* eventSlot, QWidget *parent = nullptr);
    ~EventSlotLineWidget();

signals:

private slots:

private:
    Ui::EventSlotLineWidget *ui;
    EventProtocol::EventSlot* _eventSlot;
};

#endif // EVENT_SLOT_LINE_WIDGET_H
