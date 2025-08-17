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
    ui->textEdit_data->clear();
    ui->textEdit_data->append("                   00 01 02 03 04 05 06 07  08 09 0A 0B 0C 0D 0E 0F");
    ui->textEdit_data->append("                   ................................................");

    uint16_t eepromSize = _busDevice->management().eeprom().eepromSize();

    for(uint16_t i = 0; i<eepromSize; i+=16){

        QString line = QString::number(i).rightJustified(5,' ')+" / ";
        line += QString::number(i,16).toUpper().rightJustified(4,'0').prepend("0x");
        line += "  : ";
        for(uint16_t j = 0; j<16; j++){

            uint16_t offset = i+j;
            Eeprom::EepromByte byte = _busDevice->management().eeprom().at(offset);

            if(byte.read){
                line +=" "+QString::number(byte.byte,16).toUpper().rightJustified(2,'0');
            }else{
                line +=" --";
            }
            if(j == 7)line +=" ";
        }

        ui->textEdit_data->append(line);
    }
}

