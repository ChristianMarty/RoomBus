#ifndef EEPROMWIDGET_H
#define EEPROMWIDGET_H

#include <QWidget>
#include "busDevice.h"

namespace Ui {
class EepromWidget;
}

class EepromWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EepromWidget(RoomBusDevice *busDevice, QWidget *parent = nullptr);
    ~EepromWidget();

private slots:
    void on_pushButton_read_clicked();
    void on_pushButton_clear_clicked();

    void on_epromDataChanged(void);

private:
    Ui::EepromWidget *ui;
    RoomBusDevice *_busDevice;

    void _updateData(void);
};

#endif // EEPROMWIDGET_H
