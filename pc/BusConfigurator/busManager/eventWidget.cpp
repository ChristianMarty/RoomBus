#include "eventWidget.h"
#include "ui_eventWidget.h"

eventWidget::eventWidget(busDevice *busDevice, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::eventWidget),
    _triggerProtocol(busDevice),
    _eventProtocol(busDevice)
{
    ui->setupUi(this);

    _busDevice = busDevice;
    connect(&_triggerProtocol,&triggerProtocol::triggerSlotListChange, this, &eventWidget::on_nameChange);
    connect(&_eventProtocol,&eventProtocol::eventSlotListChange, this, &eventWidget::on_nameChange);

    _triggerProtocol.requestTriggerSlotNames();
}

eventWidget::~eventWidget()
{
    delete ui;
}

void eventWidget::on_trigger(uint8_t triggerChannel)
{
    _triggerProtocol.sendTrigger(triggerChannel);
}

void eventWidget::on_eventActive(uint8_t eventChannel, bool active)
{
    _eventProtocol.setActiveState(eventChannel, active);
}

void eventWidget::updateTriggerList(QMap<uint8_t, QString> triggerList)
{
    QMapIterator<uint8_t, QString> i(triggerList);
    bool found;

    while (i.hasNext())
    {
        i.next();
        found = false;

        for(uint8_t j = 0; j<_triggerLines.size();j++)
        {
            if(_triggerLines.at(j)->channel() == i.key())
            {
                _triggerLines.at(j)->setName(i.value());
                found = true;
                break;
            }
        }

        if(!found)
        {
            triggerLine *temp = new triggerLine(i.value(),i.key(),this);
            ui->triggerLayout->insertWidget(ui->triggerLayout->count()-1,temp);

            connect(temp,&triggerLine::trigger, this, &eventWidget::on_trigger);

            _triggerLines.append(temp);
        }
    }
}

void eventWidget::updateEventList(QList<eventSlot *> eventSlots)
{
    bool found;

    for(int i = 0; i< eventSlots.size(); i++)
    {
        found = false;

        for(uint8_t j = 0; j<_eventSignalLines.size();j++)
        {
            if(_eventSignalLines.at(j)->channel() == eventSlots.at(i)->channel)
            {
                _eventSignalLines.at(j)->update(*eventSlots.at(i));
                found = true;
                break;
            }
        }

        if(!found)
        {
            eventSignalLine *temp = new eventSignalLine(*eventSlots.at(i),this);
            ui->eventLayout->insertWidget(ui->triggerLayout->count()-1,temp);
            connect(temp,&eventSignalLine::eventActive, this, &eventWidget::on_eventActive);
            _eventSignalLines.append(temp);
        }
    }
}

void eventWidget::on_nameChange(void)
{
  //  updateTriggerList(_triggerProtocol.triggerSlotNames());
    updateEventList(_eventProtocol.eventSlots());
}

void eventWidget::on_triggerListButton_clicked()
{
    _triggerProtocol.requestTriggerSlotNames();
}

void eventWidget::on_updateEventsButton_clicked()
{
    _eventProtocol.requestEventSlotNames();
}
