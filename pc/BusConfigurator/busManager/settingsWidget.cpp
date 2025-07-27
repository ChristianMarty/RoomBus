#include "settingsWidget.h"
#include "ui_settingsWidget.h"

#include <QFileDialog>

settingsWidget::settingsWidget(RoomBusDevice *busDevice, QWidget *parent) :
  QWidget(parent),
  ui(new Ui::settingsWidget)
{
    ui->setupUi(this);

    _busDevice = busDevice;
    connect(&busDevice->management(), &DeviceManagementProtocol::bootloadStatusUpdate, this, &settingsWidget::on_bootloadStatusUpdate);
    connect(&busDevice->management(), &DeviceManagementProtocol::statusUpdate, this, &settingsWidget::on_statusUpdate);

    connect(&_bootloadFileWatcher,&QFileSystemWatcher::fileChanged,this, &settingsWidget::on_bootloadFileChanged);

    updateData();
}

settingsWidget::~settingsWidget()
{
    delete ui;
}

void settingsWidget::updateData(void)
{
    ui->nameEdit->setText(_busDevice->deviceName());
    ui->addressBox->setValue(_busDevice->deviceAddress());

    ui->ledOnBox->setChecked(_busDevice->systemStatus().ledOnOff);
    ui->appRunBox->setChecked(_busDevice->systemStatus().appRuning);
    ui->autostartAppBox->setChecked(_busDevice->systemStatus().appRunOnStartup);

    ui->systemInfoIntervalBox->setValue(_busDevice->management().systemInfoInterval()*10);
    ui->heartbeatIntervalBox->setValue(_busDevice->management().heartbeatInterval());
}

void settingsWidget::on_setNameButton_clicked()
{
    _busDevice->management().enterRootMode();

    _busDevice->management().writeDeviceName(ui->nameEdit->text());

    _busDevice->management().exitRootMode();

}

void settingsWidget::on_setAddressButton_clicked()
{
    _busDevice->management().enterRootMode();

    _busDevice->management().writeAddress(static_cast<uint8_t>(ui->addressBox->value()));

    _busDevice->management().exitRootMode();
}

void settingsWidget::on_ledOnBox_clicked(bool checked)
{
    DeviceManagementProtocol::SystemControl temp;
    temp.reg = 0;
    temp.bit.ledOnOff = true;

    if(checked)_busDevice->management().writeSetControl(temp);
    else _busDevice->management().writeClearControl(temp);
}

void settingsWidget::on_appRunBox_clicked(bool checked)
{
    DeviceManagementProtocol::SystemControl temp;
    temp.reg = 0;
    temp.bit.appRun = true;

    if(checked)_busDevice->management().writeSetControl(temp);
    else _busDevice->management().writeClearControl(temp);
}

void settingsWidget::on_autostartAppBox_clicked(bool checked)
{
    DeviceManagementProtocol::SystemControl temp;
    temp.reg = 0;
    temp.bit.appRunOnStartup = true;

    if(checked)_busDevice->management().writeSetControl(temp);
    else _busDevice->management().writeClearControl(temp);
}

void settingsWidget::on_setIntervalButton_clicked()
{
    _busDevice->management().writeHeartbeatInterval(static_cast<uint16_t>(ui->heartbeatIntervalBox->value()),static_cast<uint16_t>(ui->systemInfoIntervalBox->value()/10));
}

void settingsWidget::on_firmwarePathButton_clicked()
{
    ui->firmwarePathEdit->setText(QFileDialog::getOpenFileName(this,tr("Open HEX File"), "", tr("Hex Files (*.hex)|All files (*.*)")));

    _bootloadFileWatcher.removePaths(_bootloadFileWatcher.files());
    _bootloadFileWatcher.addPath(ui->firmwarePathEdit->text());
}

void settingsWidget::on_firmwareUploadButton_clicked()
{
    _busDevice->management().startFirmwareUpload(ui->firmwarePathEdit->text());
}

void settingsWidget::on_bootloadStatusUpdate(uint8_t progress, bool error, QString message)
{
   ui->progressBar->setValue(progress);

   QString temp;
   if(error)  temp.append("<font color=\"Red\">");
   else temp.append("<font color=\"Black\">");
   temp.append(message);
   temp.append("</font>");
   ui->logBox->append(temp);
}

void settingsWidget::on_statusUpdate(void)
{

}

void settingsWidget::on_bootloadFileChanged(const QString &path)
{
    if(ui->autoUploadBox->isChecked())
    {
        _busDevice->management().startFirmwareUpload(ui->firmwarePathEdit->text());
    }
}

void settingsWidget::on_clearLog_button_clicked()
{
     ui->logBox->clear();
}

