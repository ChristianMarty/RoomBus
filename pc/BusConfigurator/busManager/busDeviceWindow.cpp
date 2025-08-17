#include "busDeviceWindow.h"
#include "ui_busDeviceWindow.h"
#include <QMdiSubWindow>

#include "settingsWidget.h"
#include "trigger/triggerWidget.h"
#include "event/eventWidget.h"
#include "state/stateReportWidget.h"
#include "value/valueWidget.h"
#include "fileTransferWidget.h"
#include "echoTestWidget.h"
#include "eepromWidget.h"
#include "serialBridge/serialBridgeWidget.h"

BusDeviceWindow::BusDeviceWindow(RoomBusDevice *device, QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::BusDeviceWindow)
    ,_device{device}
{
    ui->setupUi(this);

    ui->logWidget->assignDevice(device);

    updateData();
    connect(&_device->management(), &DeviceManagementProtocol::statusUpdate, this, &BusDeviceWindow::on_statusUpdate);
}

BusDeviceWindow::~BusDeviceWindow()
{
    delete ui;
}

void BusDeviceWindow::on_statusUpdate(void)
{
    updateStatus();
    updateData();
}

void BusDeviceWindow::updateData(void)
{
    this->setWindowTitle(QString::number(_device->deviceAddress(),10)+" - "+_device->deviceName()+" - "+_device->applicationName());

    ui->kernelVersionLabel->setText(_device->kernelVersionString());
    ui->hwVersionLabel->setText(_device->hardwareVersionString());
    ui->deviceLabel->setText(_device->deviceIdentificationString());
    ui->serialNumberLabel->setText(_device->deviceSerialNumberString());

    updateStatus();
}

void BusDeviceWindow::updateStatus()
{
    DeviceManagementProtocol::SystemStatus systemStatus = _device->systemStatus();
    if(_device->timeoutStatus()){
        ui->label_runStatus->setText("Timeout");
        ui->label_runStatus->setStyleSheet("font-weight: bold; color: orange;");
    }else if(systemStatus.applicationRuning){
        ui->label_runStatus->setText("Runnig");
        ui->label_runStatus->setStyleSheet("font-weight: bold; color: green;");
    }else{
        ui->label_runStatus->setText("Stopped");
        ui->label_runStatus->setStyleSheet("font-weight: bold; color: red;");
    }

    ui->label_administrationMode->setEnabled(systemStatus.administrationMode);
    ui->label_watchdogError->setEnabled(systemStatus.watchdogError);
    ui->label_watchdogWarning->setEnabled(systemStatus.watchdogWarning);
    ui->label_txBufferOverrun->setEnabled(systemStatus.txBufferOverrun);
    ui->label_txMessageOverrun->setEnabled(systemStatus.txMessageOverrun);
    ui->label_rxBufferOverrun->setEnabled(systemStatus.rxBufferOverrun);

    if(_device->systemStatus().identify == false)ui->pushButton_identify->setText("Identify");
    else ui->pushButton_identify->setText("Identify Off");
}

void BusDeviceWindow::on_triggerButton_clicked()
{
    QMdiSubWindow *triggerWindow = new QMdiSubWindow;
    triggerWindow->setWidget(new TriggerWidget(_device));
    triggerWindow->setAttribute(Qt::WA_DeleteOnClose);
    triggerWindow->setWindowTitle("Trigger");
    ui->mdiArea->addSubWindow(triggerWindow);
    triggerWindow->show();
    triggerWindow->setFocus();
}

void BusDeviceWindow::on_pushButton_event_clicked()
{
    QMdiSubWindow *eventWindow= new QMdiSubWindow;
    eventWindow->setWidget(new EventWidget(_device));
    eventWindow->setAttribute(Qt::WA_DeleteOnClose);
    eventWindow->setWindowTitle("Event");
    ui->mdiArea->addSubWindow(eventWindow);
    eventWindow->show();
    eventWindow->setFocus();
}

void BusDeviceWindow::on_settingsButton_clicked()
{
    QMdiSubWindow *settingsWindow = new QMdiSubWindow;
    settingsWindow->setWidget(new SettingsWidget(_device));
    settingsWindow->setAttribute(Qt::WA_DeleteOnClose);
    settingsWindow->setWindowTitle("Settings");
    ui->mdiArea->addSubWindow(settingsWindow);
    settingsWindow->show();
    settingsWindow->setFocus();
}

void BusDeviceWindow::on_stateButton_clicked()
{
    QMdiSubWindow *stateReportWindow = new QMdiSubWindow;
    stateReportWindow->setWidget(new StateReportWidget(_device));
    stateReportWindow->setAttribute(Qt::WA_DeleteOnClose);
    stateReportWindow->setWindowTitle("State Report");
    ui->mdiArea->addSubWindow(stateReportWindow);
    stateReportWindow->show();
    stateReportWindow->setFocus();
}

void BusDeviceWindow::on_echoButton_clicked()
{
    QMdiSubWindow *echoWindow = new QMdiSubWindow;
    echoWindow->setWidget(new EchoTestWidget(_device));
    echoWindow->setAttribute(Qt::WA_DeleteOnClose);
    echoWindow->setWindowTitle("Echo Test");
    ui->mdiArea->addSubWindow(echoWindow);
    echoWindow->show();
    echoWindow->setFocus();
}

void BusDeviceWindow::on_valueButton_clicked()
{
    QMdiSubWindow *valueReportWindow = new QMdiSubWindow;
    valueReportWindow->setWidget(new ValueWidget(_device));
    valueReportWindow->setAttribute(Qt::WA_DeleteOnClose);
    valueReportWindow->setWindowTitle("Value Report");
    ui->mdiArea->addSubWindow(valueReportWindow);
    valueReportWindow->show();
    valueReportWindow->setFocus();
}

void BusDeviceWindow::on_fileButton_clicked()
{
    QMdiSubWindow *fileTransferWindow = new QMdiSubWindow;
    fileTransferWindow->setWidget(new FileTransferWidget(_device));
    fileTransferWindow->setAttribute(Qt::WA_DeleteOnClose);
    fileTransferWindow->setWindowTitle("File Transfer");
    ui->mdiArea->addSubWindow(fileTransferWindow);
    fileTransferWindow->show();
    fileTransferWindow->setFocus();
}

void BusDeviceWindow::on_pushButton_eeprom_clicked()
{
    QMdiSubWindow *eepromWindow = new QMdiSubWindow;
    eepromWindow->setWidget(new EepromWidget(_device));
    eepromWindow->setAttribute(Qt::WA_DeleteOnClose);
    eepromWindow->setWindowTitle("EEPROM");
    ui->mdiArea->addSubWindow(eepromWindow);
    eepromWindow->show();
    eepromWindow->setFocus();
}

void BusDeviceWindow::on_pushButton_serialBridge_clicked()
{
    QMdiSubWindow *serialBridgeWindow = new QMdiSubWindow;
    serialBridgeWindow->setWidget(new SerialBridgeWidget(_device));
    serialBridgeWindow->setAttribute(Qt::WA_DeleteOnClose);
    ui->mdiArea->addSubWindow(serialBridgeWindow);
    serialBridgeWindow->show();
    serialBridgeWindow->setFocus();
}

void BusDeviceWindow::on_rebootButton_clicked()
{
    _device->management().requestSystemRestart();
}

void BusDeviceWindow::on_pushButton_identify_clicked()
{
    DeviceManagementProtocol::SystemControl temp;
    temp.reg = 0;
    temp.bit.identify = true;

    if(_device->systemStatus().identify == false)_device->management().writeSetControl(temp);
    else _device->management().writeClearControl(temp);
}


