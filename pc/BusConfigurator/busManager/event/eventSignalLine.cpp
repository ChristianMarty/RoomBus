#include "eventSignalLine.h"
#include "ui_eventSignalLine.h"

EventSignalLineWidget::EventSignalLineWidget(EventSystemProtocol::EventSignal *eventSignal, QWidget *parent):
    QWidget(parent),
    ui(new Ui::EventSignalLineWidget)
{
    ui->setupUi(this);

    _eventSignal = eventSignal;

    ui->label_name->setText(_eventSignal->description);
    ui->label_channel->setText("Ch: 0x"+ QString::number(_eventSignal->channel,16).rightJustified(4,'0'));
    ui->label_interval->setText("Interval: "+ QString::number(_eventSignal->interval)+"ms");
}

EventSignalLineWidget::~EventSignalLineWidget()
{
    delete ui;
}
