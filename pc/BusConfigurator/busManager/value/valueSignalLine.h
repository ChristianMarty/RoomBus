#ifndef VALUE_SIGNAL_LINE_WIDGET_H
#define VALUE_SIGNAL_LINE_WIDGET_H

#include <QWidget>
#include "protocol/valueSystemProtocol.h"

namespace Ui {
class ValueSignalLineWidget;
}

class ValueSignalLineWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ValueSignalLineWidget(class::ValueSystemProtocol* protocol, ValueSystemProtocol::ValueSignal* valueSignal, QWidget *parent = nullptr);
    ~ValueSignalLineWidget();

signals:

private slots:
    void on_signalValueChnage(uint16_t channel);

    void on_pushButton_write_clicked();

private:
    Ui::ValueSignalLineWidget *ui;

    ValueSystemProtocol::ValueSignal* _valueSignal;
    ValueSystemProtocol* _protocol;
};

#endif // VALUE_SIGNAL_LINE_WIDGET_H
