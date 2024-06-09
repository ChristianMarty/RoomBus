#include "stateReportLine.h"
#include "ui_stateReportLine.h"

stateReportLine::stateReportLine(QString name, uint8_t channel, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::stateReportLine)
{
    ui->setupUi(this);

    ui->nameLabel->setText(name);
    ui->channelLabel->setText("Ch: "+ QString::number(channel,10));

    _channel = channel;
}

stateReportLine::~stateReportLine()
{
    delete ui;
}

void stateReportLine::setState(QString state)
{
    ui->stateLabel->setText(state);
}

void stateReportLine::setName(QString name)
{
    ui->nameLabel->setText(name);
}

uint8_t stateReportLine::channel() const
{
    return _channel;
}
