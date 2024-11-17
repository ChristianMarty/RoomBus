#ifndef BUSCONNECTIONWIDGET_H
#define BUSCONNECTIONWIDGET_H

#include <QWidget>
#include "busAccess.h"


namespace Ui {
class busConnectionWidget;
}

class busConnectionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit busConnectionWidget(RoomBusAccess &roomBusAccess, QWidget *parent = nullptr);
    ~busConnectionWidget();


private slots:
    void on_connectionChanged(void);

private:
    Ui::busConnectionWidget *ui;
    RoomBusAccess &_roomBusAccess;

    void _updateUi(void);
};

#endif // BUSCONNECTIONWIDGET_H
