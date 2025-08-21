#ifndef VALUE_SLOT_LINE_WIDGET_H
#define VALUE_SLOT_LINE_WIDGET_H

#include <QWidget>
#include "protocol/valueSystemProtocol.h"

namespace Ui {
class ValueSlotLineWidget;
}

class ValueSlotLineWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ValueSlotLineWidget(ValueSystemProtocol::ValueSlot* valueSlot, QWidget *parent = nullptr);
    ~ValueSlotLineWidget();

signals:

private:
    Ui::ValueSlotLineWidget *ui;

    ValueSystemProtocol::ValueSlot* _valueSlot;
};

#endif // VALUE_SLOT_LINE_WIDGET_H
