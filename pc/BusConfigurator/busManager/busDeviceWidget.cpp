#include "busDeviceWidget.h"
#include "ui_busDeviceWidget.h"

#include "settingsWidget.h"
#include "echoTestWidget.h"

busDeviceWidget::busDeviceWidget(busDevice *device, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::busDeviceWidget)
{
    ui->setupUi(this);

    _device = device;
}

busDeviceWidget::~busDeviceWidget()
{
    delete ui;
}

void busDeviceWidget::updateData(void)
{
    ui->deviceNameLabel->setText(QString::number(_device->deviceAddress(),10)+" - "+_device->deviceName());
    ui->applicationNameLabel->setText(_device->applicationName());
    ui->hardwareNameLabel->setText(_device->hardwareName());
    ui->lastHeartbeatLabel->setText(_device->lastHeartbeat().toString("hh:mm:ss"));

    if(_device->sysStatus().identify == false)ui->identifyButton->setText("Identify");
    else ui->identifyButton->setText("Identify Off");

    if(_device->sysStatus().appRunOnStartup) ui->appRunStartupStatus->setStyleSheet("background-color: green;");
    else ui->appRunStartupStatus->setStyleSheet("background-color: orange;");

    if(_device->sysStatus().appCrcError == false) ui->appCrcStatus->setStyleSheet("background-color: green;");
    else ui->appCrcStatus->setStyleSheet("background-color: red;");

    if(_device->timeoutStatus())
    {
        ui->appStatusLabel->setText("Timeout");
        ui->appStatusLabel->setStyleSheet("color: orange;");
    }
    else if(_device->sysStatus().appRuning)
    {
        ui->appStatusLabel->setText("Runnig");
        ui->appStatusLabel->setStyleSheet("color: green;");
    }
    else
    {
        ui->appStatusLabel->setText("Stopped");
        ui->appStatusLabel->setStyleSheet("color: red;");
    }
}

void busDeviceWidget::on_identifyButton_clicked()
{
    busDevice::sysControl_t temp;
    temp.reg = 0;
    temp.bit.identify = true;

    if(_device->sysStatus().identify == false)_device->writeSetControl(temp);
    else _device->writeClearControl(temp);
}

void busDeviceWidget::on_showButton_clicked()
{
    emit busDeviceShow(_device);
}
