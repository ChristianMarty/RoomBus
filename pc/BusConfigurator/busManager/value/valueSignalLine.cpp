#include "valueSignalLine.h"
#include "ui_valueSignalLine.h"
#include "protocol/valueProtocol.h"

ValueSignalLineWidget::ValueSignalLineWidget(ValueProtocol* protocol, ValueProtocol::ValueSignal* valueSignal, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ValueSignalLineWidget)
{
    ui->setupUi(this);

    _valueSignal = valueSignal;
    _protocol = protocol;
    connect(protocol,&ValueProtocol::signalValueChnage, this, &ValueSignalLineWidget::on_signalValueChnage);


    ui->label_name->setText(_valueSignal->description);
    ui->label_channel->setText("Ch: 0x"+ QString::number(_valueSignal->channel, 16).rightJustified(4,'0'));
    ValueProtocol::UnitType type = ValueProtocol::uomToType(_valueSignal->uom);
    ui->label_minimum->setText(ValueProtocol::valueToString(type, _valueSignal->minimum));
    ui->label_maximum->setText(ValueProtocol::valueToString(type ,_valueSignal->maximum));

    ui->label_readOnly->setEnabled(_valueSignal->readOnly);

    ui->label_uom->setText(ValueProtocol::uomName(_valueSignal->uom));

    if(_valueSignal->readOnly){
        ui->pushButton_write->setEnabled(false);
        ui->lineEdit_value->setEnabled(false);
    }else{
        ui->pushButton_write->setEnabled(true);
        ui->lineEdit_value->setEnabled(true);
    }
}

ValueSignalLineWidget::~ValueSignalLineWidget()
{
    delete ui;
}

void ValueSignalLineWidget::on_signalValueChnage(uint16_t channel)
{
    if(channel != _valueSignal->channel) return;

    ui->label_value->setText(_protocol->valueString(channel));
}

void ValueSignalLineWidget::on_pushButton_write_clicked()
{
    ValueProtocol::ValueData value = ValueProtocol::stringToValue(ValueProtocol::uomToType(_valueSignal->uom), ui->lineEdit_value->text());
   _protocol->sendValueCommand(_valueSignal->channel, value);
}
