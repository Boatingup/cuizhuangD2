#include "driver_chn_set.h"
#include "ui_driver_chn_set.h"

#include <QEvent>
#include <QKeyEvent>
#include "./Resources/key_value.h"
driver_chn_set::driver_chn_set(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::driver_chn_set)
{
    ui->setupUi(this);
    QFile file(":/qss/driver_chn_set.qss");
    if(!file.open(QFile::ReadOnly)){
       TLOG_INFO("open qss file failed in menuPage().");
    }else{
        QString styleSheet = file.readAll();
        this->setStyleSheet(styleSheet);
        file.close();
    }
    this->move(60, 16);
    this->comm_acs_thread = CommAcsSocketThread::getInstance();
    this->widget_stack = WidgetStack::getInstance();
    flag = 0;
    recover = 0;
    ui->label->setText("驱动器参数保存");
    ui->set_label->setText("按确认保存");
    ui->cancel_label->setText("按取消返回");
}

driver_chn_set::~driver_chn_set()
{
    delete ui;
}


bool driver_chn_set::event(QEvent *event)
{
    QString strtab;

    QKeyEvent *key_event = static_cast<QKeyEvent*>(event);
    switch(key_event->type()){
    case KeyValue::KEY_RELEASED:  // key released
        switch(key_event->key()){
        case KeyValue::KEY_ARROW_RIGHT:
        case KeyValue::KEY_ARROW_MINUS:
        case KeyValue::KEY_ARROW_PLUS:
        case KeyValue::KEY_ARROW_UP:
        case KeyValue::KEY_ARROW_DOWN:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_LEFT:
            if(++recover == 5)
            {
                ui->label->setText("驱动器通道恢复出厂设置");
                ui->set_label->setText("按确认恢复");
                ui->cancel_label->setText("按取消返回");
            }
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_OK:
            comm_acs_thread->control_timer->stop();
            if((flag == 0)&&(recover < 5)){
                emit save_signal(1);
                flag = 1;
                ui->label->setText("驱动器参数同步");
                ui->set_label->setText("按确认同步");
                ui->cancel_label->setText("按取消返回");
            }
            else {
                if(recover > 4)
                    emit save_signal(3);
                else
                    emit save_signal(2);
                this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
                this->close();
                this->~driver_chn_set();
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_CANCEL:
            emit save_signal(0);
            this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
            this->close();
            this->~driver_chn_set();
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
