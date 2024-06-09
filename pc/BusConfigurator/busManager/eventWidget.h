#ifndef EVENTWIDGET_H
#define EVENTWIDGET_H

#include <QDialog>
#include "busDevice.h"
#include "triggerLine.h"
#include "eventSignalLine.h"
#include "protocol/eventProtocol.h"
#include "protocol/triggerProtocol.h"

namespace Ui {
class eventWidget;
}

class eventWidget : public QDialog
{
    Q_OBJECT

public:
    explicit eventWidget(busDevice *busDevice, QWidget *parent = nullptr);
    ~eventWidget();

    void updateTriggerList(QMap<uint8_t, QString> triggerList);
    void updateEventList(QList<eventSlot*> eventSlots);

private slots:

    void on_trigger(uint8_t triggerChannel);
    void on_eventActive(uint8_t eventChannel, bool active);
    void on_nameChange(void);

    void on_triggerListButton_clicked();

    void on_updateEventsButton_clicked();

private:
    Ui::eventWidget *ui;

    QList<triggerLine*> _triggerLines;
    QList<eventSignalLine*> _eventSignalLines;

    busDevice *_busDevice;

    triggerProtocol _triggerProtocol;
    eventProtocol _eventProtocol;
};

#endif // EVENTWIDGET_H
