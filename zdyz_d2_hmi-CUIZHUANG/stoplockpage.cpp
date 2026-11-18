#include "stoplockpage.h"
#include "ui_stoplockpage.h"

#include <QFile>
#include <QDebug>
#include "./Resources/error_code.h"

StopLockpage::StopLockpage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StopLockpage)
{
    ui->setupUi(this);
}

StopLockpage::StopLockpage(QString stop_lock, QString number, QWidget *parent):
    QWidget(parent),
    ui(new Ui::StopLockpage)
{
    QFile file(":/qss/stoplockpage.qss");
    if(!file.open(QIODevice::ReadOnly)){
        TLOG_INFO("open qss file failed %s %d", __FILE__, __LINE__);
    }else{
        QString ss = file.readAll();
        //LOG(INFO) << ss.toLocal8Bit().data();
        this->setStyleSheet(ss);
        file.close();
    }

    ui->setupUi(this);
    this->address_set = AddressSet::getInstance();

    this->label_human_lock = nullptr;

    if(number != ""){
        if(QString::number(this->address_set->get_local_address())== number )
        {
            ui->label_name->setText(stop_lock);
            ui->label_number->setText("本架");
        } else {
            ui->label_name->setText(stop_lock);
            ui->label_number->setText(number + "#");
        }
    } else {
        ui->label_name->setText("");
        ui->label_number->setText("");

        this->label_human_lock = new QLabel(this);

        this->label_human_lock->setGeometry(0, 80, 480, 100);
        this->label_human_lock->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        this->label_human_lock->setText(stop_lock);

    }
    this->widget_stack = WidgetStack::getInstance();
}


StopLockpage::~StopLockpage()
{
    delete ui;
}


void StopLockpage::update(QString stop_lock, QString number){
    if(this->label_human_lock != nullptr){
        this->label_human_lock->hide();
    }
    if(QString::number(address_set->get_local_address()) == number)
    {
        ui->label_number->setText("本架");
        ui->label_name->setText(stop_lock);
        return;
    }
    ui->label_name->setText(stop_lock);
    ui->label_number->setText(number + "#");

}
