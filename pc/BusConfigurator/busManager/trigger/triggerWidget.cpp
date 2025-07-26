#include "triggerWidget.h"
#include "ui_triggerWidget.h"

TriggerWidget::TriggerWidget(RoomBusDevice *busDevice, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TriggerWidget),
    _triggerProtocol(busDevice)
{
    ui->setupUi(this);

    _busDevice = busDevice;
    connect(&_triggerProtocol,&TriggerSystemProtocol::triggerSignalListChange, this, &TriggerWidget::on_signalListChange);
    connect(&_triggerProtocol,&TriggerSystemProtocol::triggerSlotListChange, this, &TriggerWidget::on_slotListChange);

    _triggerProtocol.requestSlotInformation();
    _triggerProtocol.requestSignalInformation();
}

TriggerWidget::~TriggerWidget()
{
    delete ui;
}

void TriggerWidget::on_trigger(uint16_t triggerChannel)
{
    _triggerProtocol.sendTrigger(triggerChannel);
}

void TriggerWidget::on_signalListChange()
{
    ui->listWidget_signal->clear();

    for( auto &signal: _triggerProtocol.triggerSignls()){
        QListWidgetItem *item = new QListWidgetItem(ui->listWidget_signal);
        TriggerSignalLineWidget *lineWidget = new TriggerSignalLineWidget(signal, ui->listWidget_signal);
        item->setSizeHint(lineWidget->sizeHint());
        ui->listWidget_signal->setItemWidget(item, lineWidget);
    }
}

void TriggerWidget::on_slotListChange()
{
    ui->listWidget_slot->clear();

    for( auto &slot: _triggerProtocol.triggerSlots()){
        QListWidgetItem *item = new QListWidgetItem(ui->listWidget_slot);
        TriggerSlotLineWidget *lineWidget = new TriggerSlotLineWidget(slot, ui->listWidget_slot);
        item->setSizeHint(lineWidget->sizeHint());
        ui->listWidget_slot->setItemWidget(item, lineWidget);
    }
}

void TriggerWidget::on_pushButton_clear_clicked()
{
    ui->listWidget_signal->clear();
    ui->listWidget_slot->clear();

    _triggerProtocol.clearInformation();
}

void TriggerWidget::on_pushButton_refresh_clicked()
{
    _triggerProtocol.requestSlotInformation();
    _triggerProtocol.requestSignalInformation();
}

