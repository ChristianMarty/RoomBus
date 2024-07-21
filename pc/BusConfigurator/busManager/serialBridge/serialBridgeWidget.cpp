#include "serialBridgeWidget.h"
#include "ui_serialBridgeWidget.h"

#include <QFileDialog>

SerialBridgeWidget::SerialBridgeWidget(busDevice *busDevice, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SerialBridgeWidget)
{
    ui->setupUi(this);

    _tinyLoader = new SerialBridge(busDevice);

    connect(_tinyLoader, &SerialBridge::busMessage, this, &SerialBridgeWidget::on_busMessage);
    connect(_tinyLoader, &SerialBridge::tcpBridgeStatus, this, &SerialBridgeWidget::on_tcpBridgeStatus);
}

SerialBridgeWidget::~SerialBridgeWidget()
{
    delete ui;
}

void SerialBridgeWidget::on_busMessage(QString message)
{
    ui->busMessages->append(message);
}

void SerialBridgeWidget::on_tcpOpenButton_clicked()
{
    uint16_t port = ui->tcpPortEdit->text().toInt();

    _tinyLoader->tcpBridgeOpen(port);
}

void SerialBridgeWidget::on_tcpCloseButton_clicked()
{
    _tinyLoader->tcpBridgeClose();
}

void SerialBridgeWidget::on_tcpBridgeStatus(QString message, bool open, bool error)
{
    ui->tcpOpenButton->setEnabled(!open);
    ui->tcpCloseButton->setEnabled(open);
    ui->tcpStatusLabel->setText(message);
}
