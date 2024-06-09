#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "../../QCodebase/hexFileParser.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_merge_button_clicked();

    void on_selectUpdater_button_clicked();

    void on_selectKernel_button_clicked();

private:
    Ui::MainWindow *ui;

    void _updateValues(void);

    void _loadUpdater(void);
    void _loadKernel(void);

    hexFileParser _updaterBinary;
    hexFileParser _kernelBinary;

    hexFileParser _output;

    QString _savePath;
};
#endif // MAINWINDOW_H
