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
    explicit busConnectionWidget(busAccess *com, QWidget *parent = nullptr);
    ~busConnectionWidget();


private slots:
    void on_connectionChanged(bool connected);

private:
    Ui::busConnectionWidget *ui;

    void setStatusLabel(bool open);

     busAccess *_com;
};

#endif // BUSCONNECTIONWIDGET_H
