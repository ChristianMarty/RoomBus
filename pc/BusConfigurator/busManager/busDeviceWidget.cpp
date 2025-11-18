#include "busDeviceWidget.h"
#include "ui_busDeviceWidget.h"

#include "settingsWidget.h"
#include "echoTestWidget.h"

BusDeviceWidget::BusDeviceWidget(RoomBusDevice *device, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::BusDeviceWidget)
    ,_device{device}
{
    ui->setupUi(this);

    connect(&_device->management(), &DeviceManagementProtocol::statusUpdate, this, &BusDeviceWidget::on_statusUpdate);
}

BusDeviceWidget::~BusDeviceWidget()
{
    delete ui;
}

void BusDeviceWidget::on_statusUpdate(void)
{
    updateData();
}

void BusDeviceWidget::updateData(void)
{
    ui->deviceNameLabel->setText(QString::number(_device->deviceAddress(),10)+" - "+_device->deviceName());
    ui->applicationNameLabel->setText(_device->applicationName());
    ui->hardwareNameLabel->setText(_device->hardwareName());
    ui->lastHeartbeatLabel->setText(_device->lastHeartbeat().toString("hh:mm:ss"));

    if(_device->systemStatus().identify == false)ui->identifyButton->setText("Identify");
    else ui->identifyButton->setText("Identify Off");

    if(_device->systemStatus().applicationRunOnStartup) ui->label_autostart->setStyleSheet("color: green;");
    else ui->label_autostart->setStyleSheet("color: orange;");

    if(_device->systemStatus().applicationCrcError) ui->label_applicationCRC->setStyleSheet("color: red;");
    else ui->label_applicationCRC->setStyleSheet("color: green;");

    DeviceManagementProtocol::SystemStatus systemStatus = _device->systemStatus();
    if(_device->timeoutStatus()){
        ui->appStatusLabel->setText("Timeout");
        ui->appStatusLabel->setStyleSheet("color: orange;");
    }else if(systemStatus.administratorAccess){
        ui->appStatusLabel->setText("Administrator Access");
        ui->appStatusLabel->setStyleSheet("color: blue;");
    }else{
        switch((DeviceManagementProtocol::ApplicationState)systemStatus.applicationState) {
            case DeviceManagementProtocol::ApplicationState::Stopped :
                ui->appStatusLabel->setText("Stopped");
                ui->appStatusLabel->setStyleSheet("color: red;");
                break;
            case DeviceManagementProtocol::ApplicationState::Starting :
                ui->appStatusLabel->setText("Starting");
                ui->appStatusLabel->setStyleSheet("color: green;");
                break;
            case DeviceManagementProtocol::ApplicationState::Running :
                ui->appStatusLabel->setText("Runnig");
                ui->appStatusLabel->setStyleSheet("color: green;");
                break;
            case DeviceManagementProtocol::ApplicationState::Shutdown :
                ui->appStatusLabel->setText("Shutdown");
                ui->appStatusLabel->setStyleSheet("color: red;");
                break;
        }
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
