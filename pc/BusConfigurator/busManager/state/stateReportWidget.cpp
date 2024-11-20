#include "stateReportWidget.h"
#include "ui_stateReportWidget.h"

StateReportWidget::StateReportWidget(RoomBusDevice *busDevice, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StateReportWidget),
    _stateReportProtocol(busDevice)
{
    ui->setupUi(this);

    _busDevice = busDevice;
    connect(&_stateReportProtocol,&StateSystemProtocol::signalListChange, this, &StateReportWidget::on_signalListChange);
    connect(&_stateReportProtocol,&StateSystemProtocol::slotListChange, this, &StateReportWidget::on_slotListChange);

    _stateReportProtocol.requestSlotInformation();
    _stateReportProtocol.requestSignalInformation();
}

StateReportWidget::~StateReportWidget()
{
    delete ui;
}

void StateReportWidget::on_signalListChange()
{
    ui->listWidget_signal->clear();

    for( auto &signal: _stateReportProtocol.stateReportSignls()){
        QListWidgetItem *item = new QListWidgetItem(ui->listWidget_signal);
        StateReportSignalLineWidget *lineWidget = new StateReportSignalLineWidget(&_stateReportProtocol, signal, ui->listWidget_signal);
        item->setSizeHint(lineWidget->sizeHint());
        ui->listWidget_signal->setItemWidget(item, lineWidget);
    }
}

void StateReportWidget::on_slotListChange()
{
    ui->listWidget_slot->clear();

    for( auto &slot: _stateReportProtocol.stateReportSlots()){
        QListWidgetItem *item = new QListWidgetItem(ui->listWidget_slot);
        StateReportSlotLineWidget *lineWidget = new StateReportSlotLineWidget(slot, ui->listWidget_slot);
        item->setSizeHint(lineWidget->sizeHint());
        ui->listWidget_slot->setItemWidget(item, lineWidget);
    }
}

void StateReportWidget::on_pushButton_clear_clicked()
{
    ui->listWidget_signal->clear();
    ui->listWidget_slot->clear();

    _stateReportProtocol.reset();
}

void StateReportWidget::on_pushButton_refresh_clicked()
{
    _stateReportProtocol.requestSlotInformation();
    _stateReportProtocol.requestSignalInformation();
}

