#include "triggerSignalLine.h"
#include "ui_triggerSignalLine.h"

TriggerSignalLineWidget::TriggerSignalLineWidget(TriggerSystemProtocol::TriggerSignal* triggerSignal, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TriggerSignalLineWidget)
{
    ui->setupUi(this);

    _triggerSignal = triggerSignal;

    ui->label_namel->setText(_triggerSignal->description);
    ui->label_channel->setText("Ch: 0x"+ QString::number(_triggerSignal->channel, 16).rightJustified(4,'0'));
}

TriggerSignalLineWidget::~TriggerSignalLineWidget()
{
    delete ui;
}
