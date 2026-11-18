#include "dialog_help.h"
#include "ui_dialog_help.h"
//#include "glog/logging.h"
#include "zlog/zlog.h"
#include "tlog/tlog.h"
#include <QNetworkInterface>

Dialog_help::Dialog_help(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_help)
{
    ui->setupUi(this);
    sensor = Sensor::getInstance();
    GetStylePath();
    ui->textEdit_readme->moveCursor(QTextCursor::Start);
}

Dialog_help::~Dialog_help()
{
    delete ui;
}

int Dialog_help::GetStylePath()
{

    QFile file(":/qss/help.qss");
    if(!file.open(QIODevice::ReadOnly)){
    }else{
        QString ss = file.readAll();
        this->setStyleSheet(ss);
        file.close();
    }
    this->comm_acs_thread = CommAcsSocketThread::getInstance();
    this->widget_stack = WidgetStack::getInstance();
    comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);

    QString display = QString("phy info:\r\n");

    display += QString("port10:     mode(");
    display += sensor->phy_info[0][2]==1?QString("master)   link("):QString("slave)   link(");
    display += sensor->phy_info[0][0]==1?QString("up)  speed( "):QString("down)   speed(");
    display += QString::number(sensor->phy_info[0][1]) + QString(")\r\n");

    display += QString("portleft:    mode(");
    display += sensor->phy_info[1][2]==1?QString("master)   link("):QString("slave)   link(");
    display += sensor->phy_info[1][0]==1?QString("up)  speed( "):QString("down)   speed(");
    display += QString::number(sensor->phy_info[1][1]) + QString(")\r\n");

    display += QString("portright:   mode(");
    display += sensor->phy_info[2][2]==1?QString("master)   link("):QString("slave)   link(");
    display += sensor->phy_info[2][0]==1?QString("up)  speed( "):QString("down)   speed(");
    display += QString::number(sensor->phy_info[2][1]) + QString(")\r\n\r\n");

    system("ifconfig -a > /var/run/ifconfig.info");
    file.setFileName("/var/run/ifconfig.info");
    if(!file.open(QIODevice::ReadOnly)){
    }else{
        display += file.readAll();
        ui->textEdit_readme->setText(display);
        file.close();
    }
    return 0;
}

bool Dialog_help::event(QEvent *event)
{
    QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
    switch(key_event->type())
    {
    case KeyValue::KEY_PRESSED:    // key pressed
        break;
    case KeyValue::KEY_RELEASED:  // key released
        switch(key_event->key()){
        case KeyValue::KEY_OK:
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_DOWN:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            ui->textEdit_readme->moveCursor(QTextCursor::Down);
            ui->textEdit_readme->moveCursor(QTextCursor::Down);
            ui->textEdit_readme->moveCursor(QTextCursor::Down);
            ui->textEdit_readme->moveCursor(QTextCursor::Down);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_UP:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            ui->textEdit_readme->moveCursor(QTextCursor::Up);
            ui->textEdit_readme->moveCursor(QTextCursor::Up);
            ui->textEdit_readme->moveCursor(QTextCursor::Up);
            ui->textEdit_readme->moveCursor(QTextCursor::Up);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_CANCEL:
            this->comm_acs_thread->control_timer->stop();
            this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
            this->close();
            this->~Dialog_help();
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        default:
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        break;
    default:
        break;
    }
    return QWidget::event(event);
}


