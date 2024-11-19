#include "valueWidget.h"
#include "ui_valueWidget.h"

ValueWidget::ValueWidget(busDevice *busDevice, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ValueWidget),
    _valueProtocol(busDevice)
{
    ui->setupUi(this);

    _busDevice = busDevice;
    connect(&_valueProtocol, &ValueSystemProtocol::signalListChange, this, &ValueWidget::on_signalListChange);
    connect(&_valueProtocol, &ValueSystemProtocol::slotListChange, this, &ValueWidget::on_slotListChange);

    _valueProtocol.requestSlotInformation();
    _valueProtocol.requestSignalInformation();
}

ValueWidget::~ValueWidget()
{
    delete ui;
}

void ValueWidget::on_trigger(uint16_t triggerChannel)
{

}

void ValueWidget::on_signalListChange()
{
    ui->listWidget_signal->clear();

    for( auto &signal: _valueProtocol.valueSignls()){
        QListWidgetItem *item = new QListWidgetItem(ui->listWidget_signal);
        ValueSignalLineWidget *lineWidget = new ValueSignalLineWidget(&_valueProtocol, signal, ui->listWidget_signal);
        item->setSizeHint(lineWidget->sizeHint());
        ui->listWidget_signal->setItemWidget(item, lineWidget);
    }
}

void ValueWidget::on_slotListChange()
{
    ui->listWidget_slot->clear();

    for( auto &slot: _valueProtocol.valueSlots()){
        QListWidgetItem *item = new QListWidgetItem(ui->listWidget_slot);
        ValueSlotLineWidget *lineWidget = new ValueSlotLineWidget(slot, ui->listWidget_slot);
        item->setSizeHint(lineWidget->sizeHint());
        ui->listWidget_slot->setItemWidget(item, lineWidget);
    }
}

void ValueWidget::on_pushButton_clear_clicked()
{
    ui->listWidget_signal->clear();
    ui->listWidget_slot->clear();

    _valueProtocol.reset();
}

void ValueWidget::on_pushButton_refresh_clicked()
{
    _valueProtocol.requestSlotInformation();
    _valueProtocol.requestSignalInformation();
}

