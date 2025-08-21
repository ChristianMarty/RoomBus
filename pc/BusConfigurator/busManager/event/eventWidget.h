#ifndef EVENT_WIDGET_H
#define EVENT_WIDGET_H

#include <QDialog>
#include "busDevice.h"
#include "protocol/eventSystemProtocol.h"
#include "eventSignalLine.h"
#include "eventSlotLine.h"

namespace Ui {
class EventWidget;
}

class EventWidget : public QDialog
{
    Q_OBJECT
public:
    explicit EventWidget(RoomBusDevice *busDevice, QWidget *parent = nullptr);
    ~EventWidget();

private slots:

    void on_eventActive(uint8_t eventChannel, bool active);
    void on_signalListChange(void);
    void on_slotListChange(void);

    void on_pushButton_refresh_clicked();
    void on_pushButton_clear_clicked();

private:
    Ui::EventWidget *ui;

    QList<EventSignalLineWidget*> _eventSignalLines;
    QList<EventSlotLineWidget*> _eventSlotLines;

    RoomBusDevice *_busDevice;

    class::EventSystemProtocol _eventProtocol;
};

#endif // EVENT_WIDGET_H
