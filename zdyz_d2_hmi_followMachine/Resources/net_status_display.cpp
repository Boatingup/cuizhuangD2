#include "net_status_display.h"
#include "ui_net_status_display.h"

net_status_display::net_status_display(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::net_status_display)
{
    ui->setupUi(this);
}

net_status_display::~net_status_display()
{
    delete ui;
}
