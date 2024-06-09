#include "valueReportWidget.h"
#include "ui_valueReportWidget.h"


valueReportWidget::valueReportWidget(busDevice *busDevice, QWidget *parent):
    QDialog(parent),
    ui(new Ui::valueReportWidget), _valueReportProtocol(busDevice)
{
    ui->setupUi(this);
    _busDevice = busDevice;

   connect(&_valueReportProtocol,&valueReportProtocol::newValue, this, &valueReportWidget::on_newValue);
   connect(&_valueReportProtocol,&valueReportProtocol::metaDataChange, this, &valueReportWidget::on_metaDataChange);
}

valueReportWidget::~valueReportWidget()
{
    delete ui;
}

void valueReportWidget::on_metaDataChange(uint16_t channel, valueReportProtocol::valueItem_t item)
{
    for (int i = 0; i<_valueReportLines.size();i++)
    {
        if(channel == _valueReportLines[i]->getChannel())
        {
            _valueReportLines[i]->updateValue(item.value);
            return;
        }
    }

    valueReportLine *temp = new valueReportLine(item,&_valueReportProtocol,this);
    ui->valueReportLayout->insertWidget(ui->valueReportLayout->count()-1,temp);
    _valueReportLines.append(temp);
    temp->updateValue(item.value);
}

void valueReportWidget::on_newValue(uint16_t channel, float value)
{
    for (int i = 0; i<_valueReportLines.size();i++)
    {
        if(channel == _valueReportLines[i]->getChannel())
        {
            _valueReportLines[i]->updateValue(value);
            return;
        }
    }
}

void valueReportWidget::on_pushButton_clicked()
{
    _valueReportProtocol.requestMetaData();
}
