#include "passwd_set.h"
#include "ui_passwd_set.h"

#include <QKeyEvent>
#include <QEvent>

#include "./Resources/gui_setting.h"
#include "./Resources/guiconfig.h"

extern Gui_config   guisetting;
extern QString syzk_user;

passwd_set::passwd_set(QString name, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::passwd_set)
{
    ui->setupUi(this);
    index = 0;
    this->pst = ParaSynchronizeThread::getInstance();
    this->comm_acs_thread = CommAcsSocketThread::getInstance();
    comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
    this->widget_stack = WidgetStack::getInstance();
    this->para = Parameters::getInstance();
    this->gui_set = gui_setting::getInstance();
    ui->label_old_passwd->setFocus();
    name_switch = name;

    if(name == "user"){
        ui->label_title->setText("操作员");
    } else if (name == "admin"){
        ui->label_title->setText("管理员");
    } else if (name == "dev"){
        ui->label_title->setText("开发人员");
    }

    ui->lineEdit_old_passwd->setEchoMode(QLineEdit::Password);
    ui->lineEdit_new_passwd->setEchoMode(QLineEdit::Password);
    ui->lineEdit_sure_passwd->setEchoMode(QLineEdit::Password);
    ui->lineEdit_old_passwd->setPlaceholderText("请输入原密码");
    ui->lineEdit_new_passwd->setPlaceholderText("请输入新密码");
    ui->lineEdit_sure_passwd->setPlaceholderText("请再次输入新密码");
    ui->lineEdit_old_passwd->setFocus();
}

passwd_set::~passwd_set()
{
    delete ui;
}

bool passwd_set::event(QEvent *event)
{
    QString first = "user_passwd";
    QString second_user = "user";
    QString content_encrpt;
    QString dst, src, sure;

    QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
    switch(key_event->type())
    {
    case KeyValue::KEY_PRESSED:    // key pressed
        break;
    case KeyValue::KEY_RELEASED:  // key released
        switch(key_event->key()){

        case KeyValue::KEY_ARROW_LEFT:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(index == 2)
            {
                str_temp = ui->lineEdit_sure_passwd->text();
                if(str_temp.size() > 0){
                    str_temp = str_temp.mid(0, str_temp.size()-1);
                    ui->lineEdit_sure_passwd->setText(str_temp);
                }

            } else if(index == 1) {
                str_temp = ui->lineEdit_new_passwd->text();
                if(str_temp.size() > 0){
                    str_temp = str_temp.mid(0, str_temp.size()-1);
                    ui->lineEdit_new_passwd->setText(str_temp);
                }
            } else if(index == 0) {
                str_temp = ui->lineEdit_old_passwd->text();
                if(str_temp.size() > 0){
                    str_temp = str_temp.mid(0, str_temp.size()-1);
                    ui->lineEdit_old_passwd->setText(str_temp);
                }
            }

            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_OK:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            dst = ui->lineEdit_new_passwd->text();
            src = ui->lineEdit_old_passwd->text();
            sure = ui->lineEdit_sure_passwd->text();
            ui->label_msg->setStyleSheet("color:yellow;");
            if (src == "") {
                ui->label_msg->setText("原密码不能为空!");
            } else if(dst == "") {
                ui->label_msg->setText("新密码不能为空!");
            } else if (sure == "") {
                ui->label_msg->setText("确认密码不能为空!");
            } else {
                QString ori_passwd;
                if(syzk_user == "user"){
                    ori_passwd = guisetting.user_passwd;
                } else if(syzk_user == "admin") {
                    ori_passwd = guisetting.admin_passwd;
                } else if(syzk_user == "dev") {
                    ori_passwd = guisetting.dev_passwd;
                }

                if(ui->lineEdit_old_passwd->text() != ori_passwd)
                {
                    ui->label_msg->setText("密码错误!");
                } else if (dst != sure) {
                    ui->label_msg->setText("密码输入不一致!");
                } else {
                    this->para->Encrypt(dst, content_encrpt);
                    if(syzk_user == "user"){
                       guisetting.user_passwd = sure;
                       second_user = "user";
                    } else if(syzk_user == "admin") {
                        guisetting.admin_passwd = sure;
                        second_user = "admin";
                    } else if(syzk_user == "dev") {
                        guisetting.dev_passwd = sure;
                        second_user = "dev";
                    }
                    gui_set->write(first, second_user, content_encrpt);
//                    str_send += first + ":" + second_user + ":" + content_encrpt;

                    QJsonObject sync_passwd_obj;
                    sync_passwd_obj["group"] = "user_passwd";
                    sync_passwd_obj["user"]  = "admin";
                    sync_passwd_obj["pwd"]   = content_encrpt;

                    QJsonObject param_obj;
                    param_obj["sync_passwd"] = sync_passwd_obj;

                    QJsonDocument doc(param_obj);
                    QString str_send = doc.toJson(QJsonDocument::Compact);
                    this->pst->send_para_synchronization_msg(str_send);

                    this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
                    this->close();
                    this->~passwd_set();
                }
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_UP://textedit
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(index == 1)
            {
                index = 0;
                ui->lineEdit_old_passwd->setFocus();
            } else if (index == 2){
                index = 1;
                ui->lineEdit_new_passwd->setFocus();
            } else if (index == 0){
                index = 2;
                ui->lineEdit_sure_passwd->setFocus();
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_DOWN:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);

            if(index == 0)
            {
                index = 1;
                ui->lineEdit_new_passwd->setFocus();
            } else if (index == 1){
                index = 2;
                ui->lineEdit_sure_passwd->setFocus();
            } else if (index == 2){
                index = 0;
                ui->lineEdit_old_passwd->setFocus();
            }

            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_NUM_0:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(index == 2)
                ui->lineEdit_sure_passwd->setText(ui->lineEdit_sure_passwd->text() + "0");
            else if(index == 1)
                ui->lineEdit_new_passwd->setText(ui->lineEdit_new_passwd->text() + "0");
            else if(index == 0)
                ui->lineEdit_old_passwd->setText(ui->lineEdit_old_passwd->text() + "0");


            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_NUM_1:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(index == 2)
                ui->lineEdit_sure_passwd->setText(ui->lineEdit_sure_passwd->text() + "1");
            else if(index == 1)
                ui->lineEdit_new_passwd->setText(ui->lineEdit_new_passwd->text() + "1");
            else if(index == 0)
                ui->lineEdit_old_passwd->setText(ui->lineEdit_old_passwd->text() + "1");

            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_NUM_2:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);

            if(index == 2)
                ui->lineEdit_sure_passwd->setText(ui->lineEdit_sure_passwd->text() + "2");
            else if(index == 1)
                ui->lineEdit_new_passwd->setText(ui->lineEdit_new_passwd->text() + "2");
            else if(index == 0)
                ui->lineEdit_old_passwd->setText(ui->lineEdit_old_passwd->text() + "2");

            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_NUM_3:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);

            if(index == 2)
                ui->lineEdit_sure_passwd->setText(ui->lineEdit_sure_passwd->text() + "3");
            else if(index == 1)
                ui->lineEdit_new_passwd->setText(ui->lineEdit_new_passwd->text() + "3");
            else if(index == 0)
                ui->lineEdit_old_passwd->setText(ui->lineEdit_old_passwd->text() + "3");


            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_NUM_4:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);

            if(index == 2)
                ui->lineEdit_sure_passwd->setText(ui->lineEdit_sure_passwd->text() + "4");
            else if(index == 1)
                ui->lineEdit_new_passwd->setText(ui->lineEdit_new_passwd->text() + "4");
            else if(index == 0)
                ui->lineEdit_old_passwd->setText(ui->lineEdit_old_passwd->text() + "4");

            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_NUM_5:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(index == 2)
                ui->lineEdit_sure_passwd->setText(ui->lineEdit_sure_passwd->text() + "5");
            else if(index == 1)
                ui->lineEdit_new_passwd->setText(ui->lineEdit_new_passwd->text() + "5");
            else if(index == 0)
                ui->lineEdit_old_passwd->setText(ui->lineEdit_old_passwd->text() + "5");

            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_NUM_6:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);

            if(index == 2)
                ui->lineEdit_sure_passwd->setText(ui->lineEdit_sure_passwd->text() + "6");
            else if(index == 1)
                ui->lineEdit_new_passwd->setText(ui->lineEdit_new_passwd->text() + "6");
            else if(index == 0)
                ui->lineEdit_old_passwd->setText(ui->lineEdit_old_passwd->text() + "6");

            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_NUM_7:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);

            if(index == 2)
                ui->lineEdit_sure_passwd->setText(ui->lineEdit_sure_passwd->text() + "7");
            else if(index == 1)
                ui->lineEdit_new_passwd->setText(ui->lineEdit_new_passwd->text() + "7");
            else if(index == 0)
                ui->lineEdit_old_passwd->setText(ui->lineEdit_old_passwd->text() + "7");

            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_PUSH_PULL:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);

            if(index == 2)
                ui->lineEdit_sure_passwd->setText(ui->lineEdit_sure_passwd->text() + "8");
            else if(index == 1)
                ui->lineEdit_new_passwd->setText(ui->lineEdit_new_passwd->text() + "8");
            else if(index == 0)
                ui->lineEdit_old_passwd->setText(ui->lineEdit_old_passwd->text() + "8");

            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_NUM_9:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if(index == 2)
                ui->lineEdit_sure_passwd->setText(ui->lineEdit_sure_passwd->text() + "9");
            else if(index == 1)
                ui->lineEdit_new_passwd->setText(ui->lineEdit_new_passwd->text() + "9");
            else if(index == 0)
                ui->lineEdit_old_passwd->setText(ui->lineEdit_old_passwd->text() + "9");
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;

        case KeyValue::KEY_CANCEL:
            this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
            this->close();
            this->~passwd_set();
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
