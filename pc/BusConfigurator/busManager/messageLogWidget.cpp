#include "messageLogWidget.h"
#include "ui_messageLogWidget.h"


messageLogWidget::messageLogWidget( QWidget *parent) :
    QWidget(parent),
    ui(new Ui::messageLogWidget)
{
    ui->setupUi(this);
    _filter = 0xFF;
}


messageLogWidget::~messageLogWidget()
{
    delete ui;
    if(_logProtocol != nullptr) delete _logProtocol;
}

void messageLogWidget::assignDevice(RoomBusDevice *busDevice)
{
    if(_logProtocol == nullptr) _logProtocol = new MessageLogProtocol(busDevice);
    connect(_logProtocol,&MessageLogProtocol::newMessage, this, &messageLogWidget::on_newMessage);
}

void messageLogWidget::on_newMessage(LogMessage msg)
{
    addMessage(msg);
    ui->textEdit->setHtml(_log);
}

void messageLogWidget::addMessage(LogMessage msg)
{
    QString temp;

    if(msg.messageType == LogMessage::sysMessage && (_filter & 0x01))  temp.append("<font color=\"Black\">");
    else if(msg.messageType == LogMessage::sysWarning && (_filter & 0x02))  temp.append("<font color=\"Orange\">");
    else if(msg.messageType == LogMessage::sysError && (_filter & 0x04))  temp.append("<font color=\"Red\">");
    else if(msg.messageType == LogMessage::appMessage && (_filter & 0x10))  temp.append("<font color=\"Black\">");
    else if(msg.messageType == LogMessage::appWarning && (_filter & 0x20))  temp.append("<font color=\"Orange\">");
    else if(msg.messageType == LogMessage::appError && (_filter & 0x40))  temp.append("<font color=\"Red\">");
    else return;

    temp.append(msg.receiveTime.toString("hh:mm:ss"));
    temp.append(" : ");
    temp.append(QString::number(msg.tickTime).rightJustified(10,'0')+"ms - ");
    temp.append(msg.massage);
    temp.append("</font><br>");

    _log.prepend(temp);
}

void messageLogWidget::updateLog(void)
{
    _log.clear();

    foreach( LogMessage msg , _logProtocol->messages() )
    {
        addMessage(msg);
    }

    ui->textEdit->setHtml(_log);
}

void messageLogWidget::on_clearButton_clicked()
{
    _logProtocol->clearLog();
    updateLog();
}

void messageLogWidget::on_sysMsgBox_stateChanged(int arg1)
{
    if(arg1) _filter |= 0x01;
    else _filter &= ~0x01;

    updateLog();
}

void messageLogWidget::on_sysWarnBox_stateChanged(int arg1)
{
    if(arg1) _filter |= 0x02;
    else _filter &= ~0x02;

    updateLog();
}

void messageLogWidget::on_sysErrBox_stateChanged(int arg1)
{
    if(arg1) _filter |= 0x04;
    else _filter &= ~0x04;

    updateLog();
}

void messageLogWidget::on_appMsgBox_stateChanged(int arg1)
{
    if(arg1) _filter |= 0x10;
    else _filter &= ~0x10;

    updateLog();
}

void messageLogWidget::on_appWarnBox_stateChanged(int arg1)
{
    if(arg1) _filter |= 0x20;
    else _filter &= ~0x20;

    updateLog();
}

void messageLogWidget::on_appErrBox_stateChanged(int arg1)
{
    if(arg1) _filter |= 0x40;
    else _filter &= ~0x40;

    updateLog();
}


