#include "stateReportWidget.h"
#include "ui_stateReportWidget.h"

stateReportWidget::stateReportWidget(busDevice *busDevice, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::stateReportWidget), _stateReportProtocol(busDevice)
{
    ui->setupUi(this);
    _busDevice = busDevice;

    connect(&_stateReportProtocol,&stateReportProtocol::stateReportListChange, this, &stateReportWidget::on_nameChange);
    connect(&_stateReportProtocol,&stateReportProtocol::stateChange, this, &stateReportWidget::on_stateChange);

    _stateReportProtocol.requestStateReportChannelNames();
}

stateReportWidget::~stateReportWidget()
{
    delete ui;
}

void stateReportWidget::on_stateChange(void)
{
    QMapIterator<uint8_t, stateReportProtocol::state_t> i(_stateReportProtocol.stateReports());

    while (i.hasNext())
    {
        i.next();

        for(uint8_t j = 0; j<_stateReportLines.size();j++)
        {
            if(_stateReportLines.at(j)->channel() == i.key())
            {
                if(i.value() ==  stateReportProtocol::on)_stateReportLines.at(j)->setState("On");
                else if(i.value() ==  stateReportProtocol::off)_stateReportLines.at(j)->setState("Off");
                else if(i.value() ==  stateReportProtocol::transitioning)_stateReportLines.at(j)->setState("Transitioning");
                else if(i.value() ==  stateReportProtocol::unknown)_stateReportLines.at(j)->setState("Unknown");
                else if(i.value() ==  stateReportProtocol::undefined)_stateReportLines.at(j)->setState("Undefined");
                break;
            }

        }
    }
}

void stateReportWidget::updateStateChannelList(QMap<uint8_t, QString> stateChannelList)
{
    QMapIterator<uint8_t, QString> i(stateChannelList);
    bool found;

    while (i.hasNext())
    {
        i.next();
        found = false;

        for(uint8_t j = 0; j<_stateReportLines.size();j++)
        {
            if(_stateReportLines.at(j)->channel() == i.key())
            {
                _stateReportLines.at(j)->setName(i.value());
                found = true;
                break;
            }
        }

        if(!found)
        {
            stateReportLine *temp = new stateReportLine(i.value(),i.key(),this);
            ui->stateReportLayout->insertWidget(ui->stateReportLayout->count()-1,temp);
            _stateReportLines.append(temp);
        }
    }
}

void stateReportWidget::on_nameChange(void)
{
    updateStateChannelList(_stateReportProtocol.stateReportNames());
}

void stateReportWidget::on_updateListButton_clicked()
{
    _stateReportProtocol.requestStateReportChannelNames();
}

void stateReportWidget::on_readButton_clicked()
{
    _stateReportProtocol.requestStateAll();
}

void stateReportWidget::on_clearButton_clicked()
{
    stateReportLine * line;
    foreach(line, _stateReportLines)
    {
        delete line;
    }
    _stateReportLines.clear();

    _stateReportProtocol.reset();
}
