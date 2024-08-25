#include "valueSlotLine.h"
#include "ui_valueSlotLine.h"

ValueSlotLineWidget::ValueSlotLineWidget(ValueProtocol::ValueSlot* valueSlot, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ValueSlotLineWidget)
{
    ui->setupUi(this);
    _valueSlot = valueSlot;

    ui->label_name->setText(_valueSlot->description);
    ui->label_channel->setText("Ch: 0x"+ QString::number(_valueSlot->channel,16).rightJustified(4,'0'));
}

ValueSlotLineWidget::~ValueSlotLineWidget()
{
    delete ui;
}
