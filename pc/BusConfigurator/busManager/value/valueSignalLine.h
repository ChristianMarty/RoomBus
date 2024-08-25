#ifndef VALUE_SIGNAL_LINE_WIDGET_H
#define VALUE_SIGNAL_LINE_WIDGET_H

#include <QWidget>
#include "protocol/valueProtocol.h"

namespace Ui {
class ValueSignalLineWidget;
}

class ValueSignalLineWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ValueSignalLineWidget(class::ValueProtocol* protocol, ValueProtocol::ValueSignal* valueSignal, QWidget *parent = nullptr);
    ~ValueSignalLineWidget();

signals:

private slots:
    void on_signalValueChnage(uint16_t channel);

    void on_pushButton_write_clicked();

private:
    Ui::ValueSignalLineWidget *ui;

    ValueProtocol::ValueSignal* _valueSignal;
    ValueProtocol* _protocol;
};

#endif // VALUE_SIGNAL_LINE_WIDGET_H
