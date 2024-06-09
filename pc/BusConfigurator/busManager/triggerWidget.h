#ifndef TRIGGERWIDGET_H
#define TRIGGERWIDGET_H

#include <QDialog>
#include "busDevice.h"
#include "triggerLine.h"
#include "protocol/triggerProtocol.h"

namespace Ui {
class triggerWidget;
}

class triggerWidget : public QDialog
{
    Q_OBJECT

public:
    explicit triggerWidget(busDevice *busDevice, QWidget *parent = nullptr);
    ~triggerWidget();

    void updateTriggerList(QMap<uint8_t, triggerProtocol::triggerSlot> triggerList);

private slots:

    void on_trigger(uint8_t triggerChannel);
    void on_nameChange(void);

    void on_triggerListButton_clicked();

    void on_clearListButton_clicked();

private:
    Ui::triggerWidget *ui;

    QList<triggerLine*> _triggerLines;

    busDevice *_busDevice;

    triggerProtocol _triggerProtocol;
};

#endif // TRIGGERWIDGET_H
