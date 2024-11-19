#include "stateReportSlotLine.h"
#include "ui_stateReportSlotLine.h"

StateReportSlotLineWidget::StateReportSlotLineWidget(StateSystemProtocol::StateReportSlot* stateReportSlot, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StateReportSlotLineWidget)
{
    ui->setupUi(this);
    _stateReportSlot = stateReportSlot;

    ui->label_name->setText(_stateReportSlot->description);
    ui->label_channel->setText("Ch: 0x"+ QString::number(_stateReportSlot->channel,16).rightJustified(4,'0'));
    ui->label_timeout->setText("Timeout: "+QString::number(_stateReportSlot->timeout)+"s");
}

StateReportSlotLineWidget::~StateReportSlotLineWidget()
{
    delete ui;
}

void StateReportSlotLineWidget::on_pushButton_trigger_clicked()
{
   // _triggerSlot->trigger->sendTrigger(_triggerSlot->channel);
}

