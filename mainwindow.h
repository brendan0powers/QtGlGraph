#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>

namespace Ui {
class MainWindow;
}

class QTimer;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected slots:
    void newData();
    void on_LeftAxisButton_toggled();
    void on_RightAxisButton_toggled();
    void on_NoAxisButton_toggled();
    void on_AutoScaleButton_toggled(bool checked);
    void on_YMax_valueChanged(double value);
    void on_YMin_valueChanged(double value);
    void on_LineWidth_valueChanged(int value);
    void on_AxisLineWidth_valueChanged(int value);
    void on_GridLineWidth_valueChanged(int value);
    void on_GridX_valueChanged(int value);
    void on_GridY_valueChanged(int value);
    void on_fps_valueChanged(int value);

private:
    Ui::MainWindow *ui;
    QVector<float> m_data;

    QTimer *m_timer;
};

#endif // MAINWINDOW_H
