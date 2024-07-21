#include "eventSlotLine.h"
#include "ui_eventSlotLine.h"

EventSlotLineWidget::EventSlotLineWidget(EventProtocol::EventSlot* eventSlot, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EventSlotLineWidget)
{
    ui->setupUi(this);

    _eventSlot = eventSlot;

    ui->label_name->setText(_eventSlot->description);
    ui->label_channel->setText("Ch: 0x"+ QString::number(_eventSlot->channel,16).rightJustified(4,'0'));
    ui->label_timeout->setText("Timeout: "+ QString::number(_eventSlot->timeout)+"ms");
}

EventSlotLineWidget::~EventSlotLineWidget()
{
    delete ui;
}

