#include "stateReportSignalLine.h"
#include "ui_stateReportSignalLine.h"

StateReportSignalLineWidget::StateReportSignalLineWidget(class::StateSystemProtocol* protocol, StateSystemProtocol::StateReportSignal* stateReportSignal, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StateReportSignalLineWidget)
{
    ui->setupUi(this);

    _stateReportSignal = stateReportSignal;
    _protocol = protocol;

    connect(protocol,&StateSystemProtocol::signalStateChnage, this, &on_signalStateChnage);

    ui->label_namel->setText(_stateReportSignal->description);
    ui->label_channel->setText("Ch: 0x"+ QString::number(_stateReportSignal->channel, 16).rightJustified(4,'0'));
    ui->label_interval->setText("Interval: "+QString::number(_stateReportSignal->interval)+"s");
}

StateReportSignalLineWidget::~StateReportSignalLineWidget()
{
    delete ui;
}

void StateReportSignalLineWidget::on_signalStateChnage(uint16_t channel, StateSystemProtocol::SignalState newState)
{
    if(channel != _stateReportSignal->channel) return;

    switch(newState){
        case StateSystemProtocol::off: ui->label_state->setText("Off"); break;
        case StateSystemProtocol::on: ui->label_state->setText("On"); break;
        case StateSystemProtocol::transitioning: ui->label_state->setText("Transitioning"); break;
        case StateSystemProtocol::undefined: ui->label_state->setText("Undefined"); break;
        default: ui->label_state->setText("Unknown"); break;
    }
}
