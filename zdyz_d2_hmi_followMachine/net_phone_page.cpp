#include "net_phone_page.h"
#include "ui_net_phone_page.h"
extern bool net_phone_page;
bool net_phone_page = false;
NetPhonePage::NetPhonePage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NetPhonePage)
{
    ui->setupUi(this);
    net_phone_page = true;
}

NetPhonePage::~NetPhonePage()
{
    net_phone_page = false;
    delete ui;
}

void NetPhonePage::set_occuried(int occur_io)
{
    QString display = "第";
    display += QString::number(occur_io);
    display += "架正在广播";
    ui->label->setText(display);
    ui->label_2->setText("请勿点击广播");
}
