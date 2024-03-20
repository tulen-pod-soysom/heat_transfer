#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->h_renderer->program = &program;
    this->on_pushButton_3_clicked();

    timer1.setInterval(1000/30);
    connect(&timer1,SIGNAL(timeout()),this,SLOT(timer1_start()));
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    static bool running = false;

    running = !running;

    if (running)
    {
        program.start_loop();
        timer1.start();
        ui->pushButton_3->setEnabled(false);
        ui->pushButton->setText("СТОП");
    }
    else
    {
        program.stop_loop();
        timer1.stop();
        ui->pushButton_3->setEnabled(true);
        ui->pushButton->setText("СТАРТ");
    }
}


void MainWindow::timer1_start()
{
    ui->h_renderer->repaint();
    QString str; str.sprintf("время системы: %f",program.v.t);
    ui->label->setText(str);
}





void MainWindow::on_pushButton_3_clicked()
{
    auto& prp =program.p;

    prp.liquid = parameters::material(
                ui->conductivity_liquid->text().toDouble(),
                ui->capacity_liquid->text().toDouble()
                );
    prp.metal = parameters::material(
                ui->conductivity_metal->text().toDouble(),
                ui->capacity_metal->text().toDouble()
                );

    prp.z0 = sqrt(prp.liquid.lambda2);

    prp.height                = prp.z_to_z0(ui->height->text().toDouble());
    prp.radius                = prp.z_to_z0(ui->radius->text().toDouble());
    prp.wall_width            = prp.z_to_z0(ui->wall_width->text().toDouble());
    prp.heater_height         = prp.z_to_z0(ui->heater_height->text().toDouble());
    prp.heater_radius         = prp.z_to_z0(ui->heater_radius->text().toDouble());
    prp.heater_power          = ui->heater_power->text().toDouble();
    prp.external_temperature  = ui->temperature->text().toDouble();
    prp.epsilon               = ui->epsilon->text().toDouble();
    prp.r_divisions           = ui->r_divisions->text().toUInt();
    prp.z_divisions           = ui->z_divisions->text().toUInt();
    prp.t_step                = ui->t_step->text().toDouble();

    program.init();
    ui->centralwidget->repaint();
}


void MainWindow::on_do_izolines_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked) ui->h_renderer->do_izolines = true;
    else if (arg1 == Qt::Unchecked) ui->h_renderer->do_izolines = false;
}

