#include "pumpstation_monitor.h"
#include "ui_pumpstation_monitor.h"

pumpstation_monitor::pumpstation_monitor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::pumpstation_monitor)
{
    ui->setupUi(this);
}

pumpstation_monitor::~pumpstation_monitor()
{
    delete ui;
}
