#ifndef TCPCONNECTIONWIDGET_H
#define TCPCONNECTIONWIDGET_H

#include <QWidget>

#include <busAccess.h>

namespace Ui {
class tcpConnectionWidget;
}

class tcpConnectionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit tcpConnectionWidget(QWidget *parent = nullptr);
    ~tcpConnectionWidget();

private slots:
    void on_connectButton_clicked();

private:
    Ui::tcpConnectionWidget *ui;
};

#endif // TCPCONNECTIONWIDGET_H
