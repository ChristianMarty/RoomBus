#include "busMonitorLine.h"
#include "ui_busMonitorLine.h"

#include "protocolDecoder.h"

BusMonitorLine::BusMonitorLine(QWidget *parent):
    QWidget(parent),
    ui(new Ui::BusMonitorLine)
{
    ui->setupUi(this);
}

BusMonitorLine::BusMonitorLine(RoomBus::Message &msg, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BusMonitorLine)
{
    ui->setupUi(this);

    ui->label_timestamp->setText(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"));
    ui->label_source->setText(QString::number(msg.sourceAddress,10));
    ui->label_destination->setText(QString::number(msg.destinationAddress,10));
    ui->label_protocol->setText(ProtocolDecoder::protocolName(msg.protocol));
    ui->label_command->setText(ProtocolDecoder::commandName(msg.protocol, msg.command));

    ui->label_data->setText((QString)msg.data.toHex(' ').toUpper());
    ui->label_decoded->setText(ProtocolDecoder::dataDecoded(msg.protocol, msg.command, msg.data));
}

BusMonitorLine::~BusMonitorLine()
{
    delete ui;
}
