#include "manual_address.h"
#include "ui_manual_address.h"
#include "./Resources/key_value.h"
#include "./threads/comm_acs_thread.h"
#include "./window/mainpage.h"
#include "./Resources/error_code.h"
#include "./Resources/mqtt_message.h"

#include <QEvent>
#include <QKeyEvent>

ManualAddress::ManualAddress(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ManualAddress)
{
    ui->setupUi(this);

    this->comm_acs_thread = CommAcsSocketThread::getInstance();
    this->address_set = AddressSet::getInstance();
    this->widget_stack = WidgetStack::getInstance();

    str_temp = QString::number(this->address_set->get_local_address());
    ui->label_number->setText(str_temp);

    this->first_time_to_input = true;
    this->is_auto_addressing = 0;


    this->timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timer_cb()));
    timer->setSingleShot(true);
    comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);

}

ManualAddress::~ManualAddress()
{
    delete ui;
    delete timer;
}

void ManualAddress::timer_cb(void){
    ui->label_13->setText("编址完成");
}


bool ManualAddress::event(QEvent *event){
    QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
    QString msg;
    unsigned long address_temp = 0;
    switch(key_event->type())
    {
    case KeyValue::KEY_PRESSED:    // key pressed

        break;
    case KeyValue::KEY_RELEASED:  // key released
        switch(key_event->key()){

        case KeyValue::KEY_F2:
            comm_acs_thread->control_timer->stop();
            if(is_auto_addressing == 0){
                if(str_temp != ""){
                    address_temp = static_cast<unsigned long>(str_temp.toUInt());
                    if(address_temp > 0 && address_temp < 240){
                        this->address_set->set_local_address(static_cast<unsigned char>(address_temp));
                        msg = "{\"sys\":{\"LocalID\":" + str_temp + "}}";
                        this->comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_PARAMETER_SET), msg.toLatin1());
                    }
                    msg = "{\"auto_address\":1}";
                    this->comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_CONFIG), msg.toLatin1());
                    this->is_auto_addressing = 1;

                    ui->label_13->setText("编址中...");
                    this->timer->start(10000); // 10s
                }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_F1:
            comm_acs_thread->control_timer->stop();
            if(str_temp.size() > 0){
                str_temp = str_temp.mid(0, str_temp.size()-1);
                ui->label_number->setText(str_temp);
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_F3:
            comm_acs_thread->control_timer->stop();
            address_temp = static_cast<unsigned long>(str_temp.toUInt());
            if(address_temp > 0 && address_temp < 239 ){
                address_temp += 1;
                str_temp = QString::number(address_temp);
                ui->label_number->setText(str_temp);
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_F4:
            comm_acs_thread->control_timer->stop();
            address_temp = static_cast<unsigned long>(str_temp.toUInt());
            if(address_temp > 1 && address_temp < 240 ){
                address_temp -= 1;
                str_temp = QString::number(address_temp);
                ui->label_number->setText(str_temp);
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_F5:
            comm_acs_thread->control_timer->stop();
            address_temp = static_cast<unsigned long>(str_temp.toUInt());
            if(address_temp > 0 && address_temp < 230 ){
                address_temp += 10;
                str_temp = QString::number(address_temp);
                ui->label_number->setText(str_temp);
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_F6:
            comm_acs_thread->control_timer->stop();
            address_temp = static_cast<unsigned long>(str_temp.toUInt());
            if(address_temp > 10 && address_temp < 240 ){
                address_temp -= 10;
                str_temp = QString::number(address_temp);
                ui->label_number->setText(str_temp);
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_OK:
            comm_acs_thread->control_timer->stop();
            address_temp = static_cast<unsigned long>(str_temp.toUInt());
            if(address_temp > 0 && address_temp < 240){
                this->address_set->set_local_address(static_cast<unsigned char>(address_temp));
                msg = "{\"sys\":{\"LocalID\":" + str_temp + "}}";
                this->comm_acs_thread->send_socket_msg_to_acs(QString(TOPIC_PARAMETER_SET), msg.toLatin1());

                emit this->comm_acs_thread->cmdManager->signal_update_config_data(mainPage::View_Local_Address);
                this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
                this->close();
                this->~ManualAddress();
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_CANCEL:
            this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
            this->close();
            this->~ManualAddress();
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_NUM_0:
            comm_acs_thread->control_timer->stop();
            if(first_time_to_input){
                str_temp = "";
                first_time_to_input = false;
            }
            str_temp += "0";
            ui->label_number->setText(str_temp);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_NUM_1:
            comm_acs_thread->control_timer->stop();
            if(first_time_to_input){
                str_temp = "";
                first_time_to_input = false;
            }
            str_temp += "1";
            ui->label_number->setText(str_temp);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_NUM_2:
            comm_acs_thread->control_timer->stop();
            if(first_time_to_input){
                str_temp = "";
                first_time_to_input = false;
            }
            str_temp += "2";
            ui->label_number->setText(str_temp);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_NUM_3:
            comm_acs_thread->control_timer->stop();
            if(first_time_to_input){
                str_temp = "";
                first_time_to_input = false;
            }
            str_temp += "3";
            ui->label_number->setText(str_temp);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_NUM_4:
            comm_acs_thread->control_timer->stop();
            if(first_time_to_input){
                str_temp = "";
                first_time_to_input = false;
            }
            str_temp += "4";
            ui->label_number->setText(str_temp);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_NUM_5:
            comm_acs_thread->control_timer->stop();
            if(first_time_to_input){
                str_temp = "";
                first_time_to_input = false;
            }
            str_temp += "5";
            ui->label_number->setText(str_temp);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_NUM_6:
            comm_acs_thread->control_timer->stop();
            if(first_time_to_input){
                str_temp = "";
                first_time_to_input = false;
            }
            str_temp += "6";
            ui->label_number->setText(str_temp);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_NUM_7:
            comm_acs_thread->control_timer->stop();
            if(first_time_to_input){
                str_temp = "";
                first_time_to_input = false;
            }
            str_temp += "7";
            ui->label_number->setText(str_temp);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_PUSH_PULL:
            comm_acs_thread->control_timer->stop();
            if(first_time_to_input){
                str_temp = "";
                first_time_to_input = false;
            }
            str_temp += "8";
            ui->label_number->setText(str_temp);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_NUM_9:
            comm_acs_thread->control_timer->stop();
            if(first_time_to_input){
                str_temp = "";
                first_time_to_input = false;
            }
            str_temp += "9";
            ui->label_number->setText(str_temp);
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
