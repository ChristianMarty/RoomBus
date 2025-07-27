#include "busDeviceWidget.h"
#include "ui_busDeviceWidget.h"

#include "settingsWidget.h"
#include "echoTestWidget.h"

BusDeviceWidget::BusDeviceWidget(RoomBusDevice *device, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BusDeviceWidget)
{
    ui->setupUi(this);

    _device = device;
}

BusDeviceWidget::~BusDeviceWidget()
{
    delete ui;
}

void BusDeviceWidget::updateData(void)
{
    ui->deviceNameLabel->setText(QString::number(_device->deviceAddress(),10)+" - "+_device->deviceName());
    ui->applicationNameLabel->setText(_device->applicationName());
    ui->hardwareNameLabel->setText(_device->hardwareName());
    ui->lastHeartbeatLabel->setText(_device->lastHeartbeat().toString("hh:mm:ss"));

    if(_device->systemStatus().identify == false)ui->identifyButton->setText("Identify");
    else ui->identifyButton->setText("Identify Off");

    if(_device->systemStatus().appRunOnStartup) ui->label_autostart->setStyleSheet("color: green;");
    else ui->label_autostart->setStyleSheet("color: orange;");

    if(_device->systemStatus().appCrcError) ui->label_applicationCRC->setStyleSheet("color: red;");
    else ui->label_applicationCRC->setStyleSheet("color: green;");

    if(_device->timeoutStatus()){
        ui->appStatusLabel->setText("Timeout");
        ui->appStatusLabel->setStyleSheet("color: orange;");
    }else if(_device->systemStatus().appRuning){
        ui->appStatusLabel->setText("Runnig");
        ui->appStatusLabel->setStyleSheet("color: green;");
    }else{
        ui->appStatusLabel->setText("Stopped");
        ui->appStatusLabel->setStyleSheet("color: red;");
    }
}

void BusDeviceWidget::on_identifyButton_clicked()
{
    DeviceManagementProtocol::SystemControl temp;
    temp.reg = 0;
    temp.bit.identify = true;

    if(_device->systemStatus().identify == false)_device->management().writeSetControl(temp);
    else _device->management().writeClearControl(temp);
}

void BusDeviceWidget::on_showButton_clicked()
{
    emit busDeviceShow(_device);
}
