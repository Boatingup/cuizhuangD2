#include "dialog_warrant.h"
#include "ui_dialog_warrant.h"
#include <QKeyEvent>
#include <QEvent>

void Dialog_warrant::EncData(QByteArray &data)
{
    static QByteArray key1 = "myprivatekey";
    static QByteArray key2 = "helloworld";
    for(int i = 0; i < data.size(); ++i)
    {
        int keyIndex1 = i % key1.size();
        int keyIndex2 = i % key2.size();
        data[i] = data[i] ^ key1[keyIndex1];
        data[i] = data[i] ^ key2[keyIndex2];
    }
}

Dialog_warrant::Dialog_warrant(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_warrant)
{
    ui->setupUi(this);
    this->comm_acs_thread = CommAcsSocketThread::getInstance();
    this->widget_stack = WidgetStack::getInstance();
    comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
}

Dialog_warrant::~Dialog_warrant()
{
    delete ui;
}

bool Dialog_warrant::event(QEvent *event)
{

    QString temp;
    QByteArray pass_convert;
    QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
    switch(key_event->type())
    {
    case KeyValue::KEY_PRESSED:    // key pressed
        break;
    case KeyValue::KEY_RELEASED:  // key released
        switch(key_event->key()){
        case KeyValue::KEY_ARROW_RIGHT:
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_LEFT:
            comm_acs_thread->control_timer->stop();
            str_temp = str_temp.mid(0, str_temp.size()-1);
            ui->textEdit_passwd->setText(str_temp);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_ARROW_UP:
            comm_acs_thread->control_timer->stop();
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_DOWN:
            comm_acs_thread->control_timer->stop();
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_OK:
            {
            comm_acs_thread->control_timer->stop();
            pass_convert = ui->textEdit_passwd->text().toUtf8();
            EncData(pass_convert);
            //EncData(pass_convert);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            }
            break;

        case KeyValue::KEY_CANCEL:
            this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
            this->close();
            this->~Dialog_warrant();
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_NUM_0:
            comm_acs_thread->control_timer->stop();
            str_temp+="0";
            //ui->textEdit_passwd->clear();
            ui->textEdit_passwd->setText(str_temp);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_NUM_1:
            comm_acs_thread->control_timer->stop();
            str_temp+="1";
            //ui->textEdit_passwd->clear();
            ui->textEdit_passwd->setText(str_temp);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_NUM_2:
            comm_acs_thread->control_timer->stop();
            str_temp+="2";
            //ui->textEdit_passwd->clear();
            ui->textEdit_passwd->setText(str_temp);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_NUM_3:
            comm_acs_thread->control_timer->stop();
            str_temp+="3";
            //ui->textEdit_passwd->clear();
            ui->textEdit_passwd->setText(str_temp);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_NUM_4:
            comm_acs_thread->control_timer->stop();
            str_temp+="4";
            //ui->textEdit_passwd->clear();
            ui->textEdit_passwd->setText(str_temp);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_NUM_5:
            comm_acs_thread->control_timer->stop();
            str_temp+="5";
            //ui->textEdit_passwd->clear();
            ui->textEdit_passwd->setText(str_temp);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_NUM_6:
            comm_acs_thread->control_timer->stop();
            str_temp+="6";
            //ui->textEdit_passwd->clear();
            ui->textEdit_passwd->setText(str_temp);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_NUM_7:
            comm_acs_thread->control_timer->stop();
            str_temp+="7";
            //ui->textEdit_passwd->clear();
            ui->textEdit_passwd->setText(str_temp);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_PUSH_PULL:
            comm_acs_thread->control_timer->stop();
            str_temp+="8";
            //ui->textEdit_passwd->clear();
            ui->textEdit_passwd->setText(str_temp);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_NUM_9:
            comm_acs_thread->control_timer->stop();
            str_temp+="9";
            ui->textEdit_passwd->setText(str_temp);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_F1: //used as "."
            comm_acs_thread->control_timer->stop();
            str_temp+=".";
            ui->textEdit_passwd->setText(str_temp);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        default:
            this->comm_acs_thread->cmdManager->lock_ui_off();
            //
            break;

        }
        break;
    default:
        break;
    }
    return QWidget::event(event);
}
