#ifndef BUSMONITORLINE_H
#define BUSMONITORLINE_H

#include <QWidget>
#include "busDevice.h"

namespace Ui {
class BusMonitorLine;
}

class BusMonitorLine : public QWidget
{
    Q_OBJECT

public:
    explicit BusMonitorLine(QWidget *parent = nullptr);
    explicit BusMonitorLine(busMessage &msg, QWidget *parent = nullptr);
    ~BusMonitorLine();

private:
    Ui::BusMonitorLine *ui;
};

#endif // BUSMONITORLINE_H
