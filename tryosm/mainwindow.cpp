#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSqlQuery>
#include <QDebug>
#include "tosmtosqlite.h"
#include "routingprofiles.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
//    TOsmToSqlite tosm("/home/arxaoc/russia.highway.osm", "/media/arxaoc/A701/russia.highway.sqlite", true);

//    TOsmToSqlite tosm("/home/arxaoc/RU-SVE.osm", "/home/arxaoc/ArxOsm5.sqlite", true);
//    exit(0);
    osm = new TOSMWidget("C:\\temp\\ArxOsm5.sqlite", this);
//    osm = new TOSMWidget("/home/arxaoc/ArxOsm5.sqlite", this);
//    osm = new TOSMWidget("/media/arxaoc/A701/russia.highway.sqlite", this);
    ui->verticalLayout->addWidget(osm);
    osm->routeProfile = new TPedestrianProfile;

}

void MainWindow::paintEvent(QPaintEvent *)
{

}

MainWindow::~MainWindow()
{
//    qDebug() << "~MainWindow() {";
//    delete osm;
//    qDebug() << "osm deleted";
    delete ui;
//    qDebug() << "}";
}

void MainWindow::on_horizontalSlider_actionTriggered(int action)
{
//    osm->fillUsage();
    osm->drawProp = (double)ui->horizontalSlider->value() / 100.0;
    osm->update();
}

void MainWindow::on_checkBox_clicked()
{
    osm->useMetric = ui->checkBox->isChecked();
}
