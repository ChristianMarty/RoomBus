#include "tinyLoaderWidget.h"
#include "ui_tinyLoaderWidget.h"

#include <QFileDialog>



tinyLoaderWidget::tinyLoaderWidget(busDevice *busDevice, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tinyLoaderWidget)
{
    ui->setupUi(this);

    _tinyLoader = new tinyLoader(busDevice);

    connect(_tinyLoader, &tinyLoader::progressChange, this, &tinyLoaderWidget::on_progressChange);
    connect(_tinyLoader, &tinyLoader::busScanMessage, this, &tinyLoaderWidget::on_busScanMessage);
    connect(_tinyLoader, &tinyLoader::infoChange, this, &tinyLoaderWidget::on_infoChange);
    connect(_tinyLoader, &tinyLoader::hexInfoChange, this, &tinyLoaderWidget::on_hexInfoChange);
    connect(_tinyLoader, &tinyLoader::tcpBridgeStatus, this, &tinyLoaderWidget::on_tcpBridgeStatus);
}

tinyLoaderWidget::~tinyLoaderWidget()
{
    delete ui;
}

void tinyLoaderWidget::on_uploadButton_clicked()
{
    _tinyLoader->loadFirmware(ui->uploadPathEdit->text());
    _tinyLoader->startUpload();
}


void tinyLoaderWidget::on_toolButton_clicked()
{
    ui->uploadPathEdit->setText(QFileDialog::getOpenFileName(this,tr("Open HEX File"), "", tr("Hex Files (*.hex)|All files (*.*)")));
    _tinyLoader->loadFirmware(ui->uploadPathEdit->text());
}

void tinyLoaderWidget::on_progressChange(uint8_t progress)
{
  ui->uploadProgressBar->setValue(progress);
}

void tinyLoaderWidget::on_infoChange(void)
{
    tinyLoader::bootSystemInformation_t bootSystemInformation = _tinyLoader->bootSystemInformation();

    ui->bootloaderRevisionLabel->setText(QString::number(bootSystemInformation.bootloaderRevision,16).toUpper().rightJustified(2,'0').prepend("0x"));
    ui->hardwareRevisionLabel->setText(QString::number(bootSystemInformation.deviceHardwareRevision,16).toUpper().rightJustified(2,'0').prepend("0x"));
    ui->deviceIdLabel->setText(QString::number(bootSystemInformation.deviceId,16).toUpper().rightJustified(2,'0').prepend("0x"));

    ui->appOffsetLabel->setText(QString::number(bootSystemInformation.applicationStartAddress,16).toUpper().rightJustified(4,'0').prepend("0x"));
    ui->appSizeLabel->setText(QString::number(bootSystemInformation.applicationSize,16).toUpper().rightJustified(4,'0').prepend("0x"));

    ui->ramStartLabel->setText(QString::number(bootSystemInformation.applicationRamStartAddress,16).toUpper().rightJustified(4,'0').prepend("0x"));
    ui->ramSizeLabel->setText(QString::number(bootSystemInformation.applicationRamSize,16).toUpper().rightJustified(4,'0').prepend("0x"));

    ui->eepromStartLabel->setText(QString::number(bootSystemInformation.applicationEEPROMStartAddress,16).toUpper().rightJustified(4,'0').prepend("0x"));

    ui->eepromSizeLabel->setText(QString::number(bootSystemInformation.applicationEEPROMSize,16).toUpper().rightJustified(4,'0').prepend("0x"));

    ui->flashPageLabel->setText(QString::number(bootSystemInformation.flashPageSize,16).toUpper().rightJustified(2,'0').prepend("0x"));

    ui->label_appCrc->setText(QString::number(_tinyLoader->crc(),16).toUpper().rightJustified(2,'0').prepend("0x"));

    ui->label_appState->setText(_tinyLoader->deviceStateString());
}

void tinyLoaderWidget::on_hexInfoChange()
{
    tinyLoader::hexFileInformation_t hexFileInformation = _tinyLoader->hexFileInformation();

    ui->hexOffsetLabel->setText(QString::number(hexFileInformation.applicationStartAddress,16).toUpper().rightJustified(2,'0').prepend("0x"));
    ui->hexSizeLabel->setText(QString::number(hexFileInformation.applicationSize,16).toUpper().rightJustified(2,'0').prepend("0x"));
}

void tinyLoaderWidget::on_busScanButton_clicked()
{
    ui->busScanMessages->clear();
    _tinyLoader->startBusScan(300);
}

void tinyLoaderWidget::on_busScanMessage(QString message)
{
    ui->busScanMessages->append(message);
}


void tinyLoaderWidget::on_addressBox_valueChanged(int arg1)
{
    _tinyLoader->setDeviceAddress(arg1);
}

void tinyLoaderWidget::on_getAppCrcButton_clicked()
{
    _tinyLoader->requestApplicationCrc();
}


void tinyLoaderWidget::on_clearButton_clicked()
{
    ui->busScanMessages->clear();
}


void tinyLoaderWidget::on_pushButton_clicked()
{
    _tinyLoader->sendCommand(0x07);
}


void tinyLoaderWidget::on_abortBusScanButton_clicked()
{
    _tinyLoader->abortBusScan();
}


void tinyLoaderWidget::on_updateAddressButton_clicked()
{
    uint8_t newAddress = static_cast<uint8_t>(ui->newAddressBox->value());
    _tinyLoader->writeDeviceAddress(newAddress);
}


void tinyLoaderWidget::on_rebootButton_clicked()
{
    _tinyLoader->requestReboot();
}


void tinyLoaderWidget::on_tcpOpenButton_clicked()
{
    uint16_t port = ui->tcpPortEdit->text().toInt();

    _tinyLoader->tcpBridgeOpen(port);
}


void tinyLoaderWidget::on_tcpCloseButton_clicked()
{
    _tinyLoader->tcpBridgeClose();
}

void tinyLoaderWidget::on_tcpBridgeStatus(QString message, bool open, bool error)
{
    ui->tcpOpenButton->setEnabled(!open);
    ui->tcpCloseButton->setEnabled(open);
    ui->tcpStatusLabel->setText(message);
}

void tinyLoaderWidget::on_button_appStart_clicked()
{
    _tinyLoader->requestApplicationStart();
}


void tinyLoaderWidget::on_button_appStop_clicked()
{
    _tinyLoader->requestApplicationStop();
}


void tinyLoaderWidget::on_button_getDeviceState_clicked()
{
    _tinyLoader->requestDeviceState();
}


void tinyLoaderWidget::on_button_getDeviceInfo_clicked()
{
    _tinyLoader->requestDeviceInformation();
}


void tinyLoaderWidget::on_toolButton_triggered(QAction *arg1)
{

}

