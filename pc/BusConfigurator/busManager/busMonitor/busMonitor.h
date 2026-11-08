#ifndef BUS_MONITOR_H
#define BUS_MONITOR_H
#include <QWidget>

#include "miniBusMessage.h"
#include "busMonitorModel.h"

namespace Ui {
class BusMonitor;
}

class BusMonitor : public QWidget
{
    Q_OBJECT
public:
    explicit BusMonitor(QWidget *parent = nullptr);
    ~BusMonitor();

    void append(const QList<MiniBus::Message> &message);

private slots:
    void on_clearButton_clicked();

private:
    Ui::BusMonitor *ui;
    BusMonitorModel _busMonitorModel;
};

#endif // BUS_MONITOR_H
