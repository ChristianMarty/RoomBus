#ifndef VALUEREPORTWIDGET_H
#define VALUEREPORTWIDGET_H

#include <QDialog>
#include "busDevice.h"
#include "protocol/valueReportProtocol.h"
#include "valueReportLine.h"

namespace Ui {
class valueReportWidget;
}

class valueReportWidget : public QDialog
{
    Q_OBJECT

public:
    explicit valueReportWidget(busDevice *busDevice, QWidget *parent = nullptr);
    ~valueReportWidget();

private slots:
    void on_newValue(uint16_t channel, float value);
    void on_metaDataChange(uint16_t channel, valueReportProtocol::valueItem_t item);

    void on_pushButton_clicked();

private:
    Ui::valueReportWidget *ui;

    busDevice *_busDevice;

    valueReportProtocol _valueReportProtocol;

    QList<valueReportLine*> _valueReportLines;
};

#endif // VALUEREPORTWIDGET_H
