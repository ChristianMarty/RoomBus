#include "triggerSlotLine.h"
#include "ui_triggerSlotLine.h"

TriggerSlotLineWidget::TriggerSlotLineWidget(TriggerProtocol::TriggerSlot* triggerSlot, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TriggerSlotLineWidget)
{
    ui->setupUi(this);
    _triggerSlot = triggerSlot;

    ui->label_name->setText(_triggerSlot->description);
    ui->label_channel->setText("Ch: 0x"+ QString::number(_triggerSlot->channel,16).rightJustified(4,'0'));
}

TriggerSlotLineWidget::~TriggerSlotLineWidget()
{
    delete ui;
}

void TriggerSlotLineWidget::on_pushButton_trigger_clicked()
{
    _triggerSlot->trigger->sendTrigger(_triggerSlot->channel);
}

