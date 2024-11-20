#ifndef QUALITYOFSERVICEWINDOW_H
#define QUALITYOFSERVICEWINDOW_H

#include <QDockWidget>
#include "busDevice.h"
#include <QList>
#include <QTimer>

namespace Ui {
class qualityOfServiceWindow;
}

class qualityOfServiceWindow : public QDockWidget
{
    Q_OBJECT

public:
    explicit qualityOfServiceWindow(QList<RoomBusDevice*> *deviceList, QWidget *parent = nullptr);
    ~qualityOfServiceWindow();

private slots:

    void on_statusUpdate(void);

    void on_readButton_clicked();

    void on_autoReadCheckBox_stateChanged(int arg1);

    void on_readTimer(void);

    void read(void);

    void on_readTimeBox_valueChanged(int arg1);

    void on_qualityOfServiceWindow_visibilityChanged(bool visible);

private:
    Ui::qualityOfServiceWindow *ui;

    QList<RoomBusDevice*> *_deviceList = nullptr;

    QTimer _autoReadTimer;
};

#endif // QUALITYOFSERVICEWINDOW_H
