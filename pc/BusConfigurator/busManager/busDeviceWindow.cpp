#include "busDeviceWindow.h"
#include "ui_busDeviceWindow.h"
#include <QMdiSubWindow>

BusDeviceWindow::BusDeviceWindow(RoomBusDevice *device, QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::BusDeviceWindow)
{
    ui->setupUi(this);
    _device = device;

    ui->logWidget->assignDevice(device);

    updateData();
}

BusDeviceWindow::~BusDeviceWindow()
{
    if(_settingsWidget != nullptr) delete _settingsWidget;
    if(_echoWidget != nullptr) delete _echoWidget;
    if(_triggerWidget != nullptr) delete _triggerWidget;
    if(_stateReportWidget != nullptr) delete _stateReportWidget;
    if(_valueReportWidget != nullptr) delete _valueReportWidget;
    if(_fileTransferWidget != nullptr) delete  _fileTransferWidget;

    if(_settingsWindow != nullptr) delete _settingsWindow;
    if(_echoWindow != nullptr) delete _echoWindow;
    if(_triggerWindow != nullptr) delete _triggerWindow;
    if(_stateReportWindow != nullptr) delete _stateReportWindow;
    if(_valueReportWindow!= nullptr) delete _valueReportWindow;
    if(_fileTransferWindow != nullptr) delete  _fileTransferWindow;
    delete ui;
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
    if(_device->timeoutStatus())
    {
        ui->appStatusLabel->setText("Timeout");
        ui->appStatusLabel->setStyleSheet("color: orange;");
    }
    else if(_device->systemStatus().appRuning)
    {
        ui->appStatusLabel->setText("Runnig");
        ui->appStatusLabel->setStyleSheet("color: green;");
    }
    else
    {
        ui->appStatusLabel->setText("Stopped");
        ui->appStatusLabel->setStyleSheet("color: red;");
    }

    if(_settingsWidget != nullptr)
    {
        _settingsWidget->updateData();
    }
}

void BusDeviceWindow::on_triggerButton_clicked()
{
    if(_triggerWidget == nullptr) _triggerWidget = new TriggerWidget(_device, this);
    if(_triggerWindow == nullptr)
    {
        _triggerWindow= new QMdiSubWindow;
        _triggerWindow->setWidget(_triggerWidget);
        _triggerWindow->setAttribute(Qt::WA_DeleteOnClose);
        _triggerWindow->setWindowTitle("Trigger");
        ui->mdiArea->addSubWindow(_triggerWindow);
        _triggerWindow->show();
    }
    _triggerWindow->setFocus();
}

void BusDeviceWindow::on_pushButton_event_clicked()
{
    if(_eventWidget == nullptr) _eventWidget = new EventWidget(_device, this);
    if(_eventWindow == nullptr)
    {
        _eventWindow= new QMdiSubWindow;
        _eventWindow->setWidget(_eventWidget);
        _eventWindow->setAttribute(Qt::WA_DeleteOnClose);
        _eventWindow->setWindowTitle("Event");
        ui->mdiArea->addSubWindow(_eventWindow);
        _eventWindow->show();
    }
    _eventWindow->setFocus();
}

void BusDeviceWindow::on_settingsButton_clicked()
{
    if(_settingsWidget == nullptr) _settingsWidget = new settingsWidget(_device, this);
    if(_settingsWindow == nullptr)
    {
        _settingsWindow = new QMdiSubWindow;
        _settingsWindow->setWidget(_settingsWidget);
        _settingsWindow->setAttribute(Qt::WA_DeleteOnClose);
        _settingsWindow->setWindowTitle("Settings");
        ui->mdiArea->addSubWindow(_settingsWindow);
        _settingsWindow->updateGeometry();
        _settingsWindow->show();
    }

    _settingsWindow->setFocus();
}

void BusDeviceWindow::on_stateButton_clicked()
{
    if(_stateReportWidget == nullptr) _stateReportWidget = new StateReportWidget(_device, this);
    if(_stateReportWindow == nullptr)
    {
        _stateReportWindow = new QMdiSubWindow;
        _stateReportWindow->setWidget(_stateReportWidget);
        _stateReportWindow->setAttribute(Qt::WA_DeleteOnClose);
        _stateReportWindow->setWindowTitle("State Report");
        ui->mdiArea->addSubWindow(_stateReportWindow);
        _stateReportWindow->updateGeometry();
        _stateReportWindow->show();
    }

    _stateReportWindow->setFocus();
}

void BusDeviceWindow::on_echoButton_clicked()
{    
    if(_echoWidget == nullptr) _echoWidget = new echoTestWidget(_device, this);
    if(_echoWindow == nullptr)
    {
        _echoWindow = new QMdiSubWindow;
        _echoWindow->setWidget(_echoWidget);
        _echoWindow->setAttribute(Qt::WA_DeleteOnClose);
        _echoWindow->setWindowTitle("Echo Test");
        ui->mdiArea->addSubWindow(_echoWindow);
        _echoWindow->show();
    }
    _echoWindow->setFocus();
}

void BusDeviceWindow::on_valueButton_clicked()
{
    if(_valueReportWidget == nullptr) _valueReportWidget = new ValueWidget(_device, this);
    if(_valueReportWindow == nullptr)
    {
        _valueReportWindow = new QMdiSubWindow;
        _valueReportWindow->setWidget(_valueReportWidget);
        _valueReportWindow->setAttribute(Qt::WA_DeleteOnClose);
        _valueReportWindow->setWindowTitle("Value Report");
        ui->mdiArea->addSubWindow(_valueReportWindow);
        _valueReportWindow->show();
    }

    _valueReportWindow->setFocus();
}

void BusDeviceWindow::on_fileButton_clicked()
{
    if(_fileTransferWidget == nullptr) _fileTransferWidget = new fileTransferWidget(_device, this);
    if(_fileTransferWindow == nullptr)
    {
        _fileTransferWindow = new QMdiSubWindow;
        _fileTransferWindow->setWidget(_fileTransferWidget);
        _fileTransferWindow->setAttribute(Qt::WA_DeleteOnClose);
        _fileTransferWindow->setWindowTitle("File Transfer");
        ui->mdiArea->addSubWindow(_fileTransferWindow);
        _fileTransferWindow->updateGeometry();
        _fileTransferWindow->show();
    }

    _fileTransferWidget->setFocus();
}

void BusDeviceWindow::on_rebootButton_clicked()
{
    _device->management().requestSystemRestart();
}

void BusDeviceWindow::on_pushButton_serialBridge_clicked()
{
    if(_tinyLoaderWidget == nullptr) _tinyLoaderWidget = new SerialBridgeWidget(_device,this);

    if(_serialBridgeWindow == nullptr)
    {
        _serialBridgeWindow = new QMdiSubWindow;
        _serialBridgeWindow->setWidget(_tinyLoaderWidget);
      //  _tinyLoaderWindow->setAttribute(Qt::WA_DeleteOnClose);
        ui->mdiArea->addSubWindow(_serialBridgeWindow);
        _serialBridgeWindow->show();
    }

    _serialBridgeWindow->setFocus();
}
