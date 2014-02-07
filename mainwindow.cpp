#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QVector>
#include "math.h"
#include <QTimer>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //ui->graphWidget->setDataLimits(-3, 10);
    ui->graphWidget->setGridSize(10,10);
    ui->graphWidget->setXAxisLimits(0,4000);
    ui->graphWidget->setHeaderText("Graph Title");
    ui->graphWidget->setFooterText("This is a footer.");

    m_data.resize(4000);
    newData();

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(newData()));
    m_timer->start(33);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::newData()
{
    float *data = (float *)m_data.constData();

    for(int i = 0; i < 4000; i++)
    {
        data[i] = (sin((float)i/100.0)/2.0);
        if(ui->noiseSlider->value() != 0)
        {
            data[i] += ((((float)rand()/RAND_MAX)-0.5)/(100 / ui->noiseSlider->value()));
        }

        //data[i] *= 10;
    }

    ui->graphWidget->setData(m_data);
}

void MainWindow::on_LeftAxisButton_toggled()
{
    ui->graphWidget->setAxisStyle(GlGraphWidget::LeftAxis);
}

void MainWindow::on_RightAxisButton_toggled()
{
    ui->graphWidget->setAxisStyle(GlGraphWidget::RightAxis);
}

void MainWindow::on_NoAxisButton_toggled()
{
    ui->graphWidget->setAxisStyle(GlGraphWidget::NoAxis);
}

void MainWindow::on_AutoScaleButton_toggled(bool checked)
{
    ui->graphWidget->setAutoScale(checked);
}

void MainWindow::on_YMax_valueChanged(double)
{
    ui->graphWidget->setYAxisLimits(ui->YMin->value(), ui->YMax->value());
    ui->graphWidget->setAutoScale(ui->AutoScaleButton->isChecked());
}

void MainWindow::on_YMin_valueChanged(double)
{
    ui->graphWidget->setYAxisLimits(ui->YMin->value(), ui->YMax->value());
    ui->graphWidget->setAutoScale(ui->AutoScaleButton->isChecked());
}

void MainWindow::on_LineWidth_valueChanged(int value)
{
    ui->graphWidget->setLineWidth(value);
}

void MainWindow::on_AxisLineWidth_valueChanged(int value)
{
    ui->graphWidget->setAxisLineWidth(value);
}

void MainWindow::on_GridLineWidth_valueChanged(int value)
{
    ui->graphWidget->setGridLineWidth(value);
}

void MainWindow::on_GridX_valueChanged(int)
{
    ui->graphWidget->setGridSize(ui->GridX->value(), ui->GridY->value());
}

void MainWindow::on_GridY_valueChanged(int)
{
    ui->graphWidget->setGridSize(ui->GridX->value(), ui->GridY->value());
}

void MainWindow::on_fps_valueChanged(int value)
{
    m_timer->start(1000/value);
}

void MainWindow::on_lineHeader_textChanged(const QString text)
{
    ui->graphWidget->setHeaderText(text);
}

void MainWindow::on_lineFooter_textChanged(const QString text)
{
    ui->graphWidget->setFooterText(text);
}
