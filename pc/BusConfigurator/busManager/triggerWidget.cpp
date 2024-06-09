#include "triggerWidget.h"
#include "ui_triggerWidget.h"

triggerWidget::triggerWidget(busDevice *busDevice, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::triggerWidget),
    _triggerProtocol(busDevice)
{
    ui->setupUi(this);

    _busDevice = busDevice;
    connect(&_triggerProtocol,&triggerProtocol::triggerSlotListChange, this, &triggerWidget::on_nameChange);

    _triggerProtocol.requestTriggerSlotNames();
}

triggerWidget::~triggerWidget()
{
    delete ui;
}

void triggerWidget::on_trigger(uint8_t triggerChannel)
{
    _triggerProtocol.sendTrigger(triggerChannel);
}
void triggerWidget::updateTriggerList(QMap<uint8_t, triggerProtocol::triggerSlot> triggerList)
{
    QMapIterator<uint8_t, triggerProtocol::triggerSlot> i(triggerList);
    bool found;

    while (i.hasNext())
    {
        i.next();
        found = false;

        for(uint8_t j = 0; j<_triggerLines.size();j++)
        {
            if(_triggerLines.at(j)->channel() == i.key())
            {
                _triggerLines.at(j)->setName(i.value().name);
                found = true;
                break;
            }
        }

        if(!found)
        {
            triggerLine *temp = new triggerLine(i.value().name,i.value().channel,this);
            ui->triggerLayout->insertWidget(ui->triggerLayout->count()-1,temp);

            connect(temp,&triggerLine::trigger, this, &triggerWidget::on_trigger);

            _triggerLines.append(temp);
        }
    }
}


void triggerWidget::on_nameChange(void)
{
    updateTriggerList(_triggerProtocol.triggerSlotMap());
}

void triggerWidget::on_triggerListButton_clicked()
{
    _triggerProtocol.requestTriggerSlotNames();
}


void triggerWidget::on_clearListButton_clicked()
{
    triggerLine * line;
    foreach(line, _triggerLines)
    {
        delete line;

    }

    _triggerLines.clear();
    _triggerProtocol.reset();

}
