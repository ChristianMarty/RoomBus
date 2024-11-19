#include "eventWidget.h"
#include "ui_eventWidget.h"

EventWidget::EventWidget(busDevice *busDevice, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EventWidget),
    _eventProtocol(busDevice)
{
    ui->setupUi(this);

    _busDevice = busDevice;
    connect(&_eventProtocol,&EventSystemProtocol::eventSignalListChange, this, &EventWidget::on_signalListChange);
    connect(&_eventProtocol,&EventSystemProtocol::eventSlotListChange, this, &EventWidget::on_slotListChange);

    _eventProtocol.requestSlotInformation();
    _eventProtocol.requestSignalInformation();
}

EventWidget::~EventWidget()
{
    delete ui;
}

void EventWidget::on_eventActive(uint8_t eventChannel, bool active)
{
    _eventProtocol.setActiveState(eventChannel, active);
}

void EventWidget::on_signalListChange()
{
    ui->listWidget_signal->clear();

    for( auto &signal: _eventProtocol.eventSignls()){
        QListWidgetItem *item = new QListWidgetItem(ui->listWidget_signal);
        EventSignalLineWidget *lineWidget = new EventSignalLineWidget(signal, ui->listWidget_signal);
        item->setSizeHint(lineWidget->sizeHint());
        ui->listWidget_signal->setItemWidget(item, lineWidget);
    }
}

void EventWidget::on_slotListChange()
{
    ui->listWidget_slot->clear();

    for( auto &slot: _eventProtocol.eventSlots()){
        QListWidgetItem *item = new QListWidgetItem(ui->listWidget_slot);
        EventSlotLineWidget *lineWidget = new EventSlotLineWidget(slot, ui->listWidget_slot);
        item->setSizeHint(lineWidget->sizeHint());
        ui->listWidget_slot->setItemWidget(item, lineWidget);
    }
}

void EventWidget::on_pushButton_clear_clicked()
{
    ui->listWidget_signal->clear();
    ui->listWidget_slot->clear();

    _eventProtocol.reset();
}

void EventWidget::on_pushButton_refresh_clicked()
{
    _eventProtocol.requestSlotInformation();
    _eventProtocol.requestSignalInformation();
}




