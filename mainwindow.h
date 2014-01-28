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
    void on_UpdateButton_clicked();

private:
    Ui::MainWindow *ui;
    QVector<float> m_data;

    QTimer *m_timer;
};

#endif // MAINWINDOW_H
