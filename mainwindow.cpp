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

void MainWindow::on_UpdateButton_clicked()
{
    ui->graphWidget->setGridSize(ui->GridX->value(), ui->GridY->value());

    ui->graphWidget->setLineWidth(ui->LineWidth->value());
    ui->graphWidget->setAxisLineWidth(ui->AxisLineWidth->value());
    ui->graphWidget->setGridLineWidth(ui->GridLineWidth->value());

    ui->graphWidget->setDataLimits(ui->YMin->value(), ui->YMax->value());
    ui->graphWidget->setAutoScale(ui->AutoScaleButton->isChecked());

    ui->graphWidget->setLeftAlignedAxis(ui->LeftAxisButton->isChecked());

    m_timer->start(1000/ui->fps->value());
    qDebug() << "START" << ui->fps->value();
}
