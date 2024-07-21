#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "../../QuCLib/source/hexFileParser.h"

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

    QuCLib::HexFileParser _updaterBinary;
    QuCLib::HexFileParser _kernelBinary;

    QuCLib::HexFileParser _output;

    QString _savePath;
};
#endif // MAINWINDOW_H
