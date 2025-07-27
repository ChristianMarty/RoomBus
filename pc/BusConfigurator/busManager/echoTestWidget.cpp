#include "echoTestWidget.h"
#include "ui_echoTestWidget.h"

#include <QDateTime>

echoTestWidget::echoTestWidget(RoomBusDevice *busDevice, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::echoWindow)
{
    ui->setupUi(this);
    _busDevice = busDevice;

    connect(&_busDevice->management(), &DeviceManagementProtocol::echoReceive,this,&echoTestWidget::on_echoReceive);
    connect(&_autoSendTimer,&QTimer::timeout,this,&echoTestWidget::on_sendTimer);
    connect(&_timeoutTimer,&QTimer::timeout,this,&echoTestWidget::on_timeout);

    _autoSendTimer.setInterval(100);
}

echoTestWidget::~echoTestWidget()
{
    delete ui;
}

void echoTestWidget::closeEvent(QCloseEvent *event)
{
    _autoSendTimer.stop();
    ui->autoEchoBox->setChecked(false);
    event->accept();
}

void echoTestWidget::on_echoReceive(QByteArray rxData)
{
    _timeoutTimer.stop();
    _waitForRx = false;

    _receiveCounter++;
    ui->receiveCounterLabel->setNum(int(_receiveCounter));

    uint32_t ping = QDateTime::currentMSecsSinceEpoch()-_sendTimestamp;
    ui->pingLabel->setNum(int(ping));

    if(rxData.compare(_lastSend))
    {
        ui->passFailLabel->setText("Fail");
        ui->passFailLabel->setStyleSheet("background-color: red;");
        _failCounter++;
    }
    else
    {
        ui->passFailLabel->setText("Pass");
        ui->passFailLabel->setStyleSheet("background-color: green;");
        _passCounter++;
    }

    ui->lastReceivedLabel->setText(rxData.remove(0,1));
    ui->passCounterLabel->setNum(int(_passCounter));
    ui->failCounterLabel->setNum(int(_failCounter));

}

void echoTestWidget::on_sendButton_clicked()
{
    _timeoutTimer.stop();
    _sendCounter++;
    ui->sentCounterLabel->setNum(int(_sendCounter));

    _lastSend.clear();
    _lastSend.append(_sendIndex);
    _lastSend.append(ui->sendDataEdit->text().toLocal8Bit());
    ui->lastSendLabel->setText(ui->sendDataEdit->text());


    _busDevice->management().sendEcho(_lastSend);

    _sendIndex++;
    _sendTimestamp = QDateTime::currentMSecsSinceEpoch();

    _waitForRx = true;

    _timeoutTimer.setSingleShot(true);
    _timeoutTimer.start(ui->timeoutBox->value()*10);


}

void echoTestWidget::on_autoEchoBox_stateChanged(int arg1)
{
    if(arg1 != false) _autoSendTimer.start();
    else _autoSendTimer.stop();
}

void echoTestWidget::on_sendTimer()
{
    if(!_waitForRx) on_sendButton_clicked();
}

void echoTestWidget::on_timeout(void)
{
    _lostCounter++;
    ui->lostCounterLabel->setNum(int(_lostCounter));
    ui->pingLabel->setText(" - ");

    if(ui->autoEchoBox->isChecked()) on_sendButton_clicked();
}

void echoTestWidget::on_spinBox_valueChanged(int arg1)
{
    _autoSendTimer.setInterval(arg1*10);
}

void echoTestWidget::on_clearButton_clicked()
{
    _lostCounter = 0;
    _sendCounter = 0;
    _receiveCounter = 0;
    _failCounter = 0;
    _passCounter = 0;
    ui->sentCounterLabel->setNum(int(_sendCounter));
    ui->receiveCounterLabel->setNum(int(_receiveCounter));
    ui->passCounterLabel->setNum(int(_passCounter));
    ui->failCounterLabel->setNum(int(_failCounter));
    ui->lostCounterLabel->setNum(int(_lostCounter));
}

void echoTestWidget::on_timeoutBox_valueChanged(int arg1)
{

}
