#include "fault_diagnosis_diaplsy.h"
#include "ui_fault_diagnosis_diaplsy.h"
#include "Resources/key_value.h"
#include <QEvent>
#include <QKeyEvent>
#include <QJsonDocument>
#include "Resources/mqtt_message.h"

fault_diagnosis_diaplsy::fault_diagnosis_diaplsy(QString fault_name, QString fault_dignosis, int fault_id, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::fault_diagnosis_diaplsy)
{
    ui->setupUi(this);

    comm_acs_thread = CommAcsSocketThread::getInstance();
    widget_stack = WidgetStack::getInstance();

    //ui design

    ui->fault_name_label->setText(fault_name);
    ui->fault_dignosis_text->setText(fault_dignosis);
    ui->fault_dignosis_text->moveCursor(QTextCursor::Start);
    m_fault_id = fault_id;
    hide_fn();
}

fault_diagnosis_diaplsy::~fault_diagnosis_diaplsy()
{
    delete ui;
}

void fault_diagnosis_diaplsy::hide_fn()
{
     ui->label_float->setPixmap(QPixmap(QString::fromUtf8(":/new/login/image/fault/buoy_add.png")));
    ui->label_2->hide();
    ui->label_3->hide();
    ui->label_4->hide();
    ui->label_5->hide();
    ui->label_6->hide();
    ui->label_7->hide();
    ui->label_8->hide();
    ui->label_9->hide();
}

void fault_diagnosis_diaplsy::show_fn()
{
    ui->label_float->setPixmap(QPixmap(QString::fromUtf8(":/new/login/image/fault/buoy_del.png")));
    ui->label_2->show();
    ui->label_3->show();
    ui->label_4->show();
    ui->label_5->show();
    ui->label_6->show();
    ui->label_7->show();
    ui->label_8->show();
    ui->label_9->show();
}

bool fault_diagnosis_diaplsy::event(QEvent *event)
{
    QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
    switch(key_event->type())
    {
    case KeyValue::KEY_PRESSED:
        break;
    case KeyValue::KEY_RELEASED:
    {
        switch(key_event->key()){
        case KeyValue::KEY_ARROW_DOWN:
            comm_acs_thread->control_timer->stop();
            ui->fault_dignosis_text->moveCursor(QTextCursor::Down);
            ui->fault_dignosis_text->moveCursor(QTextCursor::Down);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_UP:
            comm_acs_thread->control_timer->stop();
            ui->fault_dignosis_text->moveCursor(QTextCursor::Up);
            ui->fault_dignosis_text->moveCursor(QTextCursor::Up);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_CANCEL:
            this->comm_acs_thread->control_timer->stop();
            this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
            this->close();
            this->~fault_diagnosis_diaplsy();
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_F1:{

            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_F2:{
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_F3:{
            if(m_aux_flag)
            {
                QJsonObject obj;
                obj["secure"] = m_fault_id;
                QJsonDocument jsonDoc(obj);
                comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_FALUT), jsonDoc.toJson());
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_F4:{
            if(m_aux_flag)
            {
                QJsonObject obj;
                obj["shield"] = m_fault_id;
                QJsonDocument jsonDoc(obj);
                comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_FALUT), jsonDoc.toJson());
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_F5:{
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_F6:{
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        case KeyValue::KEY_F7:
            this->comm_acs_thread->control_timer->stop();
            if(!m_aux_flag){
                show_fn();
                m_aux_flag = true;
            } else {
                hide_fn();
                m_aux_flag = false;
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        default:
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
    }
        break;
    default:
        break;
    }
    return QWidget::event(event);
}
