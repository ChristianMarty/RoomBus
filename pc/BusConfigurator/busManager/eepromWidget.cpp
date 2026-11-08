#include "eepromWidget.h"
#include "ui_eepromWidget.h"

EepromWidget::EepromWidget(RoomBusDevice *busDevice, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::EepromWidget)
    ,_busDevice{busDevice}
{
    ui->setupUi(this);

    connect(&_busDevice->management(), &DeviceManagementProtocol::eepromDataChanged, this, &EepromWidget::on_epromDataChanged);

    _updateData();
}

EepromWidget::~EepromWidget()
{
    delete ui;
}

void EepromWidget::on_pushButton_read_clicked()
{
    _busDevice->management().eeprom().readAll();
}

void EepromWidget::on_pushButton_clear_clicked()
{
    _busDevice->management().eeprom().clear();
}

void EepromWidget::on_epromDataChanged()
{
    _updateData();
}

void EepromWidget::_updateData()
{
    ui->textEdit_data->updateMemory(_busDevice->management().eeprom().data());
}
