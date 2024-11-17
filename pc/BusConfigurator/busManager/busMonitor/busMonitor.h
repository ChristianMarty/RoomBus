#ifndef BUSMONITOR_H
#define BUSMONITOR_H

#include <QDialog>

#include "busDevice.h"

namespace Ui {
class busMonitor;
}

class busMonitor : public QDialog
{
    Q_OBJECT

public:
    explicit busMonitor(QWidget *parent = nullptr);
    ~busMonitor();

    void on_newMessage(BusMessage msg);

private slots:
    void on_clearButton_clicked();

private:
    Ui::busMonitor *ui;
};

#endif // BUSMONITOR_H
