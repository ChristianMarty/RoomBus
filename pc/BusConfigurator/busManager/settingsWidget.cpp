#include "settingsWidget.h"
#include "ui_settingsWidget.h"

#include <QFileDialog>

SettingsWidget::SettingsWidget(RoomBusDevice *busDevice, QWidget *parent) :
  QWidget(parent),
  ui(new Ui::settingsWidget)
{
    ui->setupUi(this);

    _busDevice = busDevice;
    connect(&busDevice->management(), &DeviceManagementProtocol::bootloadStatusUpdate, this, &SettingsWidget::on_bootloadStatusUpdate);
    connect(&busDevice->management(), &DeviceManagementProtocol::statusUpdate, this, &SettingsWidget::on_statusUpdate);

    connect(&_bootloadFileWatcher, &QFileSystemWatcher::fileChanged, this, &SettingsWidget::on_bootloadFileChanged);

    updateData();
}

SettingsWidget::~SettingsWidget()
{
    delete ui;
}

void SettingsWidget::updateData(void)
{
    if(!ui->nameEdit->hasFocus()){
        ui->nameEdit->setText(_busDevice->deviceName());
    }

    if(!ui->addressBox->hasFocus()){
        ui->addressBox->setValue(_busDevice->deviceAddress());
    }

    if(!ui->systemInfoIntervalBox->hasFocus() && !ui->heartbeatIntervalBox->hasFocus()){
        ui->systemInfoIntervalBox->setValue(_busDevice->management().systemInfoInterval()*10);
        ui->heartbeatIntervalBox->setValue(_busDevice->management().heartbeatInterval());
    }

    ui->ledOnBox->setChecked(_busDevice->systemStatus().userLedEnabled);
    ui->appRunBox->setChecked(_busDevice->systemStatus().applicationRuning);
    ui->autostartAppBox->setChecked(_busDevice->systemStatus().applicationRunOnStartup);
    ui->checkBox_messageLogEnabled->setChecked(_busDevice->systemStatus().messageLogEnabled);
    ui->checkBox_administrationMode->setChecked(_busDevice->systemStatus().administrationMode);
}

void SettingsWidget::on_setNameButton_clicked()
{
    _busDevice->management().writeDeviceName(ui->nameEdit->text());
}

void SettingsWidget::on_setAddressButton_clicked()
{
    _busDevice->management().writeAddress(static_cast<uint8_t>(ui->addressBox->value()));
}

void SettingsWidget::on_ledOnBox_clicked(bool checked)
{
    DeviceManagementProtocol::SystemControl temp;
    temp.reg = 0;
    temp.bit.userLedEnabled = true;

    if(checked)_busDevice->management().writeSetControl(temp);
    else _busDevice->management().writeClearControl(temp);
}

void SettingsWidget::on_checkBox_messageLogEnabled_clicked(bool checked)
{
    DeviceManagementProtocol::SystemControl temp;
    temp.reg = 0;
    temp.bit.messageLogEnabled = true;

    if(checked)_busDevice->management().writeSetControl(temp);
    else _busDevice->management().writeClearControl(temp);
}

void SettingsWidget::on_appRunBox_clicked(bool checked)
{
    DeviceManagementProtocol::SystemControl temp;
    temp.reg = 0;
    temp.bit.applicationRun = true;

    if(checked)_busDevice->management().writeSetControl(temp);
    else _busDevice->management().writeClearControl(temp);
}

void SettingsWidget::on_autostartAppBox_clicked(bool checked)
{
    DeviceManagementProtocol::SystemControl temp;
    temp.reg = 0;
    temp.bit.applicationRunOnStartup = true;

    if(checked)_busDevice->management().writeSetControl(temp);
    else _busDevice->management().writeClearControl(temp);
}

void SettingsWidget::on_setIntervalButton_clicked()
{
    _busDevice->management().writeHeartbeatInterval(static_cast<uint16_t>(ui->heartbeatIntervalBox->value()),static_cast<uint16_t>(ui->systemInfoIntervalBox->value()/10));
}

void SettingsWidget::on_firmwarePathButton_clicked()
{
    ui->firmwarePathEdit->setText(QFileDialog::getOpenFileName(this,tr("Open HEX File"), "", tr("Hex Files (*.hex)|All files (*.*)")));

    _bootloadFileWatcher.removePaths(_bootloadFileWatcher.files());
    _bootloadFileWatcher.addPath(ui->firmwarePathEdit->text());
}

void SettingsWidget::on_firmwareUploadButton_clicked()
{
    _busDevice->management().startFirmwareUpload(ui->firmwarePathEdit->text());
}

void SettingsWidget::on_bootloadStatusUpdate(uint8_t progress, bool error, QString message)
{
   ui->progressBar->setValue(progress);

   QString temp;
   if(error)  temp.append("<font color=\"Red\">");
   else temp.append("<font color=\"Black\">");
   temp.append(message);
   temp.append("</font>");
   ui->logBox->append(temp);
}

void SettingsWidget::on_statusUpdate(void)
{
    updateData();
}

void SettingsWidget::on_bootloadFileChanged(const QString &path)
{
    if(ui->autoUploadBox->isChecked()){
        _busDevice->management().startFirmwareUpload(ui->firmwarePathEdit->text());
    }
}

void SettingsWidget::on_clearLog_button_clicked()
{
     ui->logBox->clear();
}

void SettingsWidget::on_checkBox_administrationMode_clicked(bool checked)
{
    if(checked) _busDevice->management().enterAdministrationMode(ui->lineEdit_administrationKey->text());
    else _busDevice->management().exitAdministrationMode();
}

void SettingsWidget::on_pushButton_setAdministrationKey_clicked()
{
    _busDevice->management().writeAdministrationModeKey(ui->lineEdit_administrationKey->text());
}
