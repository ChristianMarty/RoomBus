#ifndef CONNECTION_WIDGET_H
#define CONNECTION_WIDGET_H

#include <QWidget>
#include "busAccess.h"


namespace Ui {
class ConnectionWidget;
}

class ConnectionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectionWidget(RoomBusAccess &roomBusAccess, QWidget *parent = nullptr);
    ~ConnectionWidget();


private slots:
    void on_connectionChanged(void);

    void on_pushButton_openClose_clicked();

private:
    Ui::ConnectionWidget *ui;
    RoomBusAccess &_roomBusAccess;

    void _updateUi(void);
};

#endif // CONNECTION_WIDGET_H
