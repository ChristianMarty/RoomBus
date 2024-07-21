#ifndef VALUEREPORTLINE_H
#define VALUEREPORTLINE_H

#include <QWidget>
#include <protocol/valueReportProtocol.h>

namespace Ui {
class valueReportLine;
}

class valueReportLine : public QWidget
{
    Q_OBJECT

public:
    explicit valueReportLine(valueReportProtocol::valueItem_t item, valueReportProtocol *valueReportProtocol, QWidget *parent = nullptr);
    ~valueReportLine();

    void updateValue(float value);

    uint16_t getChannel(void);

private slots:
    void on_horizontalSlider_sliderMoved(int position);

    void on_horizontalSlider_actionTriggered(int action);

private:
    Ui::valueReportLine *ui;
    valueReportProtocol *_valueReportProtocol;

    valueReportProtocol::valueItem_t _item;
    float _value;

};

#endif // VALUEREPORTLINE_H
