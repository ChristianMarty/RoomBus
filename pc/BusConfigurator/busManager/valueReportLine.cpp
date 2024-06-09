#include "valueReportLine.h"
#include "ui_valueReportLine.h"
#include <QDateTime>

valueReportLine::valueReportLine(valueReportProtocol::valueItem_t item, valueReportProtocol *valueReportProtocol, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::valueReportLine)
{
    ui->setupUi(this);

    this->_valueReportProtocol = valueReportProtocol;

    this->_item = item;
    ui->nameLabel->setText("CH: "+QString::number(item.channel)+" - "+item.description);
    ui->minLabel->setText(QString::number(item.min));
    ui->maxLabel->setText(QString::number(item.max));
    ui->valueLabel->setText(QString::number(item.value));

    ui->horizontalSlider->setMinimum(item.min);
    ui->horizontalSlider->setMaximum(item.max);
}

uint16_t valueReportLine::getChannel(void)
{
    return this->_item.channel;
}

valueReportLine::~valueReportLine()
{
    delete ui;
}

void valueReportLine::updateValue(float value)
{
    ui->valueLabel->setText(QString::number(value));
    ui->timestampLabel->setText(QTime::currentTime().toString());

     ui->horizontalSlider->setValue(int(value));
}

void valueReportLine::on_horizontalSlider_sliderMoved(int position)
{

}
void valueReportLine::on_horizontalSlider_actionTriggered(int action)
{
    _valueReportProtocol->setValue(_item.channel, ui->horizontalSlider->value());
    ui->valueLabel->setText(QString::number(ui->horizontalSlider->value()));
}
